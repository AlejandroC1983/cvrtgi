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
#include "../../include/rastertechnique/voxelfacepenaltytechnique.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/scene/scene.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/rastertechnique/clustervisibleprefixsumtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialvoxelfacepenalty.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

VoxelFacePenaltyTechnique::VoxelFacePenaltyTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_techniquePrefixSum(nullptr)
	, m_techniqueClusterVisiblePrefixSum(nullptr)
	, m_clusterVisibilityFacePenaltyBuffer(nullptr)
	, m_clusterVisibilityFacePenaltyDebugBuffer(nullptr)
	, m_numOccupiedVoxel(0)
	, m_prefixSumCompleted(false)
{
	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 128;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_executeCommand                    = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::init()
{
	m_clusterVisibilityFacePenaltyBuffer = bufferM->buildBuffer(
		move(string("clusterVisibilityFacePenaltyBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_closerVoxelVisibilityFacePenaltyBuffer = bufferM->buildBuffer(
		move(string("closerVoxelVisibilityFacePenaltyBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterVisibilityFacePenaltyDebugBuffer = bufferM->buildBuffer(
		move(string("clusterVisibilityFacePenaltyDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_voxelNeighbourIndexBuffer = bufferM->buildBuffer(
		move(string("voxelNeighbourIndexBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();

	MultiTypeUnorderedMap* attributeMaterial = new MultiTypeUnorderedMap();
	attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelFacePenaltyCodeChunk), string(m_computeShaderThreadMapping)));
	m_material = materialM->buildMaterial(move(string("MaterialVoxelFacePenalty")), move(string("MaterialVoxelFacePenalty")), attributeMaterial);
	m_vectorMaterialName.push_back("MaterialVoxelFacePenalty");
	m_vectorMaterial.push_back(m_material);

	SceneVoxelizationTechnique* sceneVoxelizationTechnique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	
	BBox3D& box = sceneM->refBox();
	
	vec3 max3D;
	vec3 min3D;
	box.getCenteredBoxMinMax(min3D, max3D);
	
	vec3 extent3D = max3D - min3D;
	m_sceneMin    = vec4(min3D.x,    min3D.y,    min3D.z, 0.0f);
	m_sceneExtent = vec4(extent3D.x, extent3D.y, extent3D.z, float(sceneVoxelizationTechnique->getVoxelizedSceneWidth()));

	MaterialVoxelFacePenalty* materialCasted = static_cast<MaterialVoxelFacePenalty*>(m_material);
	materialCasted->setSceneExtentAndVoxelSize(m_sceneExtent);

	m_techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_techniquePrefixSum->refPrefixSumComplete().connect<VoxelFacePenaltyTechnique, &VoxelFacePenaltyTechnique::slotPrefixSumComplete>(this);

	m_techniqueClusterVisiblePrefixSum = static_cast<ClusterVisiblePrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterVisiblePrefixSumTechnique"))));
	m_techniqueClusterVisiblePrefixSum->refSignalClusterVisiblePrefixSumTechnique().connect<VoxelFacePenaltyTechnique, &VoxelFacePenaltyTechnique::ClusterVisiblePrefixSum>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_3);
	SignalVoid* signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_3);
	signalAdd->connect<VoxelFacePenaltyTechnique, &VoxelFacePenaltyTechnique::slot3KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_4);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_4);
	signalAdd->connect<VoxelFacePenaltyTechnique, &VoxelFacePenaltyTechnique::slot4KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_5);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_5);
	signalAdd->connect<VoxelFacePenaltyTechnique, &VoxelFacePenaltyTechnique::slot5KeyPressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_6);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_6);
	signalAdd->connect<VoxelFacePenaltyTechnique, &VoxelFacePenaltyTechnique::slot6KeyPressed>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* VoxelFacePenaltyTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	commandBufferType = CommandBufferType::CBT_COMPUTE_QUEUE;

	VkCommandBuffer* commandBuffer;
	addCommandBufferQueueType(commandBufferID, commandBufferType);
	commandBuffer = addRecordedCommandBuffer(commandBufferID);
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getComputeCommandPool(), commandBuffer);

	coreM->beginCommandBuffer(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex0);
