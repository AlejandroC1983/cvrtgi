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
#include "../../include/rastertechnique/litclustertechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materiallitcluster.h"
#include "../../include/core/coremanager.h"
#include "../../include/rastertechnique/clusterizationbuildfinalbuffertechnique.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/scene/scene.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/rastertechnique/distanceshadowmappingtechnique.h"
#include "../../include/util/bufferverificationhelper.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/material/materialresetclusterirradiancedata.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/material/materiallitclusterprocessresults.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

LitClusterTechnique::LitClusterTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	
	, m_accumulatedIrradianceBuffer(nullptr)
	, m_litClusterCounterBuffer(nullptr)
	, m_litToRasterVisibleClusterCounterBuffer(nullptr)
	, m_litToRasterVisibleClusterBuffer(nullptr)
	, m_alreadyRasterizedClusterBuffer(nullptr)
	, m_litVisibleClusterBuffer(nullptr)
	, m_litClusterDebugBuffer(nullptr)
	, m_litTestVoxelBuffer(nullptr)
	, m_distanceShadowMappingTechnique(nullptr)
	, m_litVoxelTechnique(nullptr)
	, m_bufferPrefixSumTechnique(nullptr)
	, m_clusterizationBuildFinalBufferTechnique(nullptr)
	, m_voxelShadowMappingCamera(nullptr)
	, m_prefixSumCompleted(false)
	, m_emitterRadiance(0.0f)
	, m_totalRasterizedCluster(0)
	, m_numAddUpElementPerThread(NUM_ADDUP_ELEMENT_PER_THREAD)
	, m_numAddUpStepThread(0)
	, m_materialLitCluster(nullptr)

	// ResetClusterIrradianceDataTechnique
	, m_materialResetClusterIrradianceData(nullptr)
	, m_litTestClusterBuffer(nullptr)
	, m_litVisibleIndexClusterBuffer(nullptr)
	, m_litToRasterVisibleIndexClusterBuffer(nullptr)
	, m_techniqueLock(false)
	// ResetClusterIrradianceDataTechnique

	// LitClusterProcessResultsTechnique
	, m_materialLitClusterProcessResults(nullptr)
	, m_litClusterCounterValue(0)
	, m_litToRasterVisibleClusterCounterValue(0)
	// LitClusterProcessResultsTechnique
{
	m_active                 = false;
	m_needsToRecord          = false;
	m_executeCommand         = false;
	m_rasterTechniqueType    = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LitClusterTechnique::prepare(float dt)
{
	Camera* shadowMappingCamera = cameraM->getElement(move(string("emitter")));
	mat4 viewprojectionMatrix   = shadowMappingCamera->getProjection() * shadowMappingCamera->getView();
	m_materialLitCluster->setShadowViewProjection(viewprojectionMatrix);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LitClusterTechnique::init()
{
	// ResetClusterIrradianceDataTechnique
	m_litTestClusterBuffer = bufferM->buildBuffer(
		move(string("litTestClusterBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_litVisibleIndexClusterBuffer = bufferM->buildBuffer(
		move(string("litVisibleIndexClusterBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_litToRasterVisibleIndexClusterBuffer = bufferM->buildBuffer(
		move(string("litToRasterVisibleIndexClusterBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_material = materialM->buildMaterial(move(string("MaterialResetClusterIrradianceData")), move(string("MaterialResetClusterIrradianceData")), nullptr);
	m_materialResetClusterIrradianceData = static_cast<MaterialResetClusterIrradianceData*>(m_material);

	m_vectorMaterialName.push_back("MaterialResetClusterIrradianceData");
	m_vectorMaterial.push_back(m_material);
	// ResetClusterIrradianceDataTechnique

	m_litTestVoxelBuffer = bufferM->buildBuffer(
		move(string("litTestVoxelBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_accumulatedIrradianceBuffer = bufferM->buildBuffer(
		move(string("accumulatedIrradianceBuffer")),
		nullptr,
		12,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	uint tempInitValue = 0;

	m_litClusterCounterBuffer = bufferM->buildBuffer(
		move(string("litClusterCounterBuffer")),
		(void*)(&tempInitValue),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_litToRasterVisibleClusterCounterBuffer = bufferM->buildBuffer(
		move(string("litToRasterVisibleClusterCounterBuffer")),
		(void*)(&tempInitValue),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_litToRasterVisibleClusterBuffer = bufferM->buildBuffer(
		move(string("litToRasterVisibleClusterBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_alreadyRasterizedClusterBuffer = bufferM->buildBuffer(
		move(string("alreadyRasterizedClusterBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_litVisibleClusterBuffer = bufferM->buildBuffer(
		move(string("litVisibleClusterBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_litClusterDebugBuffer = bufferM->buildBuffer(
		move(string("litClusterDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	m_material = materialM->buildMaterial(move(string("MaterialLitCluster")), move(string("MaterialLitCluster")), nullptr);

	m_vectorMaterialName.push_back("MaterialLitCluster");
	m_vectorMaterial.push_back(m_material);

	BBox3D& box             = sceneM->refBox();
	vec3 min3D;
	vec3 max3D;
	box.getCenteredBoxMinMax(min3D, max3D);

	vec3 extent3D = max3D - min3D;
	vec4 min      = vec4(min3D.x,    min3D.y,    min3D.z,    0.0f);
	vec4 max      = vec4(max3D.x,    max3D.y,    max3D.z,    0.0f);
	vec4 extent   = vec4(extent3D.x, extent3D.y, extent3D.z, 0.0f);

	m_materialLitCluster = static_cast<MaterialLitCluster*>(m_material);
	m_materialLitCluster->setSceneMin(min);
	m_materialLitCluster->setSceneMax(max);
	m_materialLitCluster->setSceneExtent(extent);
	m_materialLitCluster->setNumAddUpElementPerThread(m_numAddUpElementPerThread);
	
	m_bufferPrefixSumTechnique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_bufferPrefixSumTechnique->refPrefixSumComplete().connect<LitClusterTechnique, &LitClusterTechnique::slotPrefixSumComplete>(this);

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_materialLitCluster->setVoxelSize(float(technique->getVoxelizedSceneWidth()));

	m_clusterizationBuildFinalBufferTechnique = static_cast<ClusterizationBuildFinalBufferTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterizationBuildFinalBufferTechnique"))));
	m_clusterizationBuildFinalBufferTechnique->refSignalClusterizationBuildFinalBufferCompletion().connect<LitClusterTechnique, &LitClusterTechnique::slotClusterizationBuildFinalBufferCompletion>(this);

	m_distanceShadowMappingTechnique = static_cast<DistanceShadowMappingTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("DistanceShadowMappingTechniqueEmitterCamera"))));

	// LitClusterProcessResultsTechnique
	m_materialLitClusterProcessResults = static_cast<MaterialLitClusterProcessResults*>(materialM->buildMaterial(move(string("MaterialLitClusterProcessResults")), move(string("MaterialLitClusterProcessResults")), nullptr));
	m_vectorMaterialName.push_back("MaterialLitClusterProcessResults");
	m_vectorMaterial.push_back(m_materialLitClusterProcessResults);

	m_sceneMin = vec4(min3D.x, min3D.y, min3D.z, 0.0f);
	m_sceneExtentAndVoxelSize = vec4(extent3D.x, extent3D.y, extent3D.z, float(technique->getVoxelizedSceneWidth()));

	m_materialLitClusterProcessResults->setSceneMin(m_sceneMin);
	m_materialLitClusterProcessResults->setSceneExtentAndVoxelSize(m_sceneExtentAndVoxelSize);
	// LitClusterProcessResultsTechnique
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* LitClusterTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
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

	uint32_t offsetData;

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	// LitClusterTechnique record
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialResetClusterIrradianceData->getPipeline()->getPipeline());
	offsetData = static_cast<uint32_t>(m_materialResetClusterIrradianceData->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialResetClusterIrradianceData->getPipelineLayout(), 0, 1, &m_materialResetClusterIrradianceData->refDescriptorSet(), 1, &offsetData);
	vkCmdDispatch(*commandBuffer, m_materialResetClusterIrradianceData->getLocalWorkGroupsXDimension(), m_materialResetClusterIrradianceData->getLocalWorkGroupsYDimension(), 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html
	// LitClusterTechnique record

	// ResetClusterIrradianceDataTechnique record
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialLitCluster->getPipeline()->getPipeline());
	offsetData = static_cast<uint32_t>(m_materialLitCluster->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialLitCluster->getPipelineLayout(), 0, 1, &m_materialLitCluster->refDescriptorSet(), 1, &offsetData);
	vkCmdDispatch(*commandBuffer, m_materialLitCluster->getLocalWorkGroupsXDimension(), m_materialLitCluster->getLocalWorkGroupsYDimension(), 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html
	// ResetClusterIrradianceDataTechnique record

	// LitClusterProcessResultsTechnique record
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialLitClusterProcessResults->getPipeline()->getPipeline());
	offsetData = static_cast<uint32_t>(m_materialLitClusterProcessResults->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialLitClusterProcessResults->getPipelineLayout(), 0, 1, &m_materialLitClusterProcessResults->refDescriptorSet(), 1, &offsetData);
	vkCmdDispatch(*commandBuffer, m_materialLitClusterProcessResults->getLocalWorkGroupsXDimension(), m_materialLitClusterProcessResults->getLocalWorkGroupsYDimension(), 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html
	// LitClusterProcessResultsTechnique record

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);

	// NOTE: Clear command buffer if re-recorded
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LitClusterTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;

	// LitClusterProcessResultsTechnique
	uint tempUint = 0;
	m_litToRasterVisibleClusterCounterBuffer->getContent((void*)(&m_litToRasterVisibleClusterCounterValue));
	m_litToRasterVisibleClusterCounterBuffer->setContent(&tempUint);
	m_litClusterCounterBuffer->getContent((void*)(&m_litClusterCounterValue));
	m_litClusterCounterBuffer->setContent(&tempUint);
	m_litToRasterVisibleClusterCounterBuffer->getContent((void*)(&tempUint));
	// LitClusterProcessResultsTechnique

	m_signalLitClusterCompletion.emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LitClusterTechnique::slotPrefixSumComplete()
{
	m_bufferNumElement = m_bufferPrefixSumTechnique->getFirstIndexOccupiedElement();

	bufferM->resize(m_litTestVoxelBuffer, nullptr, m_bufferNumElement * sizeof(uint));

	m_materialLitCluster->obtainDispatchWorkGroupCount(m_bufferNumElement);
	m_materialLitCluster->setNumOccupiedVoxel(m_bufferNumElement);

	bufferM->resize(m_litClusterDebugBuffer, nullptr, 250 * 60000 * sizeof(uint));
	m_numAddUpStepThread = uint(ceil(float(m_materialLitCluster->getNumThreadExecuted()) / float(m_numAddUpElementPerThread)));

	m_materialLitCluster->setNumAddUpStepThread(m_numAddUpStepThread);

	Camera* shadowMappingCamera = cameraM->getElement(move(string("emitter")));
	mat4 viewprojectionMatrix   = shadowMappingCamera->getProjection() * shadowMappingCamera->getView();
	m_materialLitCluster->setShadowViewProjection(viewprojectionMatrix);

	m_prefixSumCompleted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LitClusterTechnique::slotClusterizationBuildFinalBufferCompletion()
{
	m_bufferNumElement  = m_clusterizationBuildFinalBufferTechnique->getCompactedClusterNumber();
	const int uintSize  = sizeof(uint);
	const int floatSize = sizeof(float);

	m_materialLitCluster->setNumCluster(m_bufferNumElement);

	bufferM->resize(m_litToRasterVisibleClusterBuffer, nullptr, 3 * m_bufferNumElement * uintSize);
	bufferM->resize(m_alreadyRasterizedClusterBuffer,  nullptr,     m_bufferNumElement * uintSize);
	bufferM->resize(m_litVisibleClusterBuffer,         nullptr, 3 * m_bufferNumElement * floatSize);

	// ResetClusterIrradianceDataTechnique
	m_materialResetClusterIrradianceData->obtainDispatchWorkGroupCount(m_bufferNumElement);
	m_materialResetClusterIrradianceData->setNumCluster(m_bufferNumElement);
	m_materialResetClusterIrradianceData->setNumThreadExecuted(m_bufferNumElement);

	bufferM->resize(m_litTestClusterBuffer,                 nullptr,     m_bufferNumElement * uintSize);
	bufferM->resize(m_litVisibleIndexClusterBuffer,         nullptr,     m_bufferNumElement * uintSize);
	bufferM->resize(m_litToRasterVisibleIndexClusterBuffer, nullptr,     m_bufferNumElement * uintSize);

	m_voxelShadowMappingCamera = cameraM->getElement(move(string("emitter")));
	m_voxelShadowMappingCamera->refCameraDirtySignal().connect<LitClusterTechnique, &LitClusterTechnique::slotCameraDirty>(this);
	// ResetClusterIrradianceDataTechnique

	// LitClusterProcessResultsTechnique
	m_materialLitClusterProcessResults->obtainDispatchWorkGroupCount(m_bufferNumElement);
	m_materialLitClusterProcessResults->setNumCluster(m_bufferNumElement);
	m_materialLitClusterProcessResults->setMaximumThreadIndex(m_materialLitClusterProcessResults->getNumThreadExecuted());
	// LitClusterProcessResultsTechnique

	m_newPassRequested = true;
	m_active           = true;
	m_needsToRecord    = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LitClusterTechnique::slotCameraDirty()
{
	// TODO: UPDATE TO USE RESET IRRADIANCE CLUSTER DATA
	if (m_prefixSumCompleted)
	{
		m_cameraPosition  = m_distanceShadowMappingTechnique->getCamera()->getPosition();
		m_cameraForward   = m_distanceShadowMappingTechnique->getCamera()->getLookAt();
		m_emitterRadiance = m_distanceShadowMappingTechnique->getEmitterRadiance();
		
		m_materialLitCluster->setLightPosition(vec4(m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z, 0.0f));
		m_materialLitCluster->setLightForwardEmitterRadiance(vec4(m_cameraForward.x, m_cameraForward.y, m_cameraForward.z, m_emitterRadiance));

		m_techniqueLock  = true;
		m_active         = true;
		m_executeCommand = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

/*void LitClusterTechnique::showClassificationData()
{
	vectorUint8 m_vectorLitVisibleClusterBuffer;
	vectorUint8 m_vectorLitVisibleIndexClusterBuffer;
	vectorUint8 m_vectorLitHiddenClusterBuffer;
	vectorUint8 m_vectorLitHiddenIndexClusterBuffer;

	m_litVisibleClusterBuffer     ->getContentCopy(m_vectorLitVisibleClusterBuffer);
	m_litVisibleIndexClusterBuffer->getContentCopy(m_vectorLitVisibleIndexClusterBuffer);

	vec3 position;
	uvec3 coordinates;

	uint* pLitVisibleClusterBuffer      = (uint*)(m_vectorLitVisibleClusterBuffer.data());
	uint* pLitVisibleIndexClusterBuffer = (uint*)(m_vectorLitVisibleIndexClusterBuffer.data());
	uint* pLitHiddenClusterBuffer       = (uint*)(m_vectorLitHiddenClusterBuffer.data());
	uint* pLitHiddenIndexClusterBuffer  = (uint*)(m_vectorLitHiddenIndexClusterBuffer.data());
	uint voxelizationMaxSize            = 32;

	BBox3D& box = sceneM->refBox();
	vec3 min3D;
	vec3 max3D;
	box.getCenteredBoxMinMax(min3D, max3D);
	vec3 extent3D = max3D - min3D;

	//forI(m_litVisibleClusterCounterValue)
	forI(m_litClusterCounterValue)
	{
		coordinates = BufferVerificationHelper::unhashValue(pLitVisibleClusterBuffer[i], voxelizationMaxSize);
		position    = BufferVerificationHelper::voxelSpaceToWorld(coordinates, vec3(32.0f, 32.0f, 32.0f), extent3D, min3D);
		cout << "Lit voxel index=" << i << ", hashed=" << pLitVisibleClusterBuffer[i] << ", coordinates=" << coordinates.x << "," << coordinates .y << "," << coordinates.z << "), index=" << pLitVisibleIndexClusterBuffer[i] << endl;
	}
}*/

/////////////////////////////////////////////////////////////////////////////////////////////
