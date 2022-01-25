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
#include "../../include/shader/image.h"
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Image::Image(const int &id, string &&name, const ResourceInternalType &imageType, VkShaderStageFlagBits shaderStage) : GenericResource(move(name), move(string("Image")), GenericResourceType::GRT_IMAGE),
	m_id(id),
	m_unit(-1),
	m_imageType(imageType),
	m_shaderStage(shaderStage),
	m_hasAssignedTexture(false),
	m_hasAssignedGPUBuffer(false),
	m_texture(nullptr),
	m_gpuBuffer(nullptr),
	m_textureLevel(-1),
	m_layered(false),
	m_textureLayer(-1),
	m_accessType(ImageAccessType::IT_READ_ONLY),
	m_storingFormat(VkFormat::VK_FORMAT_MAX_ENUM),
	m_textureToSampleName(""),
	m_gpuBufferToSampleName("")
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Image::setTextureToImage(Texture *texture, const int unit, const int &textureLevel, const bool &layered, const int &textureLayer, const ImageAccessType &imageAccessType)
{
	if(texture == nullptr)
	{
		return false;
	}

	resetNonConstructorParameterValues();

	m_unit				  = unit;
	m_hasAssignedTexture  = true;
	m_texture             = texture;
	m_textureLevel        = textureLevel;
	m_layered             = layered;
	m_textureLayer        = textureLayer;
	m_accessType          = imageAccessType;
	m_textureToSampleName = m_texture->getName();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Image::setTextureToImage(string &&name, const int unit, const int &textureLevel, const bool &layered, const int &textureLayer, const ImageAccessType &imageAccessType)
{
	resetNonConstructorParameterValues();

	m_texture			  = nullptr;
	m_hasAssignedTexture  = setTextureToImage(textureM->getElement(move(name)), unit, textureLevel, layered, textureLayer, imageAccessType);
	m_textureToSampleName = move(name);

	return m_hasAssignedTexture;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Image::setGPUBufferToImage(GPUBuffer *gpuBuffer, const int unit, const ImageAccessType &imageAccessType)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Image::setGPUBufferToImage(string &&name, const int unit, const ImageAccessType &imageAccessType)
{
	return m_hasAssignedTexture;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Image::~Image()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Image::resetNonConstructorParameterValues()
{
	m_unit                  = -1;
	m_hasAssignedTexture    = false;
	m_hasAssignedGPUBuffer  = false;
	m_texture               = nullptr;
	m_gpuBuffer             = nullptr;
	m_textureLevel          = -1;
	m_layered               = false;
	m_textureLayer          = -1;
	m_accessType            = ImageAccessType::IT_READ_ONLY;
	m_storingFormat         = VkFormat::VK_FORMAT_MAX_ENUM;
	m_textureToSampleName   = "";
	m_gpuBufferToSampleName = "";
}

/////////////////////////////////////////////////////////////////////////////////////////////
