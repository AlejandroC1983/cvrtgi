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
#include "../../include/rastertechnique/distanceshadowmappingtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialdistanceshadowmapping.h"
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
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/framebuffer/framebuffer.h"
#include "../../include/framebuffer/framebuffermanager.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES
#define DISTANCE_SHADOW_MAPPING_SIZE 8192
//#define DISTANCE_SHADOW_MAPPING_SIZE 512

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

DistanceShadowMappingTechnique::DistanceShadowMappingTechnique(string &&name, string&& className) :
	  RasterTechnique(move(name), move(className))
	, m_renderPass(nullptr)
	, m_framebuffer(nullptr)
	, m_material(nullptr)
	, m_camera(nullptr)
	, m_distanceShadowMappingTexture(nullptr)
	, m_shadowMapWidth(DISTANCE_SHADOW_MAPPING_SIZE)
	, m_shadowMapHeight(DISTANCE_SHADOW_MAPPING_SIZE)
	, m_offscreenDistanceDepthTexture(nullptr)
	, m_emitterRadiance(0.0f)
	, m_useCompactedGeometry(false)
{
	m_emitterRadiance = float(gpuPipelineM->getRasterFlagValue(move(string("EMITTER_RADIANCE"))));
}

/////////////////////////////////////////////////////////////////////////////////////////////

