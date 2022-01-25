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
#include "../../include/rastertechnique/shadowmappingvoxeltechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialshadowmappingvoxel.h"
#include "../../include/scene/scene.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/core/coremanager.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/framebuffer/framebuffer.h"
#include "../../include/framebuffer/framebuffermanager.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"

#include "../../include/material/materiallighting.h"
#include "../../include/rastertechnique/scenelightingtechnique.h"
#include "../../include/rastertechnique/buildvoxelshadowmapgeometrytechnique.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES
#define SHADOW_MAPPING_VOXEL_SIZE 2048

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ShadowMappingVoxelTechnique::ShadowMappingVoxelTechnique(string &&name, string&& className) :
	  RasterTechnique(move(name), move(className))
	, m_renderPass(nullptr)
	, m_framebuffer(nullptr)
	, m_material(nullptr)
	, m_shadowMappingCamera(nullptr)
	, m_shadowMappingTexture(nullptr)
	, m_shadowMapWidth(2048)
	, m_shadowMapHeight(2048)
	, m_numUsedVertex(0)
	, m_prefixSumCompleted(false)
	, m_newPassRequested(false)
	, m_offscreenDepthTexture(nullptr)
{
	m_active        = false;
	m_needsToRecord = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

ShadowMappingVoxelTechnique::~ShadowMappingVoxelTechnique()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShadowMappingVoxelTechnique::init()
{
	m_shadowMappingCamera = cameraM->getElement(move(string("emitter")));
	m_shadowMappingCamera->refCameraDirtySignal().connect<ShadowMappingVoxelTechnique, &ShadowMappingVoxelTechnique::slotCameraDirty>(this);

	m_shadowMappingTexture = textureM->buildTexture(
		move(string("shadowmappingvoxeltexture")),
		VK_FORMAT_R16_SFLOAT,
		{ uint32_t(m_shadowMapWidth), uint32_t(m_shadowMapHeight), uint32_t(1) },
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_VIEW_TYPE_2D,
		0);

	m_offscreenDepthTexture = textureM->buildTexture(
		move(string("shadowmappingvoxeldepthtexture")),
		VK_FORMAT_D16_UNORM,
		{ uint32_t(m_shadowMapWidth), uint32_t(m_shadowMapWidth), uint32_t(1) },
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_VIEW_TYPE_2D,
		0);

	VkPipelineBindPoint* pipelineBindPoint = new VkPipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);

	vector<VkAttachmentReference>* vectorColorReference = new vector<VkAttachmentReference>;
	vectorColorReference->push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	vector<VkFormat>* vectorAttachmentFormat = new vector<VkFormat>;
	vectorAttachmentFormat->push_back(VK_FORMAT_R16_SFLOAT);
	vectorAttachmentFormat->push_back(VK_FORMAT_D16_UNORM);

	vector<VkSampleCountFlagBits>* vectorAttachmentSamplesPerPixel = new vector<VkSampleCountFlagBits>;
	vectorAttachmentSamplesPerPixel->push_back(VK_SAMPLE_COUNT_1_BIT);
	vectorAttachmentSamplesPerPixel->push_back(VK_SAMPLE_COUNT_1_BIT);

	vector<VkImageLayout>* vectorAttachmentFinalLayout = new vector<VkImageLayout>;
	vectorAttachmentFinalLayout->push_back(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	vectorAttachmentFinalLayout->push_back(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkAttachmentReference* depthReference = new VkAttachmentReference;
	depthReference->attachment            = 1;
	depthReference->layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	MultiTypeUnorderedMap *attributeUM = new MultiTypeUnorderedMap();
	attributeUM->newElement<AttributeData<VkPipelineBindPoint*>*>          (new AttributeData<VkPipelineBindPoint*>          (string(g_renderPassAttachmentPipelineBindPoint), move(pipelineBindPoint)));
	attributeUM->newElement<AttributeData<vector<VkFormat>*>*>             (new AttributeData<vector<VkFormat>*>             (string(g_renderPassAttachmentFormat),            move(vectorAttachmentFormat)));
	attributeUM->newElement<AttributeData<vector<VkSampleCountFlagBits>*>*>(new AttributeData<vector<VkSampleCountFlagBits>*>(string(g_renderPassAttachmentSamplesPerPixel),   move(vectorAttachmentSamplesPerPixel)));
	attributeUM->newElement<AttributeData<vector<VkImageLayout>*>*>        (new AttributeData<vector<VkImageLayout>*>        (string(g_renderPassAttachmentFinalLayout),       move(vectorAttachmentFinalLayout)));
	attributeUM->newElement<AttributeData<VkAttachmentReference*>*>        (new AttributeData<VkAttachmentReference*>        (string(g_renderPassAttachmentDepthReference),    move(depthReference)));
	attributeUM->newElement<AttributeData<vector<VkAttachmentReference>*>*>(new AttributeData<vector<VkAttachmentReference>*>(string(g_renderPassAttachmentColorReference),    move(vectorColorReference)));

	m_renderPass = renderPassM->buildRenderPass(move(string("shadowmappingvoxelrenderpass")), attributeUM);
	
	m_material = static_cast<MaterialShadowMappingVoxel*>(materialM->buildMaterial(move(string("MaterialShadowMappingVoxel")), move(string("MaterialShadowMappingVoxel")), nullptr));

	m_vectorMaterialName.push_back("MaterialShadowMappingVoxel");
	m_vectorMaterial.push_back(m_material);

	vector<string> arrayAttachment;
	arrayAttachment.push_back(m_shadowMappingTexture->getName());
	arrayAttachment.push_back(m_offscreenDepthTexture->getName());
	m_framebuffer = framebufferM->buildFramebuffer(move(string("shadowmappingvoxelFB")), (uint32_t)(m_shadowMapWidth), (uint32_t)(m_shadowMapHeight), move(string(m_renderPass->getName())), move(arrayAttachment));

	BuildVoxelShadowMapGeometryTechnique* techniqueVoxelShadowMapGeometryTechnique = static_cast<BuildVoxelShadowMapGeometryTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BuildVoxelShadowMapGeometryTechnique"))));
	techniqueVoxelShadowMapGeometryTechnique->refSignalBuildVoxelShadowMapGeometryCompletion().connect<ShadowMappingVoxelTechnique, &ShadowMappingVoxelTechnique::slotPrefixSumCompleted>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShadowMappingVoxelTechnique::prepare(float dt)
{
	vec3 cameraPosition = m_shadowMappingCamera->getPosition();
	m_material->setViewProjection(m_shadowMappingCamera->getViewProjection());
	m_material->setLightPosition(vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* ShadowMappingVoxelTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	commandBufferType = CommandBufferType::CBT_GRAPHICS_QUEUE;

	VkCommandBuffer* commandBuffer;
	commandBuffer = addRecordedCommandBuffer(commandBufferID);
	addCommandBufferQueueType(commandBufferID, commandBufferType);
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), commandBuffer);

	coreM->beginCommandBuffer(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, coreM->getGraphicsQueueQueryPool(), m_queryIndex0);
#endif

	VkRenderPassBeginInfo renderPassBegin = VulkanStructInitializer::renderPassBeginInfo(
		m_renderPass->getRenderPass(),
		m_framebuffer->getFramebuffer(),
		VkRect2D({ 0, 0, uint32_t(m_shadowMapWidth), uint32_t(m_shadowMapHeight) }),
		m_material->refVectorClearValue());

	vkCmdBeginRenderPass(*commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE); // Start recording the render pass instance

	const VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, &bufferM->getElement(move(string("shadowMapGeometryVertexBuffer")))->getBuffer(), offsets); // Bound the command buffer with the graphics pipeline

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material->getPipeline()->getPipeline());

	gpuPipelineM->initViewports((float)m_shadowMapWidth, (float)m_shadowMapHeight, 0.0f, 0.0f, 0.0f, 1.0f, commandBuffer);
	gpuPipelineM->initScissors(m_shadowMapWidth, m_shadowMapHeight, 0, 0, commandBuffer);

	float depthBiasConstant = 1.25f;
	float depthBiasSlope = 1.75f;
	vkCmdSetDepthBias(*commandBuffer, depthBiasConstant, 0.0f, depthBiasSlope);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	uint32_t offsetData[2];
	offsetData[0] = 0;
	offsetData[1] = static_cast<uint32_t>(m_material->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material->getPipelineLayout(), 0, 1, &m_material->refDescriptorSet(), 2, &offsetData[0]);

	//vkCmdDraw(*commandBuffer, m_numUsedVertex * 36, 1, 0, 0);
	vkCmdDraw(*commandBuffer, m_numUsedVertex, 1, 0, 0);
	vkCmdEndRenderPass(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, coreM->getGraphicsQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);

	// NOTE: Clear command buffer if re-recorded
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShadowMappingVoxelTechnique::postCommandSubmit()
{
	m_active         = false;
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShadowMappingVoxelTechnique::slotPrefixSumCompleted()
{
	m_active                                     = true; // Note: should be active when BuildVoxelShadowMapGeometryTechnique finishes
	//BufferPrefixSumTechnique* techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	//m_numUsedVertex                           = techniquePrefixSum->getFirstIndexOccupiedElement();
	BuildVoxelShadowMapGeometryTechnique* techniqueVoxelShadowMapGeometryTechnique = static_cast<BuildVoxelShadowMapGeometryTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BuildVoxelShadowMapGeometryTechnique"))));
	m_numUsedVertex = techniqueVoxelShadowMapGeometryTechnique->getNumUsedVertex();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ShadowMappingVoxelTechnique::slotCameraDirty()
{
	m_active         = true;
	m_executeCommand = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
