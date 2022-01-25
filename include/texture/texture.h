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

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/genericresource.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Texture : public GenericResource
{
	friend class TextureManager;

protected:
	/** Parameter constructor
	* @param [in] texture's name
	* @return nothing */
	Texture(string &&name);

public:
	/** Destructor
	* @return nothing */
	virtual ~Texture();

	/** Returns a vector of byte with the information of the image memory
	* @return true if the copy operation was made successfully, false otherwise */
	bool getContentCopy(vectorUint8& vectorData);

	GET(VkImage, m_image, Image)
	GETCOPY(VkImageLayout, m_imageLayout, ImageLayout)
	GET(VkDeviceMemory, m_mem, Mem)
	GET(VkImageView, m_view, View)
	GETCOPY(uint32_t, m_mipMapLevels, MipMapLevels)
	GETCOPY(uint32_t, m_layerCount, LayerCount)
	GETCOPY(uint32_t, m_width, Width)
	GETCOPY(uint32_t, m_height, Height)
	GETCOPY(uint32_t, m_depth, Depth)
	GETCOPY(bool, m_generateMipmap, GenerateMipmap)
	GETCOPY(string, m_path, Path)
	GETCOPY(VkImageUsageFlags, m_imageUsageFlags, ImageUsageFlags)
	GETCOPY(VkFormat, m_format, Format)
	GETCOPY(VkImageViewType, m_imageViewType, ImageViewType)
	GETCOPY(VkImageCreateFlags, m_flags, Flags)
	GETCOPY_SET(bool, m_isSwapChainTex, IsSwapChainTex)

protected:
	VkImage            m_image;           //!< Texture image
	VkImageLayout      m_imageLayout;     //!< Enum with the image layout
	VkDeviceMemory     m_mem;             //!< Image memory
	VkDeviceSize       m_memorySize;      //!< Size of the image memory
	VkImageView        m_view;            //!< Image view
	uint32_t           m_mipMapLevels;    //!< Number of image mip-map levels
	uint32_t           m_layerCount;      //!< Number of image layers
	uint32_t           m_width;           //!< Image width
	uint32_t           m_height;          //!< Image height
	uint32_t           m_depth;           //!< Image depth
	bool               m_generateMipmap;  //!< If true, mipmaps will be generated when setting the data of the texture
	string             m_path;            //!< Texture path to the image file this texure has loaded (if any)
	VkImageUsageFlags  m_imageUsageFlags; //!< Enum with the usage flags of this image
	VkFormat           m_format;          //!< Image format
	VkImageViewType    m_imageViewType;   //!< Image view type
	VkImageCreateFlags m_flags;           //!< Image flags
	bool               m_isSwapChainTex;  //!< Flag to know if this texture is a swapchain texture
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _TEXTURE_H_
