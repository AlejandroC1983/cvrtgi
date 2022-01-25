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
#include "../../include/core/gpupipeline.h"
#include "../../include/core/coremanager.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/scene/scene.h"
#include "../../include/pipeline/pipeline.h"
#include "../../include/uniformbuffer/uniformbuffermanager.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/rastertechnique/rastertechnique.h"
#include "../../include/rastertechnique/rastertechniquemanager.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/framebuffer/framebuffermanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES
const uint perVertexNumElement = 12;

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

GPUPipeline::GPUPipeline():
	  m_pipelineInitialized(false)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

GPUPipeline::~GPUPipeline()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::init()
{
	shaderM->obtainMaxPushConstantsSize();

	vectorUint arrayIndices;
	vectorFloat arrayVertexData;
	buildSceneBufferData(arrayIndices, arrayVertexData);
	createVertexBuffer(arrayIndices, arrayVertexData);
	setVertexInput(48); // Size in bytes of each vertex information. Currently: position, uv + ID, normal and tangent (vec3, vec3, vec3, vec3), 48 bytes
	
	// Create the vertex and fragment shader
	createPipelineCache();
	createSceneDataUniformBuffer();
	createSceneCameraUniformBuffer();

	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("SceneVoxelizationTechnique"),              string("SceneVoxelizationTechnique"),               nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ComputeFrustumCullingTechnique"),          string("ComputeFrustumCullingTechnique"),              nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("SceneIndirectDrawTechnique"),              string("SceneIndirectDrawTechnique"),                  nullptr));
	//m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("SceneRasterColorTextureTechnique"),        string("SceneRasterColorTextureTechnique"),         nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("BufferPrefixSumTechnique"),                string("BufferPrefixSumTechnique"),                 nullptr));
	//m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("BuildVoxelShadowMapGeometryTechnique"),    string("BuildVoxelShadowMapGeometryTechnique"),     nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterizationPrepareTechnique"),          string("ClusterizationPrepareTechnique"),           nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterizationInitAABBTechnique"),         string("ClusterizationInitAABBTechnique"),          nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterizationTechnique"),                 string("ClusterizationTechnique"),                  nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterizationComputeAABBTechnique"),      string("ClusterizationComputeAABBTechnique"),       nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterizationBuildFinalBufferTechnique"), string("ClusterizationBuildFinalBufferTechnique"),  nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterizationComputeNeighbourTechnique"), string("ClusterizationComputeNeighbourTechnique"),  nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterizationMergeClusterTechnique"),     string("ClusterizationMergeClusterTechnique"),      nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterVisibilityTechnique"),              string("ClusterVisibilityTechnique"),               nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ClusterVisiblePrefixSumTechnique"),        string("ClusterVisiblePrefixSumTechnique"),         nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("VoxelFacePenaltyTechnique"),               string("VoxelFacePenaltyTechnique"),                nullptr));
	//m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("ShadowMappingVoxelTechnique"),             string("ShadowMappingVoxelTechnique"),              nullptr));

	MultiTypeUnorderedMap* attribute = new MultiTypeUnorderedMap();
	attribute->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapDistanceTextureCodeChunk),      string("distanceShadowMappingTexture")));
	attribute->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapOffscreenTextureCodeChunk),     string("offscreenDistanceDepthTexture")));
	attribute->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapMaterialNameCodeChunk),         string("MaterialDistanceShadowMapping")));
	attribute->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapFramebufferNameCodeChunk),      string("distanceshadowmapframebuffer")));
	attribute->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapCameraNameCodeChunk),           string("maincamera")));
	attribute->newElement<AttributeData<bool>*>(  new AttributeData<bool>(  string(g_distanceShadowMapUseCompactedGeometryCodeChunk), bool(true)));

	MultiTypeUnorderedMap* attribute2 = new MultiTypeUnorderedMap();
	attribute2->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapDistanceTextureCodeChunk),      string("mainCameradistanceShadowMappingTexture")));
	attribute2->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapOffscreenTextureCodeChunk),     string("mainCameraOffscreenDistanceDepthTexture")));
	attribute2->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapMaterialNameCodeChunk),         string("MainCameraMaterialDistanceShadowMapping")));
	attribute2->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapFramebufferNameCodeChunk),      string("mainCameraDistanceshadowmapframebuffer")));
	attribute2->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_distanceShadowMapCameraNameCodeChunk),           string("emitter")));
	attribute2->newElement<AttributeData<bool>*>(  new AttributeData<bool>(  string(g_distanceShadowMapUseCompactedGeometryCodeChunk), bool(false)));

	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("DistanceShadowMappingTechnique"),          string("DistanceShadowMappingTechniqueMainCamera"),    attribute));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("DistanceShadowMappingTechnique"),          string("DistanceShadowMappingTechniqueEmitterCamera"), attribute2));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("LitClusterTechnique"),                     string("LitClusterTechnique"),                         nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("CameraVisibleVoxelTechnique"),             string("CameraVisibleVoxelTechnique"),                 nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("LightBounceVoxelIrradianceTechnique"),     string("LightBounceVoxelIrradianceTechnique"),         nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("SceneLightingTechnique"),                  string("SceneLightingTechnique"),                      nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("VoxelRasterInScenarioTechnique"),          string("VoxelRasterInScenarioTechnique"),              nullptr));
	m_vectorRasterTechnique.push_back(rasterTechniqueM->buildNewRasterTechnique(string("AntialiasingTechnique"),                   string("AntialiasingTechnique"),                       nullptr));

	m_vectorReRecordFlags.resize(m_vectorRasterTechnique.size());
	fill(m_vectorReRecordFlags.begin(), m_vectorReRecordFlags.end(), false);

	materialM->buildMaterialUniformBuffer();

	const vector<Material*>& vectorMaterial = materialM->getVectorElement();
	forI(vectorMaterial.size())
	{
		vectorMaterial[i]->buildPipeline();
	}

	forIT(m_vectorRasterTechnique)
	{
		(*it)->generateSempahore(); // TODO: Set as protected and call from other part of the framework
	}

	m_pipelineInitialized = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::createVertexBuffer(vectorUint& arrayIndices, vectorFloat& arrayVertexData)
{
	Buffer* buffer = bufferM->buildBuffer(
		move(string("vertexBuffer")),
		(void *)arrayVertexData.data(),
		sizeof(float) * arrayVertexData.size(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	buffer = bufferM->buildBuffer(
		move(string("indexBuffer")),
		(void *)arrayIndices.data(),
		sizeof(uint) * arrayIndices.size(),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::setVertexInput(uint32_t dataStride)
{
	// The VkVertexInputBinding viIpBind, stores the rate at which the information will be
	// injected for vertex input.
	m_viIpBind.binding   = 0;
	m_viIpBind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	m_viIpBind.stride    = dataStride;

	// The VkVertexInputAttribute - Description) structure, store 
	// the information that helps in interpreting the data.
	m_viIpAttrb[0].binding  = 0;
	m_viIpAttrb[0].location = 0;
	m_viIpAttrb[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
	m_viIpAttrb[0].offset   = 0;
	m_viIpAttrb[1].binding  = 0;
	m_viIpAttrb[1].location = 1;
	m_viIpAttrb[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
	m_viIpAttrb[1].offset   = 12; // After, 4 components - RGBA  each of 4 bytes(32bits)
	m_viIpAttrb[2].binding  = 0;
	m_viIpAttrb[2].location = 2;
	m_viIpAttrb[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
	m_viIpAttrb[2].offset   = 24;
	m_viIpAttrb[3].binding  = 0;
	m_viIpAttrb[3].location = 3;
	m_viIpAttrb[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
	m_viIpAttrb[3].offset   = 36;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::initViewports(float width, float height, float offsetX, float offsetY, float minDepth, float maxDepth, VkCommandBuffer* cmd)
{
	VkViewport m_viewport;
	m_viewport.width    = width;
	m_viewport.height   = height;
	m_viewport.minDepth = minDepth;
	m_viewport.maxDepth = maxDepth;
	m_viewport.x        = offsetX;
	m_viewport.y        = offsetY;
	vkCmdSetViewport(*cmd, 0, NUMBER_OF_VIEWPORTS, &m_viewport);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::initScissors(uint width, uint height, int offsetX, int offsetY, VkCommandBuffer* cmd)
{
	VkRect2D m_scissor;
	m_scissor.extent.width  = width;
	m_scissor.extent.height = height;
	m_scissor.offset.x      = offsetX;
	m_scissor.offset.y      = offsetY;
	vkCmdSetScissor(*cmd, 0, NUMBER_OF_SCISSORS, &m_scissor);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::update()
{
	// TODO: Only for dynamic scenes, avoid in other case.
	const vectorNodePtr& arrayModel = sceneM->getModel();
	mat4 Model;
	mat4 MVP;
	forI(arrayModel.size())
	{
		Model = arrayModel[i]->getModelMat();
		m_sceneUniformData->refCPUBuffer().resetDataAtCell(i);
		m_sceneUniformData->refCPUBuffer().appendDataAtCell<mat4>(i, Model);
	}

	m_sceneUniformData->uploadCPUBufferToGPU();

	// Update scene camera information

	mat4 viewMatrix       = cameraM->refMainCamera()->getView();
	mat4 projectionMatrix = cameraM->refMainCamera()->getProjection();
	BBox3D& sceneAABB     = sceneM->refBox();
	vec3 sceneOffset      = vec3(.0f) - sceneAABB.getMin();
	vec3 sceneExtent      = sceneAABB.getMax() - sceneAABB.getMin();

	m_sceneCameraUniformData->refCPUBuffer().resetDataAtCell(0);
	m_sceneCameraUniformData->refCPUBuffer().appendDataAtCell<mat4>(0, viewMatrix);
	m_sceneCameraUniformData->refCPUBuffer().appendDataAtCell<mat4>(0, projectionMatrix);
	m_sceneCameraUniformData->refCPUBuffer().appendDataAtCell<vec4>(0, vec4(sceneOffset.x, sceneOffset.y, sceneOffset.z, 0.0f));
	m_sceneCameraUniformData->refCPUBuffer().appendDataAtCell<vec4>(0, vec4(sceneExtent.x, sceneExtent.y, sceneExtent.z, 0.0f));
	m_sceneCameraUniformData->uploadCPUBufferToGPU();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::buildSceneBufferData(vectorUint& arrayIndices, vectorFloat& arrayVertexData)
{
	arrayIndices.clear();
	// All mesh per vertex data
	vectorNodePtr vAll3DModels;
	vAll3DModels.insert(vAll3DModels.begin(), sceneM->getModel().begin(), sceneM->getModel().end());
	vAll3DModels.insert(vAll3DModels.begin(), sceneM->getLightVolumes().begin(), sceneM->getLightVolumes().end());
	forIT(vAll3DModels)
	{
		// First, per vertex data stored in Node::m_vertexData
		vectorUint &pMI = (*it)->refIndices();
		vectorFloat &pVD = (*it)->refVertexData();
		uint uIndexOffset = uint(arrayVertexData.size()) / perVertexNumElement;
		(*it)->setStartIndex(uint(arrayIndices.size()));
		arrayVertexData.insert(arrayVertexData.end(), (*it)->refVertexData().begin(), (*it)->refVertexData().end());
		assert((*it)->refVertexData().size() != 0);

		// Add the offset to the indices being used, as all the indices of all the scene meshes will be in the same buffer
		forJT((*it)->refIndices())
		{
			(*jt) += uIndexOffset;
		}

		arrayIndices.insert(arrayIndices.end(), (*it)->refIndices().begin(), (*it)->refIndices().end());
		(*it)->setEndIndex(uint(arrayIndices.size()));
		(*it)->setIndexSize(uint((*it)->refIndices().size()));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::createPipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheInfo;
	pipelineCacheInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.pNext           = NULL;
	pipelineCacheInfo.initialDataSize = 0;
	pipelineCacheInfo.pInitialData    = NULL;
	pipelineCacheInfo.flags           = 0;

	VkResult result = vkCreatePipelineCache(coreM->getLogicalDevice(), &pipelineCacheInfo, NULL, &m_pipelineCache);
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::createSceneDataUniformBuffer()
{
	m_sceneUniformData = uniformBufferM->buildUniformBuffer(move(string("sceneUniformBuffer")), sizeof(mat4), int(sceneM->getModel().size()));

	mat4 Model = mat4(1.0f);
	const vectorNodePtr& arrayModel = sceneM->getModel();
	forI(arrayModel.size())
	{
		m_sceneUniformData->refCPUBuffer().appendDataAtCell<mat4>(i, Model);
	}

	m_sceneUniformData->uploadCPUBufferToGPU();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::createSceneCameraUniformBuffer()
{
	m_sceneCameraUniformData = uniformBufferM->buildUniformBuffer(move(string("sceneCameraUniformBuffer")), 2 * sizeof(mat4), 1);
	m_sceneCameraUniformData->uploadCPUBufferToGPU();
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkDescriptorSet GPUPipeline::buildDescriptorSet(vector<VkDescriptorType> vectorDescriptorType,
	vector<uint32_t> vectorBindingIndex,
	vector<VkShaderStageFlags> vectorStageFlags,
	VkDescriptorSetLayout& descriptorSetLayout,
	VkDescriptorPool& descriptorPool)
{
	vector<VkDescriptorSetLayoutBinding> vectorDescriptorSetLayoutBinding;

	forI(vectorDescriptorType.size())
	{
		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
		descriptorSetLayoutBinding.binding            = vectorBindingIndex[i];
		descriptorSetLayoutBinding.descriptorType     = vectorDescriptorType[i];
		descriptorSetLayoutBinding.descriptorCount    = 1;
		descriptorSetLayoutBinding.stageFlags         = vectorStageFlags[i];
		descriptorSetLayoutBinding.pImmutableSamplers = NULL;

		vectorDescriptorSetLayoutBinding.push_back(descriptorSetLayoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext        = NULL;
	descriptorLayout.bindingCount = uint(vectorDescriptorSetLayoutBinding.size());
	descriptorLayout.pBindings    = vectorDescriptorSetLayoutBinding.data();
	VkResult result = vkCreateDescriptorSetLayout(coreM->getLogicalDevice(), &descriptorLayout, NULL, &descriptorSetLayout);
	assert(result == VK_SUCCESS);

	vector<VkDescriptorPoolSize> vectorDescriptorPoolSize;
	forI(vectorStageFlags.size())
	{
		VkDescriptorPoolSize descriptorTypePool = { vectorDescriptorType[i], 1 };
		vectorDescriptorPoolSize.push_back(descriptorTypePool);
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext         = NULL;
	descriptorPoolCreateInfo.maxSets       = 1;
	descriptorPoolCreateInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // 0;
	descriptorPoolCreateInfo.poolSizeCount = (uint32_t)vectorDescriptorPoolSize.size();
	descriptorPoolCreateInfo.pPoolSizes    = vectorDescriptorPoolSize.data();
	result = vkCreateDescriptorPool(coreM->getLogicalDevice(), &descriptorPoolCreateInfo, NULL, &descriptorPool);
	assert(result == VK_SUCCESS);

	VkDescriptorSet descriptorSet;
	VkDescriptorSetAllocateInfo dsAllocInfo;
	dsAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsAllocInfo.pNext              = NULL;
	dsAllocInfo.descriptorPool     = descriptorPool;
	dsAllocInfo.descriptorSetCount = 1;
	dsAllocInfo.pSetLayouts        = &descriptorSetLayout;
	result = vkAllocateDescriptorSets(coreM->getLogicalDevice(), &dsAllocInfo, &descriptorSet);
	assert(result == VK_SUCCESS);

	return descriptorSet;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::updateDescriptorSet(
	const VkDescriptorSet&          descriptorSet,
	const vector<VkDescriptorType>& vectorDescriptorType,
	const vector<void*>&            vectorDescriptorInfo,
	const vector<int>&              vectorDescriptorInfoHint,
	const vector<uint32_t>&         vectorBinding)
{
	vector<VkWriteDescriptorSet> vectorWriteDescriptorSet;

	forI(vectorDescriptorType.size())
	{
		VkWriteDescriptorSet writes;
		writes = {};
		writes.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes.pNext           = NULL;
		writes.dstSet          = descriptorSet;
		writes.descriptorCount = 1;
		writes.descriptorType  = vectorDescriptorType[i];
		writes.dstArrayElement = 0;
		writes.dstBinding      = vectorBinding[i];

		if (vectorDescriptorInfoHint[i] == 0)
		{
			writes.pBufferInfo = (VkDescriptorBufferInfo*)vectorDescriptorInfo[i];
		}
		else if (vectorDescriptorInfoHint[i] == 1)
		{
			writes.pImageInfo = (VkDescriptorImageInfo*)vectorDescriptorInfo[i];
		}
		else if (vectorDescriptorInfoHint[i] == 2)
		{
			writes.pTexelBufferView = (VkBufferView*)vectorDescriptorInfo[i];
		}

		vectorWriteDescriptorSet.push_back(writes);
	}

	vkUpdateDescriptorSets(coreM->getLogicalDevice(), uint32_t(vectorWriteDescriptorSet.size()), vectorWriteDescriptorSet.data(), 0, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::destroyPipelineCache()
{
	vkDestroyPipelineCache(coreM->getLogicalDevice(), m_pipelineCache, NULL);
	m_pipelineCache = VK_NULL_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::shutdown()
{
	this->~GPUPipeline();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::destroyResources()
{
	forI(m_vectorRasterTechnique.size())
	{
		delete m_vectorRasterTechnique[i];
	}

	destroyPipelineCache();
}

/////////////////////////////////////////////////////////////////////////////////////////////

RasterTechnique *GPUPipeline::getRasterTechniqueByName(string &&name)
{
	forIT(m_vectorRasterTechnique)
	{
		if ((*it)->getName() == name)
		{
			return *it;
		}
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

int GPUPipeline::getRasterTechniqueIndex(RasterTechnique* technique)
{
	return findElementIndex(m_vectorRasterTechnique, technique);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool GPUPipeline::addRasterFlag(string&& flagName, int value)
{
	bool result = addIfNoPresent(move(flagName), value, m_mapRasterFlag);

	if (!result)
	{
		m_mapRasterFlag[flagName] = value;
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool GPUPipeline::removeRasterFlag(string&& flagName)
{
	auto it = m_mapRasterFlag.find(move(string(flagName)));

	if (it == m_mapRasterFlag.end())
	{
		return false;
	}

	return (m_mapRasterFlag.erase(flagName) > 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool GPUPipeline::setRasterFlag(string&& flagName, int value)
{
	map<string, int>::iterator it = m_mapRasterFlag.find(flagName);

	if (it == m_mapRasterFlag.end())
	{
		return false;
	}

	it->second = value;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

int GPUPipeline::getRasterFlagValue(string&& flagName)
{
	map<string, int>::iterator it = m_mapRasterFlag.find(flagName);

	if (it == m_mapRasterFlag.end())
	{
		return -1;
	}

	return it->second;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkVertexInputAttributeDescription* GPUPipeline::refVertexInputAttributeDescription()
{
	return &m_viIpAttrb[0];
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GPUPipeline::preSceneLoadResources()
{
	Texture* renderTargetColor = textureM->buildTexture(move(string("scenelightingcolor")),
														VK_FORMAT_R8G8B8A8_UNORM,
														{ uint32_t(coreM->getWidth()), uint32_t(coreM->getHeight()), 1 },
														VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
														VK_IMAGE_ASPECT_COLOR_BIT,
														VK_IMAGE_ASPECT_COLOR_BIT,
														VK_IMAGE_LAYOUT_UNDEFINED,
														VK_IMAGE_LAYOUT_GENERAL,
														VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
														VK_SAMPLE_COUNT_1_BIT,
														VK_IMAGE_TILING_OPTIMAL,
														VK_IMAGE_VIEW_TYPE_2D,
														0);
	
	Texture* renderTargetDepth = textureM->buildTexture(move(string("scenelightingdepth")),
														VK_FORMAT_D24_UNORM_S8_UINT,
														{ uint32_t(coreM->getWidth()), uint32_t(coreM->getHeight()), 1 },
														VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
														VK_IMAGE_ASPECT_DEPTH_BIT,
														VK_IMAGE_ASPECT_DEPTH_BIT,
														VK_IMAGE_LAYOUT_UNDEFINED,
														VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
														VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
														VK_SAMPLE_COUNT_1_BIT,
														VK_IMAGE_TILING_OPTIMAL,
														VK_IMAGE_VIEW_TYPE_2D,
														0);

	VkAttachmentReference* depthReference  = new VkAttachmentReference({ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
	VkPipelineBindPoint* pipelineBindPoint = new VkPipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);

	vector<VkFormat>* vectorAttachmentFormat = new vector<VkFormat>;
	vectorAttachmentFormat->push_back(VK_FORMAT_R8G8B8A8_UNORM);
	vectorAttachmentFormat->push_back(VK_FORMAT_D16_UNORM);

	vector<VkSampleCountFlagBits>* vectorAttachmentSamplesPerPixel = new vector<VkSampleCountFlagBits>;
	vectorAttachmentSamplesPerPixel->push_back(VK_SAMPLE_COUNT_1_BIT);
	vectorAttachmentSamplesPerPixel->push_back(VK_SAMPLE_COUNT_1_BIT);

	vector<VkImageLayout>* vectorAttachmentFinalLayout = new vector<VkImageLayout>;
	vectorAttachmentFinalLayout->push_back(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	vectorAttachmentFinalLayout->push_back(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	vector<VkAttachmentReference>* vectorColorReference = new vector<VkAttachmentReference>;
	vectorColorReference->push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	MultiTypeUnorderedMap *attributeUM = new MultiTypeUnorderedMap();
	attributeUM->newElement<AttributeData<VkAttachmentReference*>*>        (new AttributeData<VkAttachmentReference*>        (string(g_renderPassAttachmentDepthReference),    move(depthReference)));
	attributeUM->newElement<AttributeData<VkPipelineBindPoint*>*>          (new AttributeData<VkPipelineBindPoint*>          (string(g_renderPassAttachmentPipelineBindPoint), move(pipelineBindPoint)));
	attributeUM->newElement<AttributeData<vector<VkFormat>*>*>             (new AttributeData<vector<VkFormat>*>             (string(g_renderPassAttachmentFormat),            move(vectorAttachmentFormat)));
	attributeUM->newElement<AttributeData<vector<VkSampleCountFlagBits>*>*>(new AttributeData<vector<VkSampleCountFlagBits>*>(string(g_renderPassAttachmentSamplesPerPixel),   move(vectorAttachmentSamplesPerPixel)));
	attributeUM->newElement<AttributeData<vector<VkImageLayout>*>*>        (new AttributeData<vector<VkImageLayout>*>        (string(g_renderPassAttachmentFinalLayout),       move(vectorAttachmentFinalLayout)));
	attributeUM->newElement<AttributeData<vector<VkAttachmentReference>*>*>(new AttributeData<vector<VkAttachmentReference>*>(string(g_renderPassAttachmentColorReference),    move(vectorColorReference)));

	renderPassM->buildRenderPass(move(string("scenelightingrenderpass")), attributeUM);

	vector<string> arrayAttachment;
	arrayAttachment.resize(2);
	arrayAttachment[0] = renderTargetColor->getName();
	arrayAttachment[1] = renderTargetDepth->getName();
	framebufferM->buildFramebuffer(move(string("scenelightingrenderpassFB")), coreM->getWidth(), coreM->getHeight(), move(string("scenelightingrenderpass")), move(arrayAttachment));

	// Build default material
	Texture* reflectance = textureM->build2DTextureFromFile(
		move(string("reflectanceTexture0")),
		move(string("../data/textures/reflectancemap.ktx")),
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		VK_FORMAT_R8G8B8A8_UNORM);

	Texture* normal = textureM->build2DTextureFromFile(
		move(string("normalTexture0")),
		move(string("../data/textures/normal.ktx")),
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		VK_FORMAT_R8G8B8A8_UNORM);

	MultiTypeUnorderedMap *attributeMaterial = new MultiTypeUnorderedMap();
	attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_reflectanceTextureResourceName), string("reflectanceTexture0")));
	attributeMaterial->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_normalTextureResourceName), string("normalTexture0")));
	Material* defaultMaterial = materialM->buildMaterial(move(string("MaterialColorTexture")), move(string("DefaultMaterial")), attributeMaterial);
	Material* defaultMaterialInstanced = materialM->buildMaterial(move(string("MaterialIndirectColorTexture")), move(string("DefaultMaterialInstanced")), attributeMaterial);
}

/////////////////////////////////////////////////////////////////////////////////////////////
