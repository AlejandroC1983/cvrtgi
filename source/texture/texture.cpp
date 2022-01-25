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

#include "../../include/texture/texture.h"
#include "../../include/core/coremanager.h"

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/texture/texture.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

// TODO initialize properly
Texture::Texture(string &&name) : GenericResource(move(name), move(string("Texture")), GenericResourceType::GRT_TEXTURE)
	, m_image(VK_NULL_HANDLE)
	, m_imageLayout(VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED)
	, m_mem(VK_NULL_HANDLE)
	, m_memorySize(0)
	, m_view(VK_NULL_HANDLE)
	, m_mipMapLevels(0)
	, m_layerCount(0)
	, m_width(0)
	, m_height(0)
	, m_depth(0)
	, m_generateMipmap(false)
	, m_imageUsageFlags(VkImageUsageFlagBits::VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM)
	, m_format(VK_FORMAT_UNDEFINED)
	, m_imageViewType(VkImageViewType::VK_IMAGE_VIEW_TYPE_2D)
	, m_flags(0)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
	// TODO: put in a destroy method
	if (m_mem != nullptr)
	{
		vkFreeMemory(coreM->getLogicalDevice(), m_mem, nullptr);
	}

	if (!m_isSwapChainTex)
	{
		vkDestroyImage(coreM->getLogicalDevice(), m_image, nullptr);
	}

	if (!m_isSwapChainTex)
	{
		int a = 0;
	}

	vkDestroyImageView(coreM->getLogicalDevice(), m_view, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Texture::getContentCopy(vectorUint8& vectorData)
{
	void* mappedMemory;
	VkResult result = vkMapMemory(coreM->getLogicalDevice(), m_mem, 0, uint(m_memorySize), 0, &mappedMemory);
	assert(result == VK_SUCCESS);
	vectorData.resize(m_memorySize);
	memcpy((void*)vectorData.data(), mappedMemory, m_memorySize);
	vkUnmapMemory(coreM->getLogicalDevice(), m_mem);
	return (result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////
