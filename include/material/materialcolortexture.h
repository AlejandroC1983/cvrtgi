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

#ifndef _MATERIALCOLORTEXTURE_H_
#define _MATERIALCOLORTEXTURE_H_

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

class MaterialColorTexture : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialColorTexture(string &&name) : Material(move(name), move(string("MaterialColorTexture")))
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

		void* vertShaderCode = InputOutput::readFile("../data/vulkanshaders/plaincolor.vert", &sizeVert);
		void* fragShaderCode = InputOutput::readFile("../data/vulkanshaders/plaincolor.frag", &sizeFrag);

		m_shaderResourceName = "plaincolor" + to_string(m_materialInstanceIndex);
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
	string   m_reflectanceTextureName; //!< Name of the reflectance texture resource to assign to m_reflectance
	string   m_normalTextureName;      //!< Name of the reflectance texture resource to assign to m_normal
	Texture* m_reflectance;            //!< Reflectance texture
	Texture* m_normal;                 //!< Normal texture
	vec3     m_color;                  //!< Color to multiply the sampled texture value
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALCOLORTEXTURE_H_
