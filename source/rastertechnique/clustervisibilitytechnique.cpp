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
#include "../../include/rastertechnique/clustervisibilitytechnique.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/core/gpupipeline.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/scene/scene.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/rastertechnique/clusterizationmergeclustertechnique.h"
#include "../../include/material/materialclustervisibility.h"
#include "../../include/util/bufferverificationhelper.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterVisibilityTechnique::ClusterVisibilityTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_techniquePrefixSum(nullptr)
	, m_techniqueClusterMerge(nullptr)
	, m_clusterVisibilityBuffer(nullptr)
	, m_clusterVisibilityCompactedBuffer(nullptr)
	, m_clusterVisibilityNumberBuffer(nullptr)
	, m_clusterVisibilityFirstIndexBuffer(nullptr)
	, m_clusterVisibilityDebugBuffer(nullptr)
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

void ClusterVisibilityTechnique::init()
{
	m_clusterVisibilityBuffer = bufferM->buildBuffer(
		move(string("clusterVisibilityBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterVisibilityCompactedBuffer = bufferM->buildBuffer(
		move(string("clusterVisibilityCompactedBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterVisibilityNumberBuffer = bufferM->buildBuffer(
		move(string("clusterVisibilityNumberBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterVisibilityFirstIndexBuffer = bufferM->buildBuffer(
		move(string("clusterVisibilityFirstIndexBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterVisibilityDebugBuffer = bufferM->buildBuffer(
		move(string("clusterVisibilityDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();

	MultiTypeUnorderedMap* attributeMaterial = new MultiTypeUnorderedMap();
	attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_clusterVisibilityCodeChunk), string(m_computeShaderThreadMapping)));
	m_material = materialM->buildMaterial(move(string("MaterialClusterVisibility")), move(string("MaterialClusterVisibility")), attributeMaterial);
	m_vectorMaterialName.push_back("MaterialClusterVisibility");
	m_vectorMaterial.push_back(m_material);

	SceneVoxelizationTechnique* sceneVoxelizationTechnique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	
	BBox3D& box = sceneM->refBox();
	
	vec3 max3D;
	vec3 min3D;
	box.getCenteredBoxMinMax(min3D, max3D);
	
	vec3 extent3D             = max3D - min3D;
	m_sceneMin                = vec4(min3D.x,    min3D.y,    min3D.z, 0.0f);
	m_sceneExtent             = vec4(extent3D.x, extent3D.y, extent3D.z, float(sceneVoxelizationTechnique->getVoxelizedSceneWidth()));

	MaterialClusterVisibility* materialCasted = static_cast<MaterialClusterVisibility*>(m_material);
	materialCasted->setSceneExtentAndVoxelSize(m_sceneExtent);

	m_techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_techniquePrefixSum->refPrefixSumComplete().connect<ClusterVisibilityTechnique, &ClusterVisibilityTechnique::slotPrefixSumComplete>(this);

	m_techniqueClusterMerge = static_cast<ClusterizationMergeClusterTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterizationMergeClusterTechnique"))));
	m_techniqueClusterMerge->refSignalClusterizationMergeClusterCompletion().connect<ClusterVisibilityTechnique, &ClusterVisibilityTechnique::slotClusterMergeComplete>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* ClusterVisibilityTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
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

	MaterialClusterVisibility* castedBounce     = static_cast<MaterialClusterVisibility*>(m_vectorMaterial[0]);

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

void ClusterVisibilityTechnique::postCommandSubmit()
{
	m_clusterVisibilityCompletion.emit();

	//BufferVerificationHelper::verifyClusterVisibilityIndices();

	m_executeCommand = false;
	m_active         = false;
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterVisibilityTechnique::slotPrefixSumComplete()
{
	m_numOccupiedVoxel = m_techniquePrefixSum->getFirstIndexOccupiedElement();
	m_bufferNumElement = m_numOccupiedVoxel * m_numThreadPerLocalWorkgroup * 6; // Each local workgroup will work one side of each voxel in the scene
	m_sceneMin.w       = float(m_numOccupiedVoxel);

	obtainDispatchWorkGroupCount();

	MaterialClusterVisibility* materialCasted = static_cast<MaterialClusterVisibility*>(m_material);
	materialCasted->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	materialCasted->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	// TODO: Remove the numebr voxel part and set this value in ClusterVisibilityTechnique::init
	materialCasted->setSceneMinAndNumberVoxel(m_sceneMin);
	materialCasted->setNumThreadExecuted(m_bufferNumElement);

	bufferM->resize(m_clusterVisibilityBuffer, nullptr, m_bufferNumElement * sizeof(uint));

	// Initialize clusterVisibilityBuffer with maxValue (4294967295) so the cluster with index 0 can be identified
	vector<uint> vectorData;
	vectorData.resize(m_bufferNumElement);
	memset(vectorData.data(), maxValue, vectorData.size() * size_t(sizeof(uint)));
	m_clusterVisibilityBuffer->setContent(vectorData.data());

	// Make a buffer with six elements per occupied voxel (to store the amount of visible clusters
	// from each voxel face)
	bufferM->resize(m_clusterVisibilityNumberBuffer, nullptr, m_numOccupiedVoxel * 6 * sizeof(uint));
	// Resize to have, for each occupied voxel, six elements (one per voxel face), where to store the start
	// index of the visible clusters stored in clusterVisibilityCompactedBuffer for each voxel face
	bufferM->resize(m_clusterVisibilityFirstIndexBuffer, nullptr, m_numOccupiedVoxel * 6 * sizeof(uint));

	// TODO: Avoid this resize to save memory once debug is done
	bufferM->resize(m_clusterVisibilityDebugBuffer, nullptr, 725000 * sizeof(uint));

	m_prefixSumCompleted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterVisibilityTechnique::slotClusterMergeComplete()
{
	if (m_prefixSumCompleted)
	{
		m_vectorCommand.clear();
		m_active = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
