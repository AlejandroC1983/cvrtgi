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
#include "../../include/pipeline/pipelinedata.h"
#include "../../include/core/coremanager.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

PipelineData::PipelineData():
	  m_dynamicState({})
	, m_vertexInputStateInfo({})
	, m_inputAssemblyInfo({})
	, m_rasterStateInfo({})
	, m_colorBlendStateInfo({})
	, m_viewportStateInfo({})
	, m_depthStencilStateInfo({})
	, m_multiSampleStateInfo({})
	, m_pipelineInfo({})
{
	// Initialize the dynamic states, initially it’s empty
	memset(m_dynamicStateEnables, 0, sizeof(m_dynamicStateEnables));
	setDefaultValues();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::setDefaultValues()
{
	// Specify the dynamic state information to pipeline through VkPipelineDynamicStateCreateInfo control structure.
	m_dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	m_dynamicState.pNext             = nullptr;
	m_dynamicState.pDynamicStates    = m_dynamicStateEnables;
	m_dynamicState.dynamicStateCount = 0;

	m_vertexInputStateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_vertexInputStateInfo.pNext                           = nullptr;
	m_vertexInputStateInfo.flags                           = 0;
	m_vertexInputStateInfo.vertexBindingDescriptionCount   = 1;
	m_vertexInputStateInfo.pVertexBindingDescriptions      = &gpuPipelineM->refViIpBind();
	m_vertexInputStateInfo.vertexAttributeDescriptionCount = 4;
	m_vertexInputStateInfo.pVertexAttributeDescriptions    = gpuPipelineM->refVertexInputAttributeDescription();

	m_inputAssemblyInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssemblyInfo.pNext                  = nullptr;
	m_inputAssemblyInfo.flags                  = 0;
	m_inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
	m_inputAssemblyInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	m_rasterStateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterStateInfo.pNext                   = nullptr;
	m_rasterStateInfo.flags                   = 0;
	m_rasterStateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
	m_rasterStateInfo.cullMode                = VK_CULL_MODE_BACK_BIT;
	m_rasterStateInfo.frontFace               = VK_FRONT_FACE_CLOCKWISE;
	m_rasterStateInfo.depthClampEnable        = false; // is false in the offscreen case
	m_rasterStateInfo.rasterizerDiscardEnable = VK_FALSE;
	m_rasterStateInfo.depthBiasEnable         = VK_FALSE;
	m_rasterStateInfo.depthBiasConstantFactor = 0;
	m_rasterStateInfo.depthBiasClamp          = 0;
	m_rasterStateInfo.depthBiasSlopeFactor    = 0;
	m_rasterStateInfo.lineWidth               = 1.0f;

	// Create the viewport state create info and provide the the number of viewport and scissors being used in the rendering pipeline.
	VkPipelineColorBlendAttachmentState colorBlendAttachmentStateInfo = {};
	colorBlendAttachmentStateInfo.colorWriteMask      = 0xf;
	colorBlendAttachmentStateInfo.blendEnable         = VK_FALSE; // is true in the offscreen case, values are different too
	colorBlendAttachmentStateInfo.alphaBlendOp        = VK_BLEND_OP_ADD;
	colorBlendAttachmentStateInfo.colorBlendOp        = VK_BLEND_OP_ADD;
	colorBlendAttachmentStateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentStateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentStateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentStateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	m_arrayColorBlendAttachmentStateInfo.push_back(colorBlendAttachmentStateInfo);

	m_colorBlendStateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_colorBlendStateInfo.flags             = 0;
	m_colorBlendStateInfo.pNext             = nullptr;
	m_colorBlendStateInfo.attachmentCount   = uint32_t(m_arrayColorBlendAttachmentStateInfo.size());
	m_colorBlendStateInfo.pAttachments      = m_arrayColorBlendAttachmentStateInfo.data();
	m_colorBlendStateInfo.logicOpEnable     = VK_FALSE;
	m_colorBlendStateInfo.blendConstants[0] = 1.0f; // value is 0.0 in the offscreen case
	m_colorBlendStateInfo.blendConstants[1] = 1.0f; // value is 0.0 in the offscreen case
	m_colorBlendStateInfo.blendConstants[2] = 1.0f; // value is 0.0 in the offscreen case
	m_colorBlendStateInfo.blendConstants[3] = 1.0f; // value is 0.0 in the offscreen case

	m_viewportStateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewportStateInfo.pNext         = nullptr;
	m_viewportStateInfo.flags         = 0;
	m_viewportStateInfo.viewportCount = NUMBER_OF_VIEWPORTS;
	m_viewportStateInfo.scissorCount  = NUMBER_OF_SCISSORS;
	m_viewportStateInfo.pScissors     = nullptr;
	m_viewportStateInfo.pViewports    = nullptr;

	// Specify the dynamic state count and VkDynamicState enum stating which dynamic state will use the values from dynamic
	// state commands rather than from the pipeline state creation info.
	m_dynamicStateEnables[m_dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	m_dynamicStateEnables[m_dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	m_depthStencilStateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_depthStencilStateInfo.pNext                 = nullptr;
	m_depthStencilStateInfo.flags                 = 0;
	m_depthStencilStateInfo.depthTestEnable       = true;
	m_depthStencilStateInfo.depthWriteEnable      = true;
	m_depthStencilStateInfo.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
	m_depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
	m_depthStencilStateInfo.stencilTestEnable     = VK_FALSE;
	m_depthStencilStateInfo.back.failOp           = VK_STENCIL_OP_KEEP;
	m_depthStencilStateInfo.back.passOp           = VK_STENCIL_OP_KEEP;
	m_depthStencilStateInfo.back.compareOp        = VK_COMPARE_OP_ALWAYS; // Front compareOp = VK_COMPARE_OP_NEVER (0), back VK_COMPARE_OP_ALWAYS in the offscreen case, there shouldn0t be depth nor stencil operations in the offscreen case for the expanding plane
	m_depthStencilStateInfo.back.compareMask      = 0;
	m_depthStencilStateInfo.back.reference        = 0;
	m_depthStencilStateInfo.back.depthFailOp      = VK_STENCIL_OP_KEEP;
	m_depthStencilStateInfo.back.writeMask        = 0;
	m_depthStencilStateInfo.minDepthBounds        = 0;
	m_depthStencilStateInfo.maxDepthBounds        = 0;
	m_depthStencilStateInfo.front                 = m_depthStencilStateInfo.back;

	m_multiSampleStateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multiSampleStateInfo.pNext                 = nullptr;
	m_multiSampleStateInfo.flags                 = 0;
	m_multiSampleStateInfo.pSampleMask           = nullptr;
	m_multiSampleStateInfo.rasterizationSamples  = NUM_SAMPLES;
	m_multiSampleStateInfo.sampleShadingEnable   = VK_FALSE;
	m_multiSampleStateInfo.alphaToCoverageEnable = VK_FALSE;
	m_multiSampleStateInfo.alphaToOneEnable      = VK_FALSE;
	m_multiSampleStateInfo.minSampleShading      = 0.0;

	// Populate the VkGraphicsPipelineCreateInfo structure to specify programmable stages, fixed-function
	// pipeline stages render pass, sub-passes and pipeline layouts
	m_pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	m_pipelineInfo.pNext               = nullptr;
	m_pipelineInfo.layout              = VK_NULL_HANDLE;
	m_pipelineInfo.basePipelineHandle  = 0;
	m_pipelineInfo.basePipelineIndex   = 0;
	m_pipelineInfo.flags               = 0;
	m_pipelineInfo.pVertexInputState   = &m_vertexInputStateInfo;
	m_pipelineInfo.pInputAssemblyState = &m_inputAssemblyInfo;
	m_pipelineInfo.pRasterizationState = &m_rasterStateInfo;
	m_pipelineInfo.pColorBlendState    = &m_colorBlendStateInfo;
	m_pipelineInfo.pTessellationState  = nullptr;
	m_pipelineInfo.pMultisampleState   = &m_multiSampleStateInfo;
	m_pipelineInfo.pDynamicState       = &m_dynamicState;
	m_pipelineInfo.pViewportState      = &m_viewportStateInfo;
	m_pipelineInfo.pDepthStencilState  = &m_depthStencilStateInfo;
	m_pipelineInfo.pStages             = nullptr;
	m_pipelineInfo.stageCount          = 0;
	m_pipelineInfo.renderPass          = nullptr;
	m_pipelineInfo.subpass             = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::setNullDepthStencilState()
{
	m_pipelineInfo.pDepthStencilState = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::updateArrayColorBlendAttachmentStateInfo(vector<VkPipelineColorBlendAttachmentState>& arrayData)
{
	m_arrayColorBlendAttachmentStateInfo  = arrayData;
	m_colorBlendStateInfo.attachmentCount = uint32_t(m_arrayColorBlendAttachmentStateInfo.size());
	m_colorBlendStateInfo.pAttachments    = m_arrayColorBlendAttachmentStateInfo.data();
	m_pipelineInfo.pColorBlendState       = &m_colorBlendStateInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::updateVertexInputStateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputStateInfo)
{
	m_vertexInputStateInfo           = vertexInputStateInfo;
	m_pipelineInfo.pVertexInputState = &m_vertexInputStateInfo;

}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::updateRasterStateInfo(VkPipelineRasterizationStateCreateInfo& rasterStateInfo)
{
	m_rasterStateInfo                  = rasterStateInfo;
	m_pipelineInfo.pRasterizationState = &m_rasterStateInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::updateColorBlendStateInfo(VkPipelineColorBlendStateCreateInfo&         colorBlendStateInfo,
		                                     vector<VkPipelineColorBlendAttachmentState>& arrayData)
{
	m_arrayColorBlendAttachmentStateInfo  = arrayData;
	m_colorBlendStateInfo                 = colorBlendStateInfo;
	m_colorBlendStateInfo.attachmentCount = uint32_t(m_arrayColorBlendAttachmentStateInfo.size());
	m_colorBlendStateInfo.pAttachments    = m_arrayColorBlendAttachmentStateInfo.data();
	m_pipelineInfo.pColorBlendState       = &m_colorBlendStateInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::updateDepthStencilStateInfo(VkPipelineDepthStencilStateCreateInfo& depthStencilStateInfo)
{
	m_depthStencilStateInfo           = depthStencilStateInfo;
	m_pipelineInfo.pDepthStencilState = &m_depthStencilStateInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::updateinputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo)
{
	m_inputAssemblyInfo                = inputAssemblyStateCreateInfo;
	m_pipelineInfo.pInputAssemblyState = &m_inputAssemblyInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PipelineData::updateDynamicStateCreateInfo(vector<VkDynamicState> vectorDynamicState)
{
	memset(m_dynamicStateEnables, 0, sizeof(m_dynamicStateEnables));
	m_dynamicState.dynamicStateCount = 0;

	forI(vectorDynamicState.size())
	{
		m_dynamicStateEnables[m_dynamicState.dynamicStateCount++] = vectorDynamicState[i];
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
