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

#ifndef _MATERIALDISTANCESHADOWMAPPING_H_
#define _MATERIALDISTANCESHADOWMAPPING_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/material/material.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/material/materialmanager.h"
#include "../../include/shader/shader.h"
#include "../../include/core/coremanager.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/scene/scene.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MaterialDistanceShadowMapping : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialDistanceShadowMapping(string&& name) : Material(move(name), move(string("MaterialDistanceShadowMapping")))
		, m_useInstancedRendering(false)
	{
		m_resourcesUsed = MaterialBufferResource::MBR_MATERIAL | MaterialBufferResource::MBR_CAMERA;

		m_vectorClearValue.resize(2);
		m_vectorClearValue[0].color.float32[0] = 0.0f;
		m_vectorClearValue[0].color.float32[1] = 0.0f;
		m_vectorClearValue[0].color.float32[2] = 0.0f;
		m_vectorClearValue[0].color.float32[3] = 1.0f;

		// Specify the depth/stencil clear value
		m_vectorClearValue[1].depthStencil.depth   = 1.0f;
		m_vectorClearValue[1].depthStencil.stencil = 0;
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeTemp;

		void* vertShaderCode = nullptr;
		if (m_useInstancedRendering)
		{
			vertShaderCode = InputOutput::readFile("../data/vulkanshaders/distanceshadowmappinginstanced.vert", &sizeTemp);
		}
		else
		{
			vertShaderCode = InputOutput::readFile("../data/vulkanshaders/distanceshadowmapping.vert", &sizeTemp);
		}

		void* fragmentShaderCode = InputOutput::readFile("../data/vulkanshaders/distanceshadowmapping.frag", &sizeTemp);
		m_shaderResourceName     = "distanceshadowmapping" + to_string(m_materialInstanceIndex);
		m_shader                 = shaderM->buildShaderVF(move(string(m_shaderResourceName)), (const char*)vertShaderCode, (const char*)fragmentShaderCode, m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_FLOAT_MAT4, (void*)(&m_viewProjection), move(string("myMaterialData")), move(string("viewProjection")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4, (void*)(&m_lightPosition),  move(string("myMaterialData")), move(string("lightPosition")));
	}

	void setupPipelineData()
	{
		VkPipelineRasterizationStateCreateInfo rasterStateInfo = m_pipeline.refPipelineData().getRasterStateInfo();
		rasterStateInfo.depthClampEnable                       = false;
		rasterStateInfo.depthBiasEnable                        = true;
		m_pipeline.refPipelineData().updateRasterStateInfo(rasterStateInfo);

		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = m_pipeline.refPipelineData().getDepthStencilStateInfo();
		depthStencilStateInfo.front.compareOp                       = VK_COMPARE_OP_NEVER;
		m_pipeline.refPipelineData().updateDepthStencilStateInfo(depthStencilStateInfo);

		vector<VkPipelineColorBlendAttachmentState> arrayColorBlendAttachmentStateInfo = m_pipeline.refPipelineData().getArrayColorBlendAttachmentStateInfo();
		arrayColorBlendAttachmentStateInfo[0].blendEnable         = 0;
		arrayColorBlendAttachmentStateInfo[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		arrayColorBlendAttachmentStateInfo[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		arrayColorBlendAttachmentStateInfo[0].colorBlendOp        = VK_BLEND_OP_ADD;
		arrayColorBlendAttachmentStateInfo[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		arrayColorBlendAttachmentStateInfo[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		arrayColorBlendAttachmentStateInfo[0].alphaBlendOp        = VK_BLEND_OP_ADD;
		arrayColorBlendAttachmentStateInfo[0].colorWriteMask      = 15;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = m_pipeline.refPipelineData().getColorBlendStateInfo();
		colorBlendStateInfo.blendConstants[0]                   = 0.0f;
		colorBlendStateInfo.blendConstants[1]                   = 0.0f;
		colorBlendStateInfo.blendConstants[2]                   = 0.0f;
		colorBlendStateInfo.blendConstants[3]                   = 0.0f;
		colorBlendStateInfo.attachmentCount                     = 1;
		m_pipeline.refPipelineData().updateColorBlendStateInfo(colorBlendStateInfo, arrayColorBlendAttachmentStateInfo);

		m_vertexInputBindingDescription[0].binding   = 0;
		m_vertexInputBindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		m_vertexInputBindingDescription[0].stride    = 48;

		m_vertexInputBindingDescription[1].binding   = 1;
		m_vertexInputBindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		m_vertexInputBindingDescription[1].stride    = 16; // TODO: Replace by constant, here and in the original source in gpupipeline.cpp

		m_vertexInputAttributeDescription[0].binding  = 0;
		m_vertexInputAttributeDescription[0].location = 0;
		m_vertexInputAttributeDescription[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		m_vertexInputAttributeDescription[0].offset   = 0;
		m_vertexInputAttributeDescription[1].binding  = 0;
		m_vertexInputAttributeDescription[1].location = 1;
		m_vertexInputAttributeDescription[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
		m_vertexInputAttributeDescription[1].offset   = 12; // After, 4 components - RGBA  each of 4 bytes(32bits)
		m_vertexInputAttributeDescription[2].binding  = 0;
		m_vertexInputAttributeDescription[2].location = 2;
		m_vertexInputAttributeDescription[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
		m_vertexInputAttributeDescription[2].offset   = 24;
		m_vertexInputAttributeDescription[3].binding  = 0;
		m_vertexInputAttributeDescription[3].location = 3;
		m_vertexInputAttributeDescription[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
		m_vertexInputAttributeDescription[3].offset   = 36;

		// Per instance vertex atttribute description
		m_vertexInputAttributeDescription[4].binding  = 1;
		m_vertexInputAttributeDescription[4].location = 4;
		m_vertexInputAttributeDescription[4].format   = VK_FORMAT_R32G32B32_SFLOAT;
		m_vertexInputAttributeDescription[4].offset   = 0;

		m_vertexInputAttributeDescription[5].binding  = 1;
		m_vertexInputAttributeDescription[5].location = 5;
		m_vertexInputAttributeDescription[5].format   = VK_FORMAT_R32_SFLOAT;
		m_vertexInputAttributeDescription[5].offset   = 12;

		VkPipelineVertexInputStateCreateInfo inputState = m_pipeline.refPipelineData().getVertexInputStateInfo();

		if (m_useInstancedRendering)
		{
			inputState.vertexBindingDescriptionCount   = 2;
			inputState.pVertexBindingDescriptions      = &m_vertexInputBindingDescription[0];
			inputState.vertexAttributeDescriptionCount = 6;
			inputState.pVertexAttributeDescriptions    = &m_vertexInputAttributeDescription[0];
		}
		else
		{
			inputState.vertexBindingDescriptionCount   = 1;
			inputState.pVertexBindingDescriptions      = &m_vertexInputBindingDescription[0];
			inputState.vertexAttributeDescriptionCount = 4;
			inputState.pVertexAttributeDescriptions    = &m_vertexInputAttributeDescription[0];
		}

		m_pipeline.refPipelineData().setVertexInputStateInfo(inputState);

		vector<VkDynamicState> vectorDynamicState;
		vectorDynamicState.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		vectorDynamicState.push_back(VK_DYNAMIC_STATE_SCISSOR);
		vectorDynamicState.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		m_pipeline.refPipelineData().updateDynamicStateCreateInfo(vectorDynamicState);

		PipelineData& pipelineData = m_pipeline.refPipelineData();

		m_pipeline.refPipelineData().setRenderPass(renderPassM->getElement(move(string("distanceshadowmaprenderpass")))->getRenderPass());
	}

	GET_SET(mat4, m_viewProjection, ViewProjection)
	GET_SET(vec4, m_lightPosition, LightPosition)
	SET(bool, m_useInstancedRendering, UseInstancedRendering)

protected:
	mat4                              m_viewProjection;                     //!< Shadow mapping view projection matrix
	VkVertexInputBindingDescription   m_vertexInputBindingDescription[2];   //!< This material uses a different vertex input format
	VkVertexInputAttributeDescription m_vertexInputAttributeDescription[6]; //!< This material uses a different vertex input format
	vec4                              m_lightPosition;                      //!< Shadow mapping camera position
	string                            m_renderPassName;                     //!< Name of the render pass resource to be used with this material instance
	bool                              m_useInstancedRendering;              //!< Flag to know whether to use instanced rendering for this material
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALDISTANCESHADOWMAPPING_H_
