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

#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/singleton.h"
#include "../../include/util/managertemplate.h"
#include "../../include/texture/textureinfo.h"
#include "../headers.h"

// CLASS FORWARDING
class Texture;

// NAMESPACE

// DEFINES
#define textureM s_pTextureManager->instance()

/////////////////////////////////////////////////////////////////////////////////////////////

class TextureManager: public ManagerTemplate<Texture>, public Singleton<TextureManager>
{
	friend class SwapChain;
	friend class CoreManager;

public:
	/** Constructor
	* @return nothing */
	TextureManager();

	/** Destructor
	* @return nothing */
	virtual ~TextureManager();

	/** Destroys all elements in the manager
	* @return nothing */
	void destroyResources();

public:
	/** Builds a texture from the file whose path is given as parameter, with usage flags and format given as parameter
	* @param instanceName    [in] name of the new instance (the m_sName member variable)
	* @param path            [in] path to the texture file
	* @param imageUsageFlags [in] usage flags for the new texture
	* @param format          [in] format of the texture
	* @return a pointer to the built texture, nullptr if any error while building it */
	Texture* build2DTextureFromFile(
		string&&          instanceName,
		string&&          path,
		VkImageUsageFlags imageUsageFlags,
		VkFormat          format);

	/** Builds a texture from the data given
	* @param instanceName       [in] name of the new instance (the m_sName member variable)
	* @param imageUsageFlags    [in] usage flags for the new texture
	* @param format             [in] format of the texture
	* @param imageData          [in] void pointer to the full image buffer data (including all mip map levels)
	* @param dataSize           [in] size of the imageData buffer in bytes
	* @param vectorMipMapExtent [in] vector with the image dimensions of each of the mip map levels to generate, present in imageData and given by the mipMap parameter
	* @param vectorMipMapSize   [in] vector with the extent of the buffer memory of each of the mip map levels to generate, present in imageData and given by the mipMap parameter
	* @param samples            [in] number of samples per pixel when using the image for storage operations
	* @param tiling             [in] tiling type
	* @param imageViewType      [in] image view type, will decide the image type used
	* @return a pointer to the built texture, nullptr if any error while building it */
	Texture* buildTextureFromData(
		string&&                  instanceName,
		VkImageUsageFlags         imageUsageFlags,
		VkFormat                  format,
		void*                     imageData,
		uint                      dataSize,
		const vector<VkExtent3D>& vectorMipMapExtent,
		const vector<uint>&       vectorMipMapSize,
		VkSampleCountFlagBits     samples,
		VkImageTiling             tiling,
		VkImageViewType           imageViewType);

	/** Builds a texture with format and extent given as parameter. Also an aspect mask and old / new layout to transition it to
	* optimal formats
	* @param instanceName     [in] name of the new instance (the m_sName member variable)
	* @param format           [in] format of the texture
	* @param extent           [in] texture extent
	* @param usage            [in] usage flags for the new texture
	* @param aspectMask       [in] aspect mask
	* @param layoutAspectMask [in] layout aspect mask
	* @param oldImageLayout   [in] old layout aspect mask for transitioning to optimal format
	* @param newImageLayout   [in] new layout aspect mask for transitioning to optimal format
	* @param properties       [in] memory properties flag for the memory allocation process of the image
	* @param samples          [in] number of samples per pixel when using the image for storage operations
	* @param tiling           [in] tiling type
	* @param imageViewType    [in] image view type, will decide the image type used
	* @param flags            [in] image flags
	* @return a pointer to the built texture, nullptr if any error while building it */
	Texture* buildTexture(
		string&&              instanceName,
		VkFormat              format,
		VkExtent3D            extent,
		VkImageUsageFlags     usage,
		VkImageAspectFlags    aspectMask,
		VkImageAspectFlags    layoutAspectMask,
		VkImageLayout         oldImageLayout,
		VkImageLayout         newImageLayout,
		VkMemoryPropertyFlags properties,
		VkSampleCountFlagBits samples,
		VkImageTiling         tiling,
		VkImageViewType       imageViewType,
		VkImageCreateFlags    flags);

