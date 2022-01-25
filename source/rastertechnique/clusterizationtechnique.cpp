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
#include "../../include/rastertechnique/clusterizationtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialclusterization.h"
#include "../../include/material/materialclusterizationnewcenter.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/scene/scene.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/material/materialclusterizationinitvoxeldistance.h"
#include "../../include/material/materialclusterizationaddup.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterizationTechnique::ClusterizationTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_sceneVoxelizationTechnique(nullptr)
	, m_fragmentCounter(0)
	, m_prefixSumCompleted(false)
	, m_voxelizationSize(-1)
	, m_voxelizationSizeFloat(-1.0f)
	, m_numOccupiedVoxel(-1)
	, m_bufferPrefixSumTechnique(nullptr)
	, m_voxelClusterOwnerIndexBuffer(nullptr)
	, m_voxelClusterOwnerDistanceBuffer(nullptr)
	, m_clusterizationCenterCoordinatesBuffer(nullptr)
	, m_clusterizationCenterPositiveDirectionBuffer(nullptr)
	, m_clusterizationCenterNegativeDirectionBuffer(nullptr)
	, m_clusterizationCenterPositivePreviousDirectionBuffer(nullptr)
	, m_clusterizationCenterNegativePreviousDirectionBuffer(nullptr)
	, m_clusterizationCenterCountsBuffer(nullptr)
	, m_clusterizationDebugBuffer(nullptr)
	, m_clusterizationNewCenterDebugBuffer(nullptr)
	, m_clusterizationAddUpDebugBuffer(nullptr)
	, m_iterationCounterBuffer(nullptr)
	, m_clusterizationStep(-1.0f)
	, m_materialClusterization(nullptr)
	, m_materialNewCenter(nullptr)
	, m_materialInitVoxelDistance(nullptr)
	, m_materialAddUp(nullptr)
	, m_numClusterizationStep(CLUSTERIZATION_NUM_ITERATION)
	, m_iterationCounterValue(0)
{
	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationTechnique::init()
{
	m_sceneVoxelizationTechnique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_voxelizationSize           = max(m_sceneVoxelizationTechnique->getVoxelizedSceneWidth(), max(m_sceneVoxelizationTechnique->getVoxelizedSceneHeight(), m_sceneVoxelizationTechnique->getVoxelizedSceneDepth()));
	m_voxelizationSizeFloat      = float(m_voxelizationSize);

	// Shader storage buffer with the indices of the elements present in the buffer litHiddenVoxelBuffer
	buildShaderThreadMapping();

	m_bufferPrefixSumTechnique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_bufferPrefixSumTechnique->refPrefixSumComplete().connect<ClusterizationTechnique, &ClusterizationTechnique::slotPrefixSumComplete>(this);

	m_voxelClusterOwnerIndexBuffer = bufferM->buildBuffer(
		move(string("voxelClusterOwnerIndexBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_voxelClusterOwnerDistanceBuffer = bufferM->buildBuffer(
		move(string("voxelClusterOwnerDistanceBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationCenterCoordinatesBuffer = bufferM->buildBuffer(
		move(string("clusterizationCenterCoordinatesBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationCenterPositiveDirectionBuffer = bufferM->buildBuffer(
		move(string("clusterizationCenterPositiveDirectionBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationCenterNegativeDirectionBuffer = bufferM->buildBuffer(
		move(string("clusterizationCenterNegativeDirectionBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationCenterPositivePreviousDirectionBuffer = bufferM->buildBuffer(
		move(string("clusterizationCenterPositivePreviousDirectionBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationCenterNegativePreviousDirectionBuffer = bufferM->buildBuffer(
		move(string("clusterizationCenterNegativePreviousDirectionBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationCenterCountsBuffer = bufferM->buildBuffer(
		move(string("clusterizationCenterCountsBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationDebugBuffer = bufferM->buildBuffer(
		move(string("clusterizationDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationNewCenterDebugBuffer = bufferM->buildBuffer(
		move(string("clusterizationNewCenterDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationAddUpDebugBuffer = bufferM->buildBuffer(
		move(string("clusterizationAddUpDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_iterationCounterBuffer = bufferM->buildBuffer(
		move(string("iterationCounterBuffer")),
		(void*)(&m_iterationCounterValue),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	bufferM->buildBuffer(
		move(string("clusterizationDebugVoxelIndexBuffer")),
		(void*)(&m_iterationCounterValue),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	MultiTypeUnorderedMap* attributeMaterialClusterization = new MultiTypeUnorderedMap();
	attributeMaterialClusterization->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationCodeChunk), string(m_computeShaderThreadMapping)));
	m_material = materialM->buildMaterial(move(string("MaterialClusterization")), move(string("MaterialClusterization")), attributeMaterialClusterization);
	m_vectorMaterialName.push_back("MaterialClusterization");
	m_vectorMaterial.push_back(m_material);

	BBox3D& sceneAABB = sceneM->refBox();

	vec3 m;
	vec3 M;
	sceneAABB.getCenteredBoxMinMax(m, M);
	m_sceneMin         = vec4(m.x, m.y, m.z, 0.0f);
	vec3 sceneExtent3D = sceneAABB.getMax() - sceneAABB.getMin();
	m_sceneExtent      = vec4(sceneExtent3D.x, sceneExtent3D.y, sceneExtent3D.z, 0.0f);

	m_materialClusterization = static_cast<MaterialClusterization*>(m_material);
	m_materialClusterization->setSceneMin(m_sceneMin);
	m_materialClusterization->setSceneExtent(m_sceneExtent);

	MultiTypeUnorderedMap* attributeMaterialAddUpNewCenter = new MultiTypeUnorderedMap();
	attributeMaterialAddUpNewCenter->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationNewCenterCodeChunk), string(m_computeShaderThreadMapping)));
	Material* materialNewCenter = materialM->buildMaterial(move(string("MaterialClusterizationNewCenter")), move(string("MaterialClusterizationNewCenter")), attributeMaterialAddUpNewCenter);
	m_materialNewCenter         = static_cast<MaterialClusterizationNewCenter*>(materialNewCenter);
	m_vectorMaterialName.push_back("MaterialClusterizationNewCenter");
	m_vectorMaterial.push_back(materialNewCenter);

	MultiTypeUnorderedMap* attributeMaterialInitVoxelDistance = new MultiTypeUnorderedMap();
	attributeMaterialInitVoxelDistance->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationInitVoxelDistanceCodeChunk), string(m_computeShaderThreadMapping)));
	Material* materialInitVoxelDistance = materialM->buildMaterial(move(string("MaterialClusterizationInitVoxelDistance")), move(string("MaterialClusterizationInitVoxelDistance")), attributeMaterialInitVoxelDistance);
	m_materialInitVoxelDistance = static_cast<MaterialClusterizationInitVoxelDistance*>(materialInitVoxelDistance);
	m_vectorMaterialName.push_back("MaterialClusterizationInitVoxelDistance");
	m_vectorMaterial.push_back(materialInitVoxelDistance);

	MultiTypeUnorderedMap* attributeMaterialAddUp = new MultiTypeUnorderedMap();
	attributeMaterialAddUp->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationAddUpCodeChunk), string(m_computeShaderThreadMapping)));
	Material* materialAddUp = materialM->buildMaterial(move(string("MaterialClusterizationAddUp")), move(string("MaterialClusterizationAddUp")), attributeMaterialAddUp);
	m_materialAddUp         = static_cast<MaterialClusterizationAddUp*>(materialAddUp);
	m_vectorMaterialName.push_back("MaterialClusterizationAddUp");
	m_vectorMaterial.push_back(m_materialAddUp);
}


/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* ClusterizationTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	m_materialClusterization    = static_cast<MaterialClusterization*>(m_vectorMaterial[0]);
	m_materialNewCenter         = static_cast<MaterialClusterizationNewCenter*>(m_vectorMaterial[1]);
	m_materialInitVoxelDistance = static_cast<MaterialClusterizationInitVoxelDistance*>(m_vectorMaterial[2]);
	m_materialAddUp             = static_cast<MaterialClusterizationAddUp*>(m_vectorMaterial[3]);

	commandBufferType = CommandBufferType::CBT_COMPUTE_QUEUE;

	VkCommandBuffer* commandBuffer;
	addCommandBufferQueueType(commandBufferID, commandBufferType);
	commandBuffer = addRecordedCommandBuffer(commandBufferID);
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getComputeCommandPool(), commandBuffer);

	coreM->beginCommandBuffer(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex0);
#endif

	vectorBufferPtr vectorBuffer;
	vectorBuffer.resize(7);
	// TODO: set barriers only for those memory buffers needed in each step
	vectorBuffer[0] = m_clusterizationCenterCoordinatesBuffer;
	vectorBuffer[1] = m_clusterizationCenterPositiveDirectionBuffer;
	vectorBuffer[2] = m_clusterizationCenterNegativeDirectionBuffer;
	vectorBuffer[3] = m_clusterizationCenterCountsBuffer;
	vectorBuffer[4] = m_clusterizationDebugBuffer;
	vectorBuffer[5] = m_clusterizationCenterPositivePreviousDirectionBuffer;
	vectorBuffer[6] = m_clusterizationCenterNegativePreviousDirectionBuffer;
	// TODO: remove debug buffers when properly working and debugged

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	uint32_t offsetData;
	forI(m_numClusterizationStep)
	{
		// Init voxel distance buffer
		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialInitVoxelDistance->getPipeline()->getPipeline());
		offsetData = static_cast<uint32_t>(m_materialInitVoxelDistance->getMaterialUniformBufferIndex() * dynamicAllignment);
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialInitVoxelDistance->getPipelineLayout(), 0, 1, &m_materialInitVoxelDistance->refDescriptorSet(), 1, &offsetData);
		vkCmdDispatch(*commandBuffer, m_localWorkGroupsXDimension, m_localWorkGroupsYDimension, 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

		VulkanStructInitializer::insertBufferMemoryBarrier(vectorBuffer,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   commandBuffer);

		// Strange bug: if trying to upload value 0, a not proper value would arrive at the push constant in the shader...
		m_materialClusterization->setPushConstantIterationStep(vec4(float(i + 1), 0.0f, 0.0f, 0.0f));
		m_materialClusterization->updatePushConstantCPUBuffer();
		CPUBuffer& cpuBuffer = m_materialClusterization->refShader()->refPushConstant().refCPUBuffer();

		vkCmdPushConstants(
			*commandBuffer,
			m_materialClusterization->getPipelineLayout(),
			VK_SHADER_STAGE_COMPUTE_BIT,
			0,
			uint32_t(m_materialClusterization->getPushConstantExposedStructFieldSize()),
			cpuBuffer.refUBHostMemory());
		
		// Clusterization step
		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialClusterization->getPipeline()->getPipeline());
		offsetData = static_cast<uint32_t>(m_materialClusterization->getMaterialUniformBufferIndex() * dynamicAllignment);
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialClusterization->getPipelineLayout(), 0, 1, &m_materialClusterization->refDescriptorSet(), 1, &offsetData);
		vkCmdDispatch(*commandBuffer, m_localWorkGroupsXDimension, m_localWorkGroupsYDimension, 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

		VulkanStructInitializer::insertBufferMemoryBarrier(vectorBuffer,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   commandBuffer);

		// Add up cluster position and normal direction
		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialAddUp->getPipeline()->getPipeline());
		offsetData = static_cast<uint32_t>(m_materialAddUp->getMaterialUniformBufferIndex() * dynamicAllignment);
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialAddUp->getPipelineLayout(), 0, 1, &m_materialAddUp->refDescriptorSet(), 1, &offsetData);
		vkCmdDispatch(*commandBuffer, m_localWorkGroupsXDimension, m_localWorkGroupsYDimension, 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

		VulkanStructInitializer::insertBufferMemoryBarrier(vectorBuffer,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   commandBuffer);

		m_materialNewCenter->setPushConstantIterationStep(vec4(float(i + 1), 0.0f, 0.0f, 0.0f));
		m_materialNewCenter->updatePushConstantCPUBuffer();
		CPUBuffer& cpuBufferNewCenter = m_materialNewCenter->refShader()->refPushConstant().refCPUBuffer();

		vkCmdPushConstants(
			*commandBuffer,
			m_materialNewCenter->getPipelineLayout(),
			VK_SHADER_STAGE_COMPUTE_BIT,
			0,
			uint32_t(m_materialNewCenter->getPushConstantExposedStructFieldSize()),
			cpuBufferNewCenter.refUBHostMemory());

		// Compute new cluster center
		vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialNewCenter->getPipeline()->getPipeline());
		offsetData = static_cast<uint32_t>(m_materialNewCenter->getMaterialUniformBufferIndex() * dynamicAllignment);
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_materialNewCenter->getPipelineLayout(), 0, 1, &m_materialNewCenter->refDescriptorSet(), 1, &offsetData);
		vkCmdDispatch(*commandBuffer, m_localWorkGroupsXDimension, m_localWorkGroupsYDimension, 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

		VulkanStructInitializer::insertBufferMemoryBarrier(vectorBuffer,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
														   commandBuffer);
	}

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);

	// NOTE: Clear command buffer if re-recorded
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;
	m_signalResetRadianceDataCompletion.emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////

int ClusterizationTechnique::getNumberSuperVoxel(int voxelizationSize)
{
	int superPixelNumber = -1;

	switch (voxelizationSize)
	{
	case 32:
	{
		superPixelNumber = 25000; // Generates 4913 elements, 4913 * 0.1 = 491 elements with a step value of 2 units
		break;
	}
	case 64:
	{
		superPixelNumber = 30000; // Generates 29791 elements, 29791 * 0.1 = 2979 elements with a step value of 3 units
		break;
	}
	case 128:
	{
		superPixelNumber = 250000; // Generates 117649 elements, 117649 * 0.1 = 11765 elements with a step value of 11 units
		break;
	}
	case 256:
	{
		superPixelNumber = 120000; // Generates 117649 elements, 117649 * 0.1 = 11765 elements with a step value of 11 units
		break;
	}
	case 512:
	{
		superPixelNumber = 250000; // Generates 117649 elements, 117649 * 0.1 = 11765 elements with a step value of 11 units
		break;
	}
	default:
	{
		cout << "ERROR: no voxelizationSize value in ClusterizationTechnique::getNumberSuperVoxel" << endl;
		break;
	}
	}

	cout << "The number of super pixels for the clusterization step is " << superPixelNumber << endl;

	return superPixelNumber;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationTechnique::slotPrefixSumComplete()
{
	m_materialClusterization    = static_cast<MaterialClusterization*>(m_vectorMaterial[0]);
	m_materialNewCenter         = static_cast<MaterialClusterizationNewCenter*>(m_vectorMaterial[1]);
	m_materialInitVoxelDistance = static_cast<MaterialClusterizationInitVoxelDistance*>(m_vectorMaterial[2]);
	m_materialAddUp             = static_cast<MaterialClusterizationAddUp*>(m_vectorMaterial[3]);

	m_active             = true;
	m_prefixSumCompleted = true;
	int superPixelNumber = getNumberSuperVoxel(m_voxelizationSize);
	m_numOccupiedVoxel   = m_bufferPrefixSumTechnique->getFirstIndexOccupiedElement();
	m_bufferNumElement   = superPixelNumber;
	int bufferSize       = m_numOccupiedVoxel * sizeof(uint);
	int bufferCenterSize = superPixelNumber * sizeof(uint);
	m_clusterizationStep = pow((m_voxelizationSizeFloat * m_voxelizationSizeFloat * m_voxelizationSizeFloat) / float(superPixelNumber), 1.0f / 3.0f);

	// TODO: avoid generating all the needed memory to set the value of the m_voxelClusterOwnerIndexBuffer and do it through a proper
	// compute shader dispatch

	vectorInt vectorVoxelClusterOwnerIndex;
	vectorVoxelClusterOwnerIndex.resize(m_numOccupiedVoxel);
	forI(vectorVoxelClusterOwnerIndex.size())
	{
		vectorVoxelClusterOwnerIndex[i] = -1;
	}

	bufferM->resize(m_voxelClusterOwnerIndexBuffer,                        vectorVoxelClusterOwnerIndex.data(), bufferSize);
	bufferM->resize(m_voxelClusterOwnerDistanceBuffer,                     nullptr, bufferSize);
	bufferM->resize(m_clusterizationCenterCoordinatesBuffer,               nullptr, bufferCenterSize * 3);
	bufferM->resize(m_clusterizationCenterPositiveDirectionBuffer,         nullptr, bufferCenterSize * 3);
	bufferM->resize(m_clusterizationCenterNegativeDirectionBuffer,         nullptr, bufferCenterSize * 3);
	bufferM->resize(m_clusterizationCenterPositivePreviousDirectionBuffer, nullptr, bufferCenterSize * 3);
	bufferM->resize(m_clusterizationCenterNegativePreviousDirectionBuffer, nullptr, bufferCenterSize * 3);
	bufferM->resize(m_clusterizationCenterCountsBuffer,                    nullptr, bufferCenterSize);

	obtainDispatchWorkGroupCount();

	m_materialClusterization->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialClusterization->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialClusterization->setNumElement(superPixelNumber);
	m_materialClusterization->setSuperPixelNumber(superPixelNumber);
	m_materialClusterization->setNumOccupiedVoxel(m_numOccupiedVoxel);
	m_materialClusterization->setClusterizationStep(m_clusterizationStep);
	m_materialClusterization->setVoxelSize(float(m_voxelizationSize));

	m_materialNewCenter->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialNewCenter->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialNewCenter->setNumElement(superPixelNumber);
	m_materialNewCenter->setClusterizationStep(m_clusterizationStep);
	m_materialNewCenter->setVoxelSize(float(m_voxelizationSize));

	m_materialInitVoxelDistance->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialInitVoxelDistance->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialInitVoxelDistance->setSuperPixelNumber(superPixelNumber);
	m_materialInitVoxelDistance->setNumOccupiedVoxel(m_numOccupiedVoxel);

	m_materialAddUp->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialAddUp->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialAddUp->setSuperPixelNumber(superPixelNumber);
	m_materialAddUp->setNumOccupiedVoxel(m_numOccupiedVoxel);
	m_materialAddUp->setVoxelSize(m_voxelizationSize);

	m_newPassRequested = true;
	m_active           = true;
	m_needsToRecord    = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