#endif

	MaterialVoxelFacePenalty* castedBounce = static_cast<MaterialVoxelFacePenalty*>(m_vectorMaterial[0]);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	uint32_t offsetData;
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, castedBounce->getPipeline()->getPipeline());
	offsetData = static_cast<uint32_t>(castedBounce->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, castedBounce->getPipelineLayout(), 0, 1, &castedBounce->refDescriptorSet(), 1, &offsetData);
	vkCmdDispatch(*commandBuffer, castedBounce->getLocalWorkGroupsXDimension(), castedBounce->getLocalWorkGroupsYDimension(), 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);

	// NOTE: Clear command buffer if re-recorded
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::postCommandSubmit()
{
	m_signalVoxelFacePenaltyTechniqueCompletion.emit();

	m_executeCommand = false;
	m_active         = false;
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::slotPrefixSumComplete()
{
	m_numOccupiedVoxel = m_techniquePrefixSum->getFirstIndexOccupiedElement();
	m_bufferNumElement = m_numOccupiedVoxel; // Each thread in each local workgroup will process the six faces of the voxel
	m_sceneMin.w       = float(m_numOccupiedVoxel);

	obtainDispatchWorkGroupCount();

	MaterialVoxelFacePenalty* materialCasted = static_cast<MaterialVoxelFacePenalty*>(m_material);
	materialCasted->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	materialCasted->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	// TODO: Remove the numebr voxel part and set this value in VoxelFacePenaltyTechnique::init
	materialCasted->setSceneMinAndNumberVoxel(m_sceneMin);
	materialCasted->setNumThreadExecuted(m_bufferNumElement);

	// Make a buffer with six elements per occupied voxel (to store the amount of visible clusters from each voxel face)
	bufferM->resize(m_clusterVisibilityFacePenaltyBuffer, nullptr, m_numOccupiedVoxel * 6 * sizeof(uint));
	bufferM->resize(m_closerVoxelVisibilityFacePenaltyBuffer, nullptr, m_numOccupiedVoxel * 6 * sizeof(uint));

	// TODO: Avoid this resize to save memory once debug is done
	//bufferM->resize(m_clusterVisibilityFacePenaltyDebugBuffer, nullptr, 95000 * sizeof(uint));
	bufferM->resize(m_voxelNeighbourIndexBuffer, nullptr, m_numOccupiedVoxel * 27 * sizeof(uint));

	m_prefixSumCompleted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::ClusterVisiblePrefixSum()
{
	if (m_prefixSumCompleted)
	{
		m_vectorCommand.clear();
		m_active = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::slot3KeyPressed()
{
	MaterialVoxelFacePenalty* materialCasted = static_cast<MaterialVoxelFacePenalty*>(m_material);
	materialCasted->setFormFactorVoxelToVoxelAdded(materialCasted->getFormFactorVoxelToVoxelAdded() - 1.0f);
	cout << "VoxelFacePenaltyTechnique: New value for FormFactorVoxelToVoxelAdded is " << materialCasted->getFormFactorVoxelToVoxelAdded() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::slot4KeyPressed()
{
	MaterialVoxelFacePenalty* materialCasted = static_cast<MaterialVoxelFacePenalty*>(m_material);
	materialCasted->setFormFactorVoxelToVoxelAdded(materialCasted->getFormFactorVoxelToVoxelAdded() + 1.0f);
	cout << "VoxelFacePenaltyTechnique: New value for FormFactorVoxelToVoxelAdded is " << materialCasted->getFormFactorVoxelToVoxelAdded() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::slot5KeyPressed()
{
	MaterialVoxelFacePenalty* materialCasted = static_cast<MaterialVoxelFacePenalty*>(m_material);
	materialCasted->setFormFactorClusterToVoxelAdded(materialCasted->getFormFactorClusterToVoxelAdded() - 100.0f);
	cout << "VoxelFacePenaltyTechnique: New value for FormFactorClusterToVoxelAdded is " << materialCasted->getFormFactorClusterToVoxelAdded() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VoxelFacePenaltyTechnique::slot6KeyPressed()
{
	MaterialVoxelFacePenalty* materialCasted = static_cast<MaterialVoxelFacePenalty*>(m_material);
	materialCasted->setFormFactorClusterToVoxelAdded(materialCasted->getFormFactorClusterToVoxelAdded() + 100.0f);
	cout << "VoxelFacePenaltyTechnique: New value for FormFactorClusterToVoxelAdded is " << materialCasted->getFormFactorClusterToVoxelAdded() << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////