	/** Builds an sampler with min and max defined lod
	* @param minLod       [in] sampler min lod
	* @param maxLod       [in] sampler max lod
	* @param mipmapMode   [in] mip map mode
	* @param minMagFilter [in] flag for sampler minification / magnification filter
	* @return built VkSampler */
	VkSampler buildSampler(float minLod, float maxLod, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter);

protected:
	/** Builds an VkImage with the format, size, usage and mip level number given as parameter
	* @param format        [in] image format
	* @param extent        [in] image extent
	* @param mipLevels     [in] number of mip levels
	* @param usage         [in] usage flags for the new texture
	* @param samples       [in] number of samples per pixel when using the image for storage operations
	* @param tiling        [in] tiling type
	* @param imageViewType [in] image view type, will decide the image type used
	* @param flags         [in] image flags
	* @return built image */
	VkImage buildImage(
		VkFormat              format,
		VkExtent3D            extent,
		uint32_t              mipLevels,
		VkImageUsageFlags     usage,
		VkSampleCountFlagBits samples,
		VkImageTiling         tiling,
		VkImageViewType       imageViewType,
		VkImageCreateFlags    flags);

	/** Builds the memory for the image given as parameter
	* @param requirementsMask [in]    requirement mask
	* @param image            [inout] image to build the memory for
	* @param memorySize       [inout] memory size of the built image
	* @return built image memory */
	VkDeviceMemory buildImageMemory(VkFlags requirementsMask, VkImage& image, VkDeviceSize& memorySize);

	/** Builds mipmaps and fills them with the information from the supplied gliTexture
	* @param mipMap             [in] number of mip maps to fill information with
	* @param image              [in] image to fill information
	* @param imageData          [in] void pointer to the full image buffer data (including all mip map levels)
	* @param dataSize           [in] size of the imageData buffer in bytes
	* @param vectorMipMapExtent [in] vector with the image dimensions of each of the mip map levels to generate, present in imageData and given by the mipMap parameter
	* @param vectorMipMapSize   [in] vector with the extent of the buffer memory of each of the mip map levels to generate, present in imageData and given by the mipMap parameter
	* @param destinationLayout  [in] new layout to use when transitioning the image through a pipeline barrier
	* @param imageViewType      [in] image view type
	* @return nothing */
	void fillImageMemoryMipmaps(
		uint32_t                  mipMap,
		VkImage&                  image,
		void*                     imageData,
		uint                      dataSize,
		const vector<VkExtent3D>& vectorMipMapExtent,
		const vector<uint>&       vectorMipMapSize,
		VkImageLayout             destinationLayout,
		VkImageViewType           imageViewType);

	/** Transitions the layout of the image given as parameter from the old to the new layout
	* @param image            [in] image to transition
	* @param aspectMask       [in] image aspect mask
	* @param oldImageLayout   [in] old layout
	* @param newImageLayout   [in] new layout
	* @param subresourceRange [in] image subresource range
	* @return nothing */
	void setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange);

	/** Builds an image view for the image given as parameter
	* @param aspectMask [in] image aspect mask
	* @param image      [in] image to build a view from
	* @param components [in] image component mapping
	* @param levelCount [in] image subresource number of levels
	* @param viewType   [in] enum with the view type 
	* @return built image view */
	VkImageView buildImageView(VkImageAspectFlags aspectMask, VkImage image, VkComponentMapping components, uint32_t levelCount, VkFormat format, VkImageViewType viewType);

	/** Destroys TextureManager::commandBufferTexture
	* @return nothing */
	void destroyCommandBuffer();

	/** To add the swap chain color textures, build a Texture resource from the resources
	* given by the swap chain
	* @param instanceName  [in] name of the new instance (the m_sName member variable)
	* @param image         [in] image handler
	* @param view          [in] view handler
	* @param width         [in] image width
	* @param height        [in] image height
	* @param format        [in] image format
	* @param imageViewType [in] image view type
	* @return a pointer to the added texture */
	Texture* buildTextureFromExistingResources(
		string&&          instanceName,
		VkImage           image,
		VkImageView       view,
		uint32_t          width,
		uint32_t          height,
		VkFormat          format,
		VkImageViewType   imageViewType);

	/** To identify irradiance texture loading
	* @return true if the texture name present in the path parameter has extension ".irrt" and false otherwise */
	bool isIrradianceTexture(string&& path);

	/** Infer the image type given its extent through the extent parameter
	* @param imageViewType [in] image view type
	* @return image type */
	VkImageType getImageType(VkImageViewType imageViewType);

	static VkCommandBuffer commandBufferTexture; //!< Command buffer for operations related with this manager
};

static TextureManager* s_pTextureManager;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _TEXTUREMANAGER_H_
