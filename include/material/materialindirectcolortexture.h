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

#ifndef _MATERIALINDIRECTCOLORTEXTURE_H_
#define _MATERIALINDIRECTCOLORTEXTURE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/material/material.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/material/materialmanager.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/texture/texture.h"
#include "../../include/shader/shader.h"
#include "../../include/core/coremanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/renderpass/renderpassmanager.h"

// CLASS FORWARDING
class Shader;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MaterialIndirectColorTexture : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialIndirectColorTexture(string &&name) : Material(move(name), move(string("MaterialIndirectColorTexture")))
		, m_color(vec3(1.0f, 1.0f, 1.0f))
		, m_reflectance(nullptr)
		, m_normal(nullptr)
	{
		m_vectorClearValue[0].color.float32[0] = 0.5f;
		m_vectorClearValue[0].color.float32[1] = 0.5f;
		m_vectorClearValue[0].color.float32[2] = 0.5f;
		m_vectorClearValue[0].color.float32[3] = 0.5f;
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeVert;
		size_t sizeFrag;

		void* vertShaderCode = InputOutput::readFile("../data/vulkanshaders/indirectcolortexture.vert", &sizeVert);
		void* fragShaderCode = InputOutput::readFile("../data/vulkanshaders/indirectcolortexture.frag", &sizeFrag);

		m_shaderResourceName = "indirectcolortexture" + to_string(m_materialInstanceIndex);
		m_shader = shaderM->buildShaderVF(move(string(m_shaderResourceName)), (const char*)vertShaderCode, (const char*)fragShaderCode, m_materialSurfaceType);

		return true;
	}

	/** Load here all the resources needed for this material (textures and the like)
	* @return nothing */
	void loadResources()
	{
		m_reflectance = textureM->getElement(move(string(m_reflectanceTextureName))); //textureM->getElement(move(string("reflectanceTexture0")));
		m_normal      = textureM->getElement(move(string(m_normalTextureName)));
	}

	void setupPipelineData()
	{
		m_pipeline.refPipelineData().setRenderPass(renderPassM->getElement(move(string("scenelightingrenderpass")))->getRenderPass());

		// The VkVertexInputBinding viIpBind, stores the rate at which the information will be
		// injected for vertex input.
		m_viIpBind[0].binding   = 0;
		m_viIpBind[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		m_viIpBind[0].stride    = 48; // TODO: Replace by constant, here and in the original source in gpupipeline.cpp

		m_viIpBind[1].binding   = 1;
		m_viIpBind[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		m_viIpBind[1].stride    = 16; // TODO: Replace by constant, here and in the original source in gpupipeline.cpp

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

		// Per instance vertex atttribute description
		m_viIpAttrb[4].binding  = 1;
		m_viIpAttrb[4].location = 4;
		m_viIpAttrb[4].format   = VK_FORMAT_R32G32B32_SFLOAT;
		m_viIpAttrb[4].offset   = 0;

		m_viIpAttrb[5].binding  = 1;
		m_viIpAttrb[5].location = 5;
		m_viIpAttrb[5].format   = VK_FORMAT_R32_SFLOAT;
		m_viIpAttrb[5].offset   = 12;

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;

		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.pNext = nullptr;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.vertexBindingDescriptionCount = 2;
		vertexInputStateInfo.pVertexBindingDescriptions = &m_viIpBind[0];
		vertexInputStateInfo.vertexAttributeDescriptionCount = 6;
		vertexInputStateInfo.pVertexAttributeDescriptions = &m_viIpAttrb[0];

		m_pipeline.refPipelineData().setVertexInputStateInfo(vertexInputStateInfo);
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC3, (void*)(&m_color), move(string("myMaterialData")), move(string("color")));
		assignTextureToSampler(move(string("reflectance")), move(string(m_reflectanceTextureName)), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		assignTextureToSampler(move(string("normal")),      move(string(m_normalTextureName)),      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	}

	SET(vec3, m_color, Color)
	GET_SET(string, m_reflectanceTextureName, ReflectanceTextureName)
	GET_SET(string, m_normalTextureName, NormalTextureName)

protected:
	string                            m_reflectanceTextureName; //!< Name of the reflectance texture resource to assign to m_reflectance
	string                            m_normalTextureName;      //!< Name of the reflectance texture resource to assign to m_normal
	Texture*                          m_reflectance;            //!< Reflectance texture
	Texture*                          m_normal;                 //!< Normal texture
	vec3                              m_color;                  //!< Color to multiply the sampled texture value
	VkVertexInputBindingDescription   m_viIpBind[2];            //!< Stores the vertex input rate
	VkVertexInputAttributeDescription m_viIpAttrb[6];           //!< Store metadata helpful in data interpretation
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALINDIRECTCOLORTEXTURE_H_
