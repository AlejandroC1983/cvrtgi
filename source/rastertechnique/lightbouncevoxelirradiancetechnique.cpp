/*
Copyright 2022 Alejandro Cosin & Gustavo Patow

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/rastertechnique/lightbouncevoxelirradiancetechnique.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/material/materiallightbouncevoxelirradiance.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/scene/scene.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/rastertechnique/litclustertechnique.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/rastertechnique/cameravisiblevoxeltechnique.h"
#include "../../include/material/materiallightbouncevoxelgaussianfilter.h"
#include "../../include/material/materiallightbouncevoxelgaussianfiltersecond.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/util/vulkanstructinitializer.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

LightBounceVoxelIrradianceTechnique::LightBounceVoxelIrradianceTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_techniquePrefixSum(nullptr)
	, m_litClusterProcessResultsTechnique(nullptr)
	, m_litClusterTechnique(nullptr)
	, m_resetClusterIrradianceDataTechnique(nullptr)
	, m_lightBounceVoxelIrradianceBuffer(nullptr)
	, m_lightBounceVoxelDebugBuffer(nullptr)
	, m_numOccupiedVoxel(0)
	, m_prefixSumCompleted(false)
	, m_mainCamera(nullptr)
	, m_cameraVisibleVoxelTechnique(nullptr)
	, m_cameraVisibleVoxelNumber(0)
	, m_lightBounceIndirectLitCounter(0)
	, m_lightBounceVoxelGaussianFilterDebugBuffer(nullptr)
{
	m_numElementPerLocalWorkgroupThread = 1;
	//m_numThreadPerLocalWorkgroup        = 128;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_executeCommand                    = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::init()
{
	m_lightBounceVoxelDebugBuffer = bufferM->buildBuffer(
		move(string("lightBounceVoxelDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_lightBounceIndirectLitIndexBuffer = bufferM->buildBuffer(
		move(string("lightBounceIndirectLitIndexBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_lightBounceIndirectLitCounterBuffer = bufferM->buildBuffer(
		move(string("lightBounceIndirectLitCounterBuffer")),
		(void*)(&m_lightBounceIndirectLitCounter),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_lightBounceVoxelGaussianFilterDebugBuffer = bufferM->buildBuffer(
		move(string("lightBounceVoxelGaussianFilterDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Shader storage buffer with the indices of the elements present in the buffer litHiddenVoxelBuffer
	m_lightBounceVoxelIrradianceBuffer = bufferM->getElement(move(string("lightBounceVoxelIrradianceBuffer")));

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();

	MultiTypeUnorderedMap* attributeMaterial = new MultiTypeUnorderedMap();
	attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_lightBounceVoxelIrradianceCodeChunk), string(m_computeShaderThreadMapping)));
	m_material = materialM->buildMaterial(move(string("MaterialLightBounceVoxelIrradiance")), move(string("MaterialLightBounceVoxelIrradiance")), attributeMaterial);
	m_vectorMaterialName.push_back("MaterialLightBounceVoxelIrradiance");
	m_vectorMaterial.push_back(m_material);

	SceneVoxelizationTechnique* sceneVoxelizationTechnique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	
	BBox3D& box = sceneM->refBox();
	
	vec3 max3D;
	vec3 min3D;
	box.getCenteredBoxMinMax(min3D, max3D);
	
	vec3 extent3D             = max3D - min3D;
	m_sceneMin                = vec4(min3D.x,    min3D.y,    min3D.z, 0.0f);
	m_sceneExtent             = vec4(extent3D.x, extent3D.y, extent3D.z, float(sceneVoxelizationTechnique->getVoxelizedSceneWidth()));

	MaterialLightBounceVoxelIrradiance* materialCasted = static_cast<MaterialLightBounceVoxelIrradiance*>(m_material);
	materialCasted->setSceneExtentAndVoxelSize(m_sceneExtent);

	m_litClusterTechnique = static_cast<LitClusterTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("LitClusterTechnique"))));

	m_techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_techniquePrefixSum->refPrefixSumComplete().connect<LightBounceVoxelIrradianceTechnique, &LightBounceVoxelIrradianceTechnique::slotPrefixSumComplete>(this);

	m_mainCamera = cameraM->getElement(move(string("maincamera")));

	m_cameraVisibleVoxelTechnique = static_cast<CameraVisibleVoxelTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("CameraVisibleVoxelTechnique"))));
	m_cameraVisibleVoxelTechnique->refSignalCameraVisibleVoxelCompletion().connect<LightBounceVoxelIrradianceTechnique, &LightBounceVoxelIrradianceTechnique::slotCameraVisibleVoxelCompleted>(this);

	MultiTypeUnorderedMap* attributeMaterialFilter = new MultiTypeUnorderedMap();
	attributeMaterialFilter->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_lightBounceVoxelGaussianFilterCodeChunk), string(m_computeShaderThreadMapping)));
	Material* m_materialFilter = materialM->buildMaterial(move(string("MaterialLightBounceVoxelGaussianFilter")), move(string("MaterialLightBounceVoxelGaussianFilter")), attributeMaterialFilter);
	m_vectorMaterialName.push_back("MaterialLightBounceVoxelGaussianFilter");
	m_vectorMaterial.push_back(m_materialFilter);
	MaterialLightBounceVoxelGaussianFilter* materialCastedFilter = static_cast<MaterialLightBounceVoxelGaussianFilter*>(m_materialFilter);
	materialCastedFilter->setVoxelizationSize(sceneVoxelizationTechnique->getVoxelizedSceneWidth());

	Material* m_materialFilterSecond = materialM->buildMaterial(move(string("MaterialLightBounceVoxelGaussianFilterSecond")), move(string("MaterialLightBounceVoxelGaussianFilterSecond")), attributeMaterialFilter);
	m_vectorMaterialName.push_back("MaterialLightBounceVoxelGaussianFilterSecond");
	m_vectorMaterial.push_back(m_materialFilterSecond);
	MaterialLightBounceVoxelGaussianFilterSecond* materialCastedFilterSecond = static_cast<MaterialLightBounceVoxelGaussianFilterSecond*>(m_materialFilterSecond);
	materialCastedFilterSecond->setVoxelizationSize(sceneVoxelizationTechnique->getVoxelizedSceneWidth());

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_3);
	SignalVoid* signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_3);
	signalAdd->connect<LightBounceVoxelIrradianceTechnique, &LightBounceVoxelIrradianceTechnique::slot3KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_4);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_4);
	signalAdd->connect<LightBounceVoxelIrradianceTechnique, &LightBounceVoxelIrradianceTechnique::slot4KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_5);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_5);
	signalAdd->connect<LightBounceVoxelIrradianceTechnique, &LightBounceVoxelIrradianceTechnique::slot5KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_6);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_6);
	signalAdd->connect<LightBounceVoxelIrradianceTechnique, &LightBounceVoxelIrradianceTechnique::slot6KeyPressed>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* LightBounceVoxelIrradianceTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	commandBufferType = CommandBufferType::CBT_COMPUTE_QUEUE;

	VkCommandBuffer* commandBuffer;
	commandBuffer = addRecordedCommandBuffer(commandBufferID);
	addCommandBufferQueueType(commandBufferID, commandBufferType);

	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getComputeCommandPool(), commandBuffer);

	coreM->beginCommandBuffer(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex0);
#endif

	MaterialLightBounceVoxelIrradiance* castedBounce     = static_cast<MaterialLightBounceVoxelIrradiance*>(m_vectorMaterial[0]);
	MaterialLightBounceVoxelGaussianFilter* castedFilter = static_cast<MaterialLightBounceVoxelGaussianFilter*>(m_vectorMaterial[1]);
	MaterialLightBounceVoxelGaussianFilterSecond* castedFilterSecond = static_cast<MaterialLightBounceVoxelGaussianFilterSecond*>(m_vectorMaterial[2]);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	uint32_t offsetData;
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, castedBounce->getPipeline()->getPipeline());
	offsetData = static_cast<uint32_t>(castedBounce->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, castedBounce->getPipelineLayout(), 0, 1, &castedBounce->refDescriptorSet(), 1, &offsetData);
	vkCmdDispatch(*commandBuffer, castedBounce->getLocalWorkGroupsXDimension(), castedBounce->getLocalWorkGroupsYDimension(), 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

	VulkanStructInitializer::insertBufferMemoryBarrier(bufferM->getElement(move(string("lightBounceVoxelIrradianceBuffer"))),
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   commandBuffer);

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, castedFilter->getPipeline()->getPipeline());
	offsetData = static_cast<uint32_t>(castedFilter->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, castedFilter->getPipelineLayout(), 0, 1, &castedFilter->refDescriptorSet(), 1, &offsetData);
	vkCmdDispatch(*commandBuffer, castedFilter->getLocalWorkGroupsXDimension(), castedFilter->getLocalWorkGroupsYDimension(), 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

	VulkanStructInitializer::insertBufferMemoryBarrier(bufferM->getElement(move(string("lightBounceVoxelFilteredIrradianceBuffer"))),
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);

	// NOTE: Clear command buffer if re-recorded
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::postCommandSubmit()
{
	m_litClusterTechnique->setTechniqueLock(false);
	m_cameraVisibleVoxelTechnique->setLightBounceOnProgress(false);
	m_signalLightBounceVoxelIrradianceCompletion.emit();

	m_executeCommand = false;
	m_active         = false;
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::slotPrefixSumComplete()
{
	m_numOccupiedVoxel = m_techniquePrefixSum->getFirstIndexOccupiedElement();
	m_bufferNumElement = m_numOccupiedVoxel * m_numThreadPerLocalWorkgroup * 6; // Each local workgroup will work one side of each voxel in the scene
	m_sceneMin.w       = float(m_numOccupiedVoxel);

	obtainDispatchWorkGroupCount();

	MaterialLightBounceVoxelIrradiance* materialCasted = static_cast<MaterialLightBounceVoxelIrradiance*>(m_material);
	materialCasted->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	materialCasted->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	materialCasted->setSceneMinAndNumberVoxel(m_sceneMin);
	materialCasted->setNumThreadExecuted(m_bufferNumElement);

	bufferM->resize(m_lightBounceVoxelDebugBuffer,       nullptr,       100 * 120000 * sizeof(uint));
	bufferM->resize(m_lightBounceIndirectLitIndexBuffer, nullptr, m_numOccupiedVoxel * sizeof(uint));
	bufferM->resize(m_lightBounceVoxelGaussianFilterDebugBuffer, nullptr, 6 * 50 * m_numOccupiedVoxel * sizeof(uint));

	cout << "lightBounceVoxelDebugBuffer has size " << (float(m_lightBounceVoxelDebugBuffer->getDataSize()) / 1024.0f) / 1024.0f << "MB" << endl;

	m_prefixSumCompleted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::slotCameraVisibleVoxelCompleted()
{
	if (m_prefixSumCompleted)
	{
		m_cameraVisibleVoxelNumber = m_cameraVisibleVoxelTechnique->getCameraVisibleVoxelNumber();

		m_bufferNumElement = m_cameraVisibleVoxelNumber * m_numThreadPerLocalWorkgroup * 6; // Each local workgroup will work one side of each voxel in the scene
		m_sceneMin.w       = float(m_cameraVisibleVoxelNumber);

		obtainDispatchWorkGroupCount();

		MaterialLightBounceVoxelIrradiance* materialCasted = static_cast<MaterialLightBounceVoxelIrradiance*>(m_material);
		vec3 cameraPosition                                = m_litClusterTechnique->getCameraPosition();
		vec3 cameraForward                                 = m_litClusterTechnique->getCameraForward();
		float emitterRadiance                              = m_litClusterTechnique->getEmitterRadiance();

		materialCasted->setLightPosition(vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.0f));
		materialCasted->setLightForwardEmitterRadiance(vec4(cameraForward.x, cameraForward.y, cameraForward.z, emitterRadiance));
		materialCasted->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
		materialCasted->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
		materialCasted->setSceneMinAndNumberVoxel(m_sceneMin);
		materialCasted->setNumThreadExecuted(m_bufferNumElement);

		m_bufferNumElement = m_cameraVisibleVoxelNumber * 6;

		obtainDispatchWorkGroupCount();
		MaterialLightBounceVoxelGaussianFilter* materialCastedFilter = static_cast<MaterialLightBounceVoxelGaussianFilter*>(m_vectorMaterial[1]);
		materialCastedFilter->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
		materialCastedFilter->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
		materialCastedFilter->setNumThreadExecuted(m_bufferNumElement);

		MaterialLightBounceVoxelGaussianFilterSecond* materialCastedFilterSecond = static_cast<MaterialLightBounceVoxelGaussianFilterSecond*>(m_vectorMaterial[2]);
		materialCastedFilterSecond->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
		materialCastedFilterSecond->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
		materialCastedFilterSecond->setNumThreadExecuted(m_bufferNumElement);

		// Each time CameraVisibleVoxelTechnique this technique needs to record
		m_vectorCommand.clear();

		m_active = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::slot3KeyPressed()
{
	MaterialLightBounceVoxelIrradiance* castedBounce = static_cast<MaterialLightBounceVoxelIrradiance*>(m_vectorMaterial[0]);
	castedBounce->setFormFactorVoxelToVoxelAdded(castedBounce->getFormFactorVoxelToVoxelAdded() - 1.0f);
	cout << "LightBounceVoxelIrradianceTechnique: New value for FormFactorVoxelToVoxelAdded is " << castedBounce->getFormFactorVoxelToVoxelAdded() << endl;
	m_litClusterTechnique->slotCameraDirty(); // Force emitter irradiance update
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::slot4KeyPressed()
{
	MaterialLightBounceVoxelIrradiance* castedBounce = static_cast<MaterialLightBounceVoxelIrradiance*>(m_vectorMaterial[0]);
	castedBounce->setFormFactorVoxelToVoxelAdded(castedBounce->getFormFactorVoxelToVoxelAdded() + 1.0f);
	cout << "LightBounceVoxelIrradianceTechnique: New value for FormFactorVoxelToVoxelAdded is " << castedBounce->getFormFactorVoxelToVoxelAdded() << endl;
	m_litClusterTechnique->slotCameraDirty(); // Force emitter irradiance update
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::slot5KeyPressed()
{
	MaterialLightBounceVoxelIrradiance* castedBounce = static_cast<MaterialLightBounceVoxelIrradiance*>(m_vectorMaterial[0]);
	castedBounce->setFormFactorClusterToVoxelAdded(castedBounce->getFormFactorClusterToVoxelAdded() - 100.0f);
	cout << "LightBounceVoxelIrradianceTechnique: New value for FormFactorClusterToVoxelAdded is " << castedBounce->getFormFactorClusterToVoxelAdded() << endl;
	m_litClusterTechnique->slotCameraDirty(); // Force emitter irradiance update
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LightBounceVoxelIrradianceTechnique::slot6KeyPressed()
{
	MaterialLightBounceVoxelIrradiance* castedBounce = static_cast<MaterialLightBounceVoxelIrradiance*>(m_vectorMaterial[0]);
	castedBounce->setFormFactorClusterToVoxelAdded(castedBounce->getFormFactorClusterToVoxelAdded() + 100.0f);
	cout << "LightBounceVoxelIrradianceTechnique: New value for FormFactorClusterToVoxelAdded is " << castedBounce->getFormFactorClusterToVoxelAdded() << endl;
	m_litClusterTechnique->slotCameraDirty(); // Force emitter irradiance update
}

/////////////////////////////////////////////////////////////////////////////////////////////