DistanceShadowMappingTechnique::~DistanceShadowMappingTechnique()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void DistanceShadowMappingTechnique::init()
{
	string distanceTextureName;
	string offscreenTextureName;
	if (m_parameterData->elementExists(g_distanceShadowMapDistanceTextureCodeChunkHashed))
	{
		AttributeData<string>* attribute = m_parameterData->getElement<AttributeData<string>*>(g_distanceShadowMapDistanceTextureCodeChunkHashed);
		distanceTextureName = attribute->m_data;
	}
	if (m_parameterData->elementExists(g_distanceShadowMapOffscreenTextureCodeChunkHashed))
	{
		AttributeData<string>* attribute = m_parameterData->getElement<AttributeData<string>*>(g_distanceShadowMapOffscreenTextureCodeChunkHashed);
		offscreenTextureName = attribute->m_data;
	}

	m_distanceShadowMappingTexture = textureM->buildTexture(
		move(string(distanceTextureName)),
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

	m_offscreenDistanceDepthTexture = textureM->buildTexture(
		move(string(offscreenTextureName)),
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

	m_renderPass = renderPassM->buildRenderPass(move(string("distanceshadowmaprenderpass")), attributeUM);

	string materialName;
	if (m_parameterData->elementExists(g_distanceShadowMapMaterialNameCodeChunkHashed))
	{
		AttributeData<string>* attribute = m_parameterData->getElement<AttributeData<string>*>(g_distanceShadowMapMaterialNameCodeChunkHashed);
		materialName = attribute->m_data;
	}

	if (m_parameterData->elementExists(g_distanceShadowMapUseCompactedGeometryCodeChunkHashed))
	{
		AttributeData<bool>* attribute = m_parameterData->getElement<AttributeData<bool>*>(g_distanceShadowMapUseCompactedGeometryCodeChunkHashed);
		m_useCompactedGeometry = attribute->m_data;
	}

	MultiTypeUnorderedMap* attributeMaterial = new MultiTypeUnorderedMap();
	attributeMaterial->newElement<AttributeData<bool>*>(new AttributeData<bool>(string(g_distanceShadowMapUseInstancedRendering), move(bool(!m_useCompactedGeometry))));
	m_material = static_cast<MaterialDistanceShadowMapping*>(materialM->buildMaterial(move(string("MaterialDistanceShadowMapping")), move(string(materialName)), attributeMaterial));

	m_vectorMaterialName.push_back("MaterialDistanceShadowMapping");
	m_vectorMaterial.push_back(m_material);

	vector<string> arrayAttachment;
	arrayAttachment.push_back(m_distanceShadowMappingTexture->getName());
	arrayAttachment.push_back(m_offscreenDistanceDepthTexture->getName());

	string framebufferName;
	if (m_parameterData->elementExists(g_distanceShadowMapFramebufferNameCodeChunkHashed))
	{
		AttributeData<string>* attribute = m_parameterData->getElement<AttributeData<string>*>(g_distanceShadowMapFramebufferNameCodeChunkHashed);
		framebufferName = attribute->m_data;
	}

	m_framebuffer = framebufferM->buildFramebuffer(move(string(framebufferName)), (uint32_t)(m_shadowMapWidth), (uint32_t)(m_shadowMapHeight), move(string(m_renderPass->getName())), move(arrayAttachment));

	string cameraName;
	if (m_parameterData->elementExists(g_distanceShadowMapCameraNameCodeChunkHashed))
	{
		AttributeData<string>* attribute = m_parameterData->getElement<AttributeData<string>*>(g_distanceShadowMapCameraNameCodeChunkHashed);
		cameraName = attribute->m_data;
	}

	m_camera = cameraM->getElement(move(string(cameraName)));

	m_camera->refCameraDirtySignal().connect<DistanceShadowMappingTechnique, &DistanceShadowMappingTechnique::slotCameraDirty>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void DistanceShadowMappingTechnique::prepare(float dt)
{
	vec3 cameraPosition = m_camera->getPosition();
	m_material->setViewProjection(m_camera->getViewProjection());
	m_material->setLightPosition(vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* DistanceShadowMappingTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
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
	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, &bufferM->getElement(move(string("vertexBuffer")))->getBuffer(), offsets); // Bound the command buffer with the graphics pipeline

	if (!m_useCompactedGeometry)
	{
		Buffer* instanceDataBuffer = bufferM->getElement(move(string("instanceDataBuffer")));
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(*commandBuffer, 1, 1, &instanceDataBuffer->getBuffer(), offsets);
	}

	vkCmdBindIndexBuffer(*commandBuffer, bufferM->getElement(move(string("indexBuffer")))->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	gpuPipelineM->initViewports((float)m_shadowMapWidth, (float)m_shadowMapHeight, 0.0f, 0.0f, 0.0f, 1.0f, commandBuffer);
	gpuPipelineM->initScissors(m_shadowMapWidth, m_shadowMapHeight, 0, 0, commandBuffer);

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material->getPipeline()->getPipeline());

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	// Depth bias (and slope) are used to avoid shadowing artefacts 
	// Constant depth bias factor (always applied)
	float depthBiasConstant = 1.25f;
	float depthBiasSlope    = 1.75f;
	vkCmdSetDepthBias(*commandBuffer, depthBiasConstant, 0.0f, depthBiasSlope);

	if (m_useCompactedGeometry)
	{
		Node* mergedGeometry = sceneM->refElementByName(move(string("sceneCompactedGeometry")));
		uint32_t sceneDataBufferOffset = static_cast<uint32_t>(gpuPipelineM->getSceneUniformData()->getDynamicAllignment());
		uint32_t offsetData[3];
		offsetData[2] = 0;

		offsetData[0] = sceneM->getElementIndex(mergedGeometry) * sceneDataBufferOffset;
		offsetData[1] = static_cast<uint32_t>(m_material->getMaterialUniformBufferIndex() * dynamicAllignment);
		vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material->getPipelineLayout(), 0, 1, &m_material->refDescriptorSet(), 2, &offsetData[0]);
		vkCmdDrawIndexed(*commandBuffer, mergedGeometry->getIndexSize(), 1, mergedGeometry->getStartIndex(), 0, 0);
	}
	else
	{
		vectorNodePtr m_arrayNode = sceneM->getByMeshType(E_MT_RENDER_MODEL);
		forI(m_arrayNode.size())
		{
			uint32_t offsetData[3];
			uint32_t sceneDataBufferOffset = static_cast<uint32_t>(gpuPipelineM->getSceneUniformData()->getDynamicAllignment());

			offsetData[0] = sceneM->getElementIndex(m_arrayNode[i]) * sceneDataBufferOffset;
			offsetData[1] = static_cast<uint32_t>(m_material->getMaterialUniformBufferIndex() * dynamicAllignment);

			Buffer* m_indirectCommandBuffer = nullptr;

			if (m_camera->getName() == "emitter")
			{
				m_indirectCommandBuffer = bufferM->getElement(move(string("indirectCommandBufferEmitterCamera")));
			}
			else if (m_camera->getName() == "maincamera")
			{
				m_indirectCommandBuffer = bufferM->getElement(move(string("indirectCommandBufferMainCamera")));
			}

			vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_material->getPipelineLayout(), 0, 1, &m_material->refDescriptorSet(), 2, &offsetData[0]);
			vkCmdDrawIndexedIndirect(*commandBuffer, m_indirectCommandBuffer->getBuffer(), i * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
		}
	}

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

void DistanceShadowMappingTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void DistanceShadowMappingTechnique::slotCameraDirty()
{
	m_active = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
