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
#include "../../include/shader/sampler.h"
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/core/coremanager.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Sampler::Sampler(string &&name, const ResourceInternalType &samplerType, VkShaderStageFlagBits shaderStage, int bindingIndex, int setIndex) : GenericResource(move(name), move(string("Sampler")), GenericResourceType::GRT_SAMPLER)
	, m_samplerHandle(VK_NULL_HANDLE)
	, m_samplerType(samplerType)
	, m_shaderStage(shaderStage)
	, m_hasAssignedTexture(false)
	, m_bindingIndex(bindingIndex)
	, m_setIndex(setIndex)
	, m_texture(nullptr)
	, m_isImageSampler(false)
	, m_samplerFormat(ResourceInternalType::RIT_SIZE)
	, m_descriptorType(VK_DESCRIPTOR_TYPE_MAX_ENUM)
	, m_mipmapMode(VK_SAMPLER_MIPMAP_MODE_LINEAR)
	, m_minMagFilter(VkFilter::VK_FILTER_LINEAR)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Sampler::Sampler(string &&name, const ResourceInternalType &samplerType, VkShaderStageFlagBits shaderStage, int bindingIndex, int setIndex, ResourceInternalType samplerFormat) : GenericResource(move(name), move(string("Sampler")), GenericResourceType::GRT_SAMPLER)
	, m_samplerHandle(VK_NULL_HANDLE)
	, m_samplerType(samplerType)
	, m_shaderStage(shaderStage)
	, m_hasAssignedTexture(false)
	, m_bindingIndex(bindingIndex)
	, m_setIndex(setIndex)
	, m_texture(nullptr)
	, m_isImageSampler(true)
	, m_samplerFormat(samplerFormat)
	, m_descriptorType(VK_DESCRIPTOR_TYPE_MAX_ENUM)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Sampler::setTextureToSample(Texture *texture)
{
	if (texture == nullptr)
	{
		return false;
	}

	m_texture             = texture;
	m_hasAssignedTexture  = true;
	m_textureToSampleName = m_texture->getName();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Sampler::setTextureToSample(string &&name)
{
	m_texture			  = nullptr;
	m_hasAssignedTexture  = setTextureToSample(textureM->getElement(move(name)));
	m_textureToSampleName = move(name);

	return m_hasAssignedTexture;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Sampler::~Sampler()
{
	if (m_samplerHandle != VK_NULL_HANDLE)
	{
		vkDestroySampler(coreM->getLogicalDevice(), m_samplerHandle, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
