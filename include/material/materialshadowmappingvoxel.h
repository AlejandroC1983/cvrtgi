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

#ifndef _MATERIALSHADOWMAPPINGVOXEL_H_
#define _MATERIALSHADOWMAPPINGVOXEL_H_

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

class MaterialShadowMappingVoxel : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialShadowMappingVoxel(string &&name) : Material(move(name), move(string("MaterialShadowMappingVoxel")))
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

		void* vertShaderCode     = InputOutput::readFile("../data/vulkanshaders/shadowmappingvoxelgeometry.vert", &sizeTemp);
		void* fragmentShaderCode = InputOutput::readFile("../data/vulkanshaders/shadowmappingvoxelgeometry.frag", &sizeTemp);
		m_shaderResourceName     = "shadowmappingvoxel" + to_string(m_materialInstanceIndex);
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

		m_vertexInputBindingDescription.binding   = 0;
		m_vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		m_vertexInputBindingDescription.stride    = 12;

		m_vertexInputAttributeDescription[0].binding  = 0;
		m_vertexInputAttributeDescription[0].location = 0;
		m_vertexInputAttributeDescription[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		m_vertexInputAttributeDescription[0].offset   = 0;

		VkPipelineVertexInputStateCreateInfo inputState = m_pipeline.refPipelineData().getVertexInputStateInfo();
		inputState.vertexBindingDescriptionCount        = 1;
		inputState.pVertexBindingDescriptions           = &m_vertexInputBindingDescription;
		inputState.vertexAttributeDescriptionCount      = 1;
		inputState.pVertexAttributeDescriptions         = m_vertexInputAttributeDescription;

		m_pipeline.refPipelineData().updateVertexInputStateInfo(inputState);

		vector<VkDynamicState> vectorDynamicState;
		vectorDynamicState.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		vectorDynamicState.push_back(VK_DYNAMIC_STATE_SCISSOR);
		vectorDynamicState.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		m_pipeline.refPipelineData().updateDynamicStateCreateInfo(vectorDynamicState);

		PipelineData& pipelineData = m_pipeline.refPipelineData();

		m_pipeline.refPipelineData().setRenderPass(renderPassM->getElement(move(string("shadowmappingvoxelrenderpass")))->getRenderPass());
	}

	GET_SET(mat4, m_viewProjection, ViewProjection)
	GET_SET(vec4, m_voxelizationSize, VoxelizationSize)
	GET_SET(vec4, m_sceneMin, SceneMin)
	GET_SET(vec4, m_sceneExtent, SceneExtent)
	GET_SET(vec4, m_lightPosition, LightPosition)

protected:
	mat4                              m_viewProjection;                     //!< Shadow mapping view projection matrix
	vec4                              m_voxelizationSize;                   //!< Voxelizatoin size
	vec4                              m_sceneMin;                           //!< Scene minimum value
	vec4                              m_sceneExtent;                        //!< Extent of the scene. Also, w value is used to distinguish between rasterization of reflectance or normal information
	VkVertexInputBindingDescription   m_vertexInputBindingDescription;      //!< This material uses a different vertex input format
	VkVertexInputAttributeDescription m_vertexInputAttributeDescription[1]; //!< This material uses a different vertex input format
	vec4                              m_lightPosition;                      //!< Shadow mapping camera position
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALSHADOWMAPPINGVOXEL_H_
