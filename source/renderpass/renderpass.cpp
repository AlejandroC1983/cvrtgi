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
#include "../../include/renderpass/renderpass.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/core/coremanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::RenderPass(string &&name) : GenericResource(move(name), move(string("RenderPass")), GenericResourceType::GRT_RENDERPASS)
	, m_depthReference({})
	, m_pipelineBindPoint(VK_PIPELINE_BIND_POINT_MAX_ENUM)
	, m_renderPass(VK_NULL_HANDLE)
	, m_hasDepthAttachment(false)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(coreM->getLogicalDevice(), m_renderPass, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::updateRenderPassParameters()
{
	if (m_parameterData->elementExists(g_renderPassAttachmentFormatHashed))
	{
		AttributeData<vector<VkFormat>*>* attribute = m_parameterData->getElement<AttributeData<vector<VkFormat>*>*>(g_renderPassAttachmentFormatHashed);
		m_vectorAttachmentFormat = (*attribute->m_data);
		m_parameterData->removeElement(g_renderPassAttachmentFormatHashed);
	}

	if (m_parameterData->elementExists(g_renderPassAttachmentSamplesPerPixelHashed))
	{
		AttributeData<vector<VkSampleCountFlagBits>*>* attribute = m_parameterData->getElement<AttributeData<vector<VkSampleCountFlagBits>*>*>(g_renderPassAttachmentSamplesPerPixelHashed);
		m_vectorAttachmentSamplesPerPixel = (*attribute->m_data);
		m_parameterData->removeElement(g_renderPassAttachmentSamplesPerPixelHashed);
	}

	if (m_parameterData->elementExists(g_renderPassAttachmentFinalLayoutHashed))
	{
		AttributeData<vector<VkImageLayout>*>* attribute = m_parameterData->getElement<AttributeData<vector<VkImageLayout>*>*>(g_renderPassAttachmentFinalLayoutHashed);
		m_vectorAttachmentFinalLayout = (*attribute->m_data);
		m_parameterData->removeElement(g_renderPassAttachmentFinalLayoutHashed);
	}

	if (m_parameterData->elementExists(g_renderPassAttachmentColorReferenceHashed))
	{
		AttributeData<vector<VkAttachmentReference>*>* attribute = m_parameterData->getElement<AttributeData<vector<VkAttachmentReference>*>*>(g_renderPassAttachmentColorReferenceHashed);
		m_vectorColorReference = (*attribute->m_data);
		m_parameterData->removeElement(g_renderPassAttachmentColorReferenceHashed);
	}

	if (m_parameterData->elementExists(g_renderPassAttachmentDepthReferenceHashed))
	{
		AttributeData<VkAttachmentReference*>* attribute = m_parameterData->getElement<AttributeData<VkAttachmentReference*>*>(g_renderPassAttachmentDepthReferenceHashed);
		m_depthReference = (*attribute->m_data);
		m_parameterData->removeElement(g_renderPassAttachmentDepthReferenceHashed);
		m_hasDepthAttachment = true;
	}

	if (m_parameterData->elementExists(g_renderPassAttachmentPipelineBindPointHashed))
	{
		AttributeData<VkPipelineBindPoint*>* attribute = m_parameterData->getElement<AttributeData<VkPipelineBindPoint*>*>(g_renderPassAttachmentPipelineBindPointHashed);
		m_pipelineBindPoint = (*attribute->m_data);
		m_parameterData->removeElement(g_renderPassAttachmentPipelineBindPointHashed);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void RenderPass::buildRenderPass()
{
	if ((m_vectorAttachmentFormat.size() != m_vectorAttachmentFinalLayout.size() ||
		(m_vectorAttachmentFormat.size() != m_vectorAttachmentSamplesPerPixel.size())))
	{
		cout << "ERROR: no the same number of attachment format and attachment final layout data in RenderPass::buildRenderPass" << endl;
		return;
	}

	vector<VkAttachmentDescription> attchmentDescriptions;
	uint attachmentNumber = uint(m_vectorAttachmentFormat.size());
	attchmentDescriptions.resize(attachmentNumber);
	forI(attachmentNumber)
	{
		attchmentDescriptions[i].format         = m_vectorAttachmentFormat[i];
		attchmentDescriptions[i].samples        = m_vectorAttachmentSamplesPerPixel[i];
		attchmentDescriptions[i].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[i].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[i].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[i].finalLayout    = m_vectorAttachmentFinalLayout[i];
		attchmentDescriptions[i].flags          = 0; // Read doc to know if is really needed / the dependency can be found
	}

	VkSubpassDescription subpassDescription    = {};
	subpassDescription.pipelineBindPoint       = m_pipelineBindPoint;
	subpassDescription.colorAttachmentCount    = uint32_t(m_vectorColorReference.size());
	subpassDescription.pColorAttachments       = m_vectorColorReference.data();
	subpassDescription.pDepthStencilAttachment = m_hasDepthAttachment ? &m_depthReference : nullptr;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext           = NULL;
	renderPassInfo.attachmentCount = uint32_t(attchmentDescriptions.size());
	renderPassInfo.pAttachments    = attchmentDescriptions.data();
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpassDescription;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies   = NULL;

	VkResult result = vkCreateRenderPass(coreM->getLogicalDevice(), &renderPassInfo, nullptr, &m_renderPass);
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////
