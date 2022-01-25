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
#include "../../include/texture/texturemanager.h"
#include "../../include/texture/texture.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/core/coremanager.h"
#include "../../include/core/physicaldevice.h"
#include "../../include/core/logicaldevice.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/texture/irradiancetexture.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
VkCommandBuffer TextureManager::commandBufferTexture;

/////////////////////////////////////////////////////////////////////////////////////////////

TextureManager::TextureManager()
{
	m_managerName = g_textureManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////

TextureManager::~TextureManager()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::destroyResources()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

Texture* TextureManager::build2DTextureFromFile(
	string&&          instanceName,
	string&&          filename,
	VkImageUsageFlags imageUsageFlags,
	VkFormat          format)
{
	if (existsElement(move(string(instanceName))))
	{
		return getElement(move(instanceName));
	}

	Texture* texture = new Texture(move(string(instanceName)));

	gli::texture2D*    imageGLI2D        = nullptr;
	TextureInfo*       textureInfo       = nullptr;
	IrradianceTexture* irradianceTexture = nullptr;

	if (isIrradianceTexture(move(string(filename))))
	{
		irradianceTexture = new IrradianceTexture(move(string(filename)));
		textureInfo       = new TextureInfo(irradianceTexture);
	}
	else
	{
		imageGLI2D  = new gli::texture2D(gli::texture2D(gli::load(filename)));
		assert(!imageGLI2D->empty());
		textureInfo = new TextureInfo(imageGLI2D);
	}

	const vector<TextureMipMapInfo>& vectorMipMap = textureInfo->getVectorMipMap();
	assert(vectorMipMap.size() != 0);

	// Get the image dimensions
	texture->m_width           = vectorMipMap[0].m_width;
	texture->m_height          = vectorMipMap[0].m_height;
	texture->m_depth           = vectorMipMap[0].m_depth;
	texture->m_mipMapLevels    = uint32_t(vectorMipMap.size()); // Get number of mip-map levels
	texture->m_generateMipmap  = false;
	texture->m_path            = move(filename);
	texture->m_format          = format;
	texture->m_imageUsageFlags = imageUsageFlags;
	texture->m_imageViewType   = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	texture->m_flags           = 0;

	// Create image info with optimal tiling support (VK_IMAGE_TILING_OPTIMAL)
	VkExtent3D imageExtent = { texture->m_width, texture->m_height, texture->m_depth };
	texture->m_image       = buildImage(format,
										imageExtent,
										texture->m_mipMapLevels,
										imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
										VK_SAMPLE_COUNT_1_BIT,
										VK_IMAGE_TILING_OPTIMAL,
										texture->m_imageViewType,
										texture->m_flags);

	texture->m_mem = buildImageMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->m_image, texture->m_memorySize);

	vector<VkExtent3D> vectorMipMapExtent;
	vector<uint> vectorMipMapSize;

	for (uint32_t i = 0; i < texture->m_mipMapLevels; ++i)
	{
		VkExtent3D mipExtent = { vectorMipMap[i].m_width, vectorMipMap[i].m_height, vectorMipMap[i].m_depth };
		vectorMipMapExtent.push_back(mipExtent);
		vectorMipMapSize.push_back(uint(vectorMipMap[i].m_size));
	}

	texture->m_imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	fillImageMemoryMipmaps(texture->m_mipMapLevels,
						   texture->m_image,
						   textureInfo->refData(),
						   uint(textureInfo->getSize()),
						   vectorMipMapExtent,
						   vectorMipMapSize,
						   texture->m_imageLayout,
						   texture->m_imageViewType);

	VkComponentMapping components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	texture->m_view = buildImageView(VK_IMAGE_ASPECT_COLOR_BIT, texture->m_image, components, texture->getMipMapLevels(), format, VK_IMAGE_VIEW_TYPE_2D);

	TextureManager::addElement(move(string(instanceName)), texture);
	texture->m_name = move(instanceName);

	if (imageGLI2D != nullptr)
	{
		delete imageGLI2D;
	}

	if (textureInfo != nullptr)
	{
		delete textureInfo;
	}

	if (irradianceTexture != nullptr)
	{
		delete irradianceTexture;
	}

	// PENDING: custom components
	
	return texture;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Texture* TextureManager::buildTextureFromData(
	string&&                  instanceName,
	VkImageUsageFlags         imageUsageFlags,
	VkFormat                  format,
	void*                     imageData,
	uint                      dataSize,
	const vector<VkExtent3D>& vectorMipMapExtent,
	const vector<uint>&       vectorMipMapSize,
	VkSampleCountFlagBits     samples,
	VkImageTiling             tiling,
	VkImageViewType           imageViewType)
{
	if (existsElement(move(string(instanceName))))
	{
		return getElement(move(instanceName));
	}

	Texture* texture = new Texture(move(string(instanceName)));
	
	// Get the image dimensions, vectorMipMapExtent[0] is assumed to contain mip map 0
	texture->m_width           = uint32_t(vectorMipMapExtent[0].width);
	texture->m_height          = uint32_t(vectorMipMapExtent[0].height);
	texture->m_depth           = uint32_t(vectorMipMapExtent[0].depth);
	texture->m_mipMapLevels    = uint32_t(vectorMipMapExtent.size());
	texture->m_generateMipmap  = false;
	texture->m_format          = format;
	texture->m_imageUsageFlags = imageUsageFlags;
	texture->m_imageViewType   = imageViewType;
	texture->m_flags           = 0;

	// Create image info with optimal tiling support (VK_IMAGE_TILING_OPTIMAL)
	texture->m_image = buildImage(format,
								  { texture->m_width, texture->m_height, texture->m_depth },
								  texture->m_mipMapLevels,
								  imageUsageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
								  samples,
								  tiling,
								  texture->m_imageViewType,
								  texture->m_flags);

	texture->m_mem         = buildImageMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->m_image, texture->m_memorySize);
	texture->m_imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	fillImageMemoryMipmaps(texture->m_mipMapLevels,
						   texture->m_image,
						   imageData,
						   dataSize,
						   vectorMipMapExtent,
						   vectorMipMapSize,
						   texture->m_imageLayout,
						   texture->m_imageViewType);
	
	VkComponentMapping components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

	// TODO: put in separate static method

	texture->m_view = buildImageView(VK_IMAGE_ASPECT_COLOR_BIT, texture->m_image, components, 1, format, texture->m_imageViewType);

	TextureManager::addElement(move(string(instanceName)), texture);
	texture->m_name = move(instanceName);

	// PENDING: custom components

	return texture;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Texture* TextureManager::buildTexture(
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
	VkImageCreateFlags    flags)
{
	if (existsElement(move(string(instanceName))))
	{
		return getElement(move(instanceName));
	}

	Texture* texture = new Texture(move(string(instanceName)));

	texture->m_width           = extent.width;
	texture->m_height          = extent.height;
	texture->m_depth           = extent.depth;
	texture->m_mipMapLevels    = 1;
	texture->m_generateMipmap  = false;
	texture->m_imageUsageFlags = usage;
	texture->m_imageViewType   = imageViewType;
	texture->m_flags           = flags;

	texture->m_image           = buildImage(format, extent, 1, usage, samples, tiling, texture->m_imageViewType, texture->m_flags);
	//texture->m_mem             = buildImageMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT/*0*/, texture->m_image, texture->m_memorySize);
	texture->m_mem             = buildImageMemory(properties, texture->m_image, texture->m_memorySize);

	// Use command buffer to create the depth image. This includes -
	// Command buffer allocation, recording with begin/end scope and submission.
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), &commandBufferTexture);
	coreM->beginCommandBuffer(commandBufferTexture);
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask   = layoutAspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount   = 1;
		subresourceRange.layerCount   = (texture->m_imageViewType == VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;

		// Set the image layout (to optimal)
		setImageLayout(texture->m_image, aspectMask, oldImageLayout, newImageLayout, subresourceRange);
	}
	coreM->endCommandBuffer(commandBufferTexture);
	coreM->submitCommandBuffer(coreM->getLogicalDeviceGraphicsQueue(), &commandBufferTexture);

	// TODO: put in separate static method
	texture->m_view = buildImageView(aspectMask, texture->m_image, { VK_COMPONENT_SWIZZLE_IDENTITY }, 1, format, texture->m_imageViewType);

	TextureManager::addElement(move(string(instanceName)), texture);
	texture->m_name        = move(instanceName);
	texture->m_format      = format;
	texture->m_imageLayout = newImageLayout;

	// PENDING: FILL Texture::descsImgInfo fields
	// PENDING BUILD SAMPLER
	// PENDING: custom components

	return texture;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::destroyCommandBuffer()
{
	VkCommandBuffer cmdBufs[] = { commandBufferTexture };
	vkFreeCommandBuffers(coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), sizeof(cmdBufs) / sizeof(VkCommandBuffer), cmdBufs);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Texture* TextureManager::buildTextureFromExistingResources(
	string&&        instanceName,
	VkImage         image,
	VkImageView     view,
	uint32_t        width,
	uint32_t        height,
	VkFormat        format,
	VkImageViewType imageViewType)
{
	if (existsElement(move(string(instanceName))))
	{
		return getElement(move(instanceName));
	}

	Texture* texture = new Texture(move(string(instanceName)));

	texture->m_image           = image;
	texture->m_imageLayout     = VK_IMAGE_LAYOUT_MAX_ENUM;
	texture->m_mem             = VK_NULL_HANDLE;
	texture->m_view            = view;
	texture->m_mipMapLevels    = 0;
	texture->m_layerCount      = 1;
	texture->m_width           = width;
	texture->m_height          = height;
	texture->m_depth           = 1;
	texture->m_generateMipmap  = false;
	texture->m_imageUsageFlags = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
	texture->m_format          = format;
	texture->m_imageViewType   = imageViewType;
	texture->m_flags           = 0;

	TextureManager::addElement(move(string(instanceName)), texture);
	texture->m_name = move(instanceName);

	return texture;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::setImageLayout(VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, const VkImageSubresourceRange& subresourceRange)
{
	// TODO: assign texture->m_imageLayout value

	// Dependency on cmd
	assert(commandBufferTexture != VK_NULL_HANDLE);

	// The deviceObj->queue must be initialized
	assert(coreM->getLogicalDeviceGraphicsQueue() != VK_NULL_HANDLE);

	VkImageMemoryBarrier imgMemoryBarrier = {};
	imgMemoryBarrier.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgMemoryBarrier.pNext            = NULL;
	imgMemoryBarrier.srcAccessMask    = 0;
	imgMemoryBarrier.dstAccessMask    = 0;
	imgMemoryBarrier.oldLayout        = oldImageLayout;
	imgMemoryBarrier.newLayout        = newImageLayout;
	imgMemoryBarrier.image            = image;
	imgMemoryBarrier.subresourceRange = subresourceRange;

	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	// Source layouts (old)
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imgMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	switch (newImageLayout)
	{
		// Ensure that anything that was copying from this image has completed
		// An image in this layout can only be used as the destination operand of the commands
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

		// Ensure any Copy or CPU writes to image are flushed
		// An image in this layout can only be used as a read-only shader resource
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

		// An image in this layout can only be used as a framebuffer color attachment
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		//imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

		// An image in this layout can only be used as a framebuffer depth/stencil attachment
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imgMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	}

	// TODO: avoid using VK_PIPELINE_STAGE_ALL_COMMANDS_BIT and particularize to the proper type of barrier
	VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT/*VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT*/;
	VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT/*VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT*/;

	vkCmdPipelineBarrier(commandBufferTexture, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imgMemoryBarrier);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkImageView TextureManager::buildImageView(VkImageAspectFlags aspectMask, VkImage image, VkComponentMapping components, uint32_t levelCount, VkFormat format, VkImageViewType viewType)
{
	VkImageViewCreateInfo imgViewInfo = {};
	imgViewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewInfo.pNext                           = NULL;
	imgViewInfo.image                           = image;
	imgViewInfo.format                          = format;
	imgViewInfo.components                      = components; //{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A }
	imgViewInfo.subresourceRange.aspectMask     = aspectMask;
	imgViewInfo.subresourceRange.baseMipLevel   = 0;
	imgViewInfo.subresourceRange.levelCount     = levelCount;
	imgViewInfo.subresourceRange.baseArrayLayer = 0;
	imgViewInfo.subresourceRange.layerCount     = (viewType == VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;;
	imgViewInfo.viewType                        = viewType/*VK_IMAGE_VIEW_TYPE_2D*/;
	imgViewInfo.flags                           = 0;

	VkImageView imageView;
	VkResult result = vkCreateImageView(coreM->getLogicalDevice(), &imgViewInfo, nullptr, &imageView);
	assert(result == VK_SUCCESS);

	return imageView;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkSampler TextureManager::buildSampler(float minLod, float maxLod, VkSamplerMipmapMode mipmapMode, VkFilter minMagFilter)
{
	// Create sampler
	VkSamplerCreateInfo samplerCI = {};
	samplerCI.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.pNext                   = NULL;
	samplerCI.magFilter               = minMagFilter;
	samplerCI.minFilter               = minMagFilter;
	samplerCI.mipmapMode              = mipmapMode;
	samplerCI.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCI.mipLodBias              = 0.0f;
	samplerCI.anisotropyEnable        = VK_FALSE;
	samplerCI.maxAnisotropy           = 1; // could not be neccessary
	samplerCI.compareEnable           = VK_FALSE; // Implemented because of the voxelization step
	samplerCI.compareOp               = VK_COMPARE_OP_ALWAYS;  // Implemented because of the voxelization step, old value was VK_COMPARE_OP_NEVER
	samplerCI.minLod                  = minLod;
	samplerCI.maxLod                  = maxLod;
	samplerCI.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCI.unnormalizedCoordinates = VK_FALSE;

	VkSampler sampler;

	VkResult error = vkCreateSampler(coreM->getLogicalDevice(), &samplerCI, nullptr, &sampler);
	assert(!error);

	return sampler;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkImage TextureManager::buildImage(
	VkFormat              format,
	VkExtent3D            extent,
	uint32_t              mipLevels,
	VkImageUsageFlags     usage,
	VkSampleCountFlagBits samples,
	VkImageTiling         tiling,
	VkImageViewType       imageViewType,
	VkImageCreateFlags    flags)
{
	// TODO: put in separate static method

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext                 = NULL;
	imageInfo.imageType             = getImageType(imageViewType);
	imageInfo.format                = format;
	imageInfo.extent                = extent;
	imageInfo.mipLevels             = mipLevels;
	imageInfo.arrayLayers           = (imageViewType == VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;
	imageInfo.samples               = samples;
	imageInfo.tiling                = tiling; /*VK_IMAGE_TILING_OPTIMAL*/;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices   = NULL;
	imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.usage                 = usage;
	imageInfo.flags                 = flags;
	imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image;
	VkResult result = vkCreateImage(coreM->getLogicalDevice(), &imageInfo, nullptr, &image);
	assert(result == VK_SUCCESS);

	return image;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkDeviceMemory TextureManager::buildImageMemory(VkFlags requirementsMask, VkImage& image, VkDeviceSize& memorySize)
{
	VkResult result;

	VkMemoryRequirements memRqrmnt;
	vkGetImageMemoryRequirements(coreM->getLogicalDevice(), image, &memRqrmnt);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.pNext           = NULL;
	memAlloc.memoryTypeIndex = 0;
	memAlloc.allocationSize  = memRqrmnt.size;
	memorySize               = memRqrmnt.size;

	VkDeviceMemory memory;
	// Determine the type of memory required with the help of memory properties
	bool propertiesResult = coreM->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, requirementsMask, coreM->getPhysicalDeviceMemoryProperties().memoryTypes, memAlloc.memoryTypeIndex);

	assert(propertiesResult);

	// Allocate the memory for image objects
	result = vkAllocateMemory(coreM->getLogicalDevice(), &memAlloc, nullptr, &memory);
	assert(result == VK_SUCCESS);

	// Bind the allocated memeory
	result = vkBindImageMemory(coreM->getLogicalDevice(), image, memory, 0);
	assert(result == VK_SUCCESS);

	return memory;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::fillImageMemoryMipmaps(
	uint32_t                  mipMap,
	VkImage&                  image,
	void*                     imageData,
	uint                      dataSize,
	const vector<VkExtent3D>& vectorMipMapExtent,
	const vector<uint>&       vectorMipMapSize,
	VkImageLayout             destinationLayout,
	VkImageViewType           imageViewType)
{
	Buffer* buffer = bufferM->buildBuffer(move(string("fillBuffer")), imageData, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount   = mipMap;
	subresourceRange.layerCount   = (imageViewType == VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;

	// Use a separate command buffer for texture loading, start command buffer recording
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), &commandBufferTexture);
	coreM->beginCommandBuffer(commandBufferTexture);

	// set the image layout to be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL since it is destination for copying buffer into image using vkCmdCopyBufferToImage -
	setImageLayout(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, destinationLayout, subresourceRange);

	uint32_t bufferOffset = 0;
	vector<VkBufferImageCopy> bufferImgCopyList; // List contains the buffer image copy for each mipLevel -
													  // Iterater through each mip level and set buffer image copy -
	for (uint32_t i = 0; i < mipMap; i++)
	{
		VkBufferImageCopy bufImgCopyItem = {};
		bufImgCopyItem.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		bufImgCopyItem.imageSubresource.mipLevel       = i;
		bufImgCopyItem.imageSubresource.layerCount     = (imageViewType == VK_IMAGE_VIEW_TYPE_CUBE) ? 6 : 1;
		bufImgCopyItem.imageSubresource.baseArrayLayer = 0;
		bufImgCopyItem.imageExtent                     = vectorMipMapExtent[i];
		bufImgCopyItem.bufferOffset                    = bufferOffset;
										
		bufferImgCopyList.push_back(bufImgCopyItem);
		bufferOffset += uint32_t(vectorMipMapSize[i]); // adjust buffer offset
	}

	// Copy the staging buffer memory data contain the stage raw data(with mip levels) into image object
	vkCmdCopyBufferToImage(commandBufferTexture, buffer->getBuffer(), image, destinationLayout, uint32_t(bufferImgCopyList.size()), bufferImgCopyList.data());

	// Advised to change the image layout to shader read after staged buffer copied into image memory -
	setImageLayout(image, VK_IMAGE_ASPECT_COLOR_BIT, destinationLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

	coreM->endCommandBuffer(commandBufferTexture); // Submit command buffer containing copy and image layout commands-

															  // Create a fence object to ensure that the command buffer is executed, coping our staged raw data from the buffers to image memory with respective image layout and attributes into consideration -
	VkFence fence;
	VkFenceCreateInfo fenceCI = {};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.flags = 0;

	VkResult error = vkCreateFence(coreM->getLogicalDevice(), &fenceCI, nullptr, &fence);
	assert(!error);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext              = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &commandBufferTexture;

	coreM->submitCommandBuffer(coreM->getLogicalDeviceGraphicsQueue(), &commandBufferTexture, &submitInfo, fence);

	error = vkWaitForFences(coreM->getLogicalDevice(), 1, &fence, VK_TRUE, 10000000000);
	assert(!error);

	// destroy resources used for this operation
	vkDestroyFence(coreM->getLogicalDevice(), fence, nullptr);
	bufferM->removeElement(move(string("fillBuffer")));
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool TextureManager::isIrradianceTexture(string&& path)
{
	if (path.size() < 5)
	{
		return false;
	}

	string extension = path.substr(path.size() - 5, 5);

	if (strcmp(extension.c_str(), string(".irrt").c_str()))
	{
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkImageType TextureManager::getImageType(VkImageViewType imageViewType)
{
	switch (imageViewType)
	{
		case VkImageViewType::VK_IMAGE_VIEW_TYPE_1D:
		{
			return VkImageType::VK_IMAGE_TYPE_1D;
		}
		case VkImageViewType::VK_IMAGE_VIEW_TYPE_2D:
		{
			return VkImageType::VK_IMAGE_TYPE_2D;
		}
		case VkImageViewType::VK_IMAGE_VIEW_TYPE_3D:
		{
			return VkImageType::VK_IMAGE_TYPE_3D;
		}
		case VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE:
		{
			return VkImageType::VK_IMAGE_TYPE_2D;
		}
		default:
		{
			cout << "ERROR: no image view type case available" << endl;
			assert(1);
		}
	}

	return VkImageType::VK_IMAGE_TYPE_2D;
}

/////////////////////////////////////////////////////////////////////////////////////////////
