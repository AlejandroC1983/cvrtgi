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
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/buffer/buffer.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

VkRenderPassBeginInfo VulkanStructInitializer::renderPassBeginInfo(VkRenderPass          renderPass,
																   VkFramebuffer         framebuffer,
																   VkRect2D              renderArea,
																   vector<VkClearValue>& arrayClearValue)
{
	VkRenderPassBeginInfo renderPassBegin;
	renderPassBegin.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.pNext           = NULL;
	renderPassBegin.renderPass      = renderPass;
	renderPassBegin.framebuffer     = framebuffer;
	renderPassBegin.renderArea      = renderArea;
	renderPassBegin.clearValueCount = uint32_t(arrayClearValue.size());
	renderPassBegin.pClearValues    = arrayClearValue.data();

	return renderPassBegin;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VulkanStructInitializer::insertCopyImageCommand(Texture* source, Texture* destination, VkCommandBuffer* commandBuffer)
{
	// Transition destination copy, destination, to layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext               = nullptr;
	imageMemoryBarrier.srcAccessMask       = 0;
	imageMemoryBarrier.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.oldLayout           = destination->getImageLayout(); // destinationOldLayout;
	imageMemoryBarrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image               = destination->getImage();
	imageMemoryBarrier.subresourceRange    = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT,
																	 0,
																	 1,
																	 0,
																	 (destination->getImageViewType() == VK_IMAGE_VIEW_TYPE_CUBE) ? uint32_t(6) : uint32_t(1)};

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);

	// Transition source copy, source, to layout VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imageMemoryBarrier.oldLayout     = source->getImageLayout();
	imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.image         = source->getImage();

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);

	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width              = source->getWidth();
	imageCopyRegion.extent.height             = source->getHeight();
	imageCopyRegion.extent.depth              = source->getDepth();

	vkCmdCopyImage(
		*commandBuffer,
		source->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		destination->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopyRegion);

	// Transition destination copy, destination, back to layout destination->getImageLayout()
	imageMemoryBarrier.image         = destination->getImage();
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = 0;
	imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout     = destination->getImageLayout(); // destinationOldLayout; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);

	// Transition source copy, source, back to layout source->getImageLayout()
	imageMemoryBarrier.image         = source->getImage();
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imageMemoryBarrier.dstAccessMask = 0;
	imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.newLayout     = source->getImageLayout(); // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VulkanStructInitializer::insertImageMemoryBarrierCommand(Texture*             texture,
															  VkPipelineStageFlags srcStageMask,
															  VkPipelineStageFlags dstStageMask,
															  VkImageLayout        oldLayout,
															  VkImageLayout        newLayout,
															  VkAccessFlags        sourceAccessFlags,
															  VkAccessFlags        destinationAccessFlags,
															  VkImageAspectFlags   aspectMask,
															  uint32_t             baseArrayLayer,
															  VkCommandBuffer*     commandBuffer)
{
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.srcAccessMask       = sourceAccessFlags;
	imageMemoryBarrier.dstAccessMask       = destinationAccessFlags;
	imageMemoryBarrier.oldLayout           = oldLayout;
	imageMemoryBarrier.newLayout           = newLayout;
	imageMemoryBarrier.image               = texture->getImage();
	imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{aspectMask,
																  0,
																  1,
																  baseArrayLayer,
																  uint32_t(1)};
																  //(texture->getImageViewType() == VK_IMAGE_VIEW_TYPE_CUBE) ? uint32_t(6) : uint32_t(1)};

	vkCmdPipelineBarrier(
		*commandBuffer,
		srcStageMask,
		dstStageMask,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VulkanStructInitializer::insertImageAccessBarrierCommand(Texture*         texture,
															  VkAccessFlags    sourceAccessFlags,
															  VkAccessFlags    destinationAccessFlags,
															  VkCommandBuffer* commandBuffer)
{
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.srcAccessMask       = sourceAccessFlags;
	imageMemoryBarrier.dstAccessMask       = destinationAccessFlags;
	imageMemoryBarrier.oldLayout           = VK_IMAGE_LAYOUT_GENERAL; // texture->getImageLayout();
	imageMemoryBarrier.newLayout           = VK_IMAGE_LAYOUT_GENERAL; // texture->getImageLayout();
	imageMemoryBarrier.image               = texture->getImage();
	imageMemoryBarrier.subresourceRange    = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT,
																	 0,
																	 1,
																	 0,
																	 (texture->getImageViewType() == VK_IMAGE_VIEW_TYPE_CUBE) ? uint32_t(6) : uint32_t(1)};

	vkCmdPipelineBarrier(
		*commandBuffer,
		VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VulkanStructInitializer::insertBufferMemoryBarrier(vectorBufferPtr&     vectorBuffer,
														VkAccessFlags        srcAccessMask,
														VkAccessFlags        dstAccessMask,
														VkPipelineStageFlags srcStageMask,
														VkPipelineStageFlags dstStageMask,
														VkCommandBuffer*     commandBuffer)
{
	const uint maxIndex = uint(vectorBuffer.size());
	vector<VkBufferMemoryBarrier> vectorBufferBarrier;
	vectorBufferBarrier.resize(maxIndex);

	forI(maxIndex)
	{
		vectorBufferBarrier[i].sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		vectorBufferBarrier[i].pNext               = nullptr;
		vectorBufferBarrier[i].srcAccessMask       = srcAccessMask;
		vectorBufferBarrier[i].dstAccessMask       = dstAccessMask;
		vectorBufferBarrier[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vectorBufferBarrier[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vectorBufferBarrier[i].offset              = 0;
		vectorBufferBarrier[i].buffer              = vectorBuffer[i]->getBuffer();
		vectorBufferBarrier[i].size                = vectorBuffer[i]->getDescriptorBufferInfo().range;

		vkCmdPipelineBarrier(
			*commandBuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, NULL,
			1, &vectorBufferBarrier[i],
			0, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VulkanStructInitializer::insertBufferMemoryBarrier(Buffer*              buffer,
														VkAccessFlags        srcAccessMask,
														VkAccessFlags        dstAccessMask,
														VkPipelineStageFlags srcStageMask,
														VkPipelineStageFlags dstStageMask,
														VkCommandBuffer*     commandBuffer)
{
	vectorBufferPtr vectorBuffer;
	vectorBuffer.push_back(buffer);
	insertBufferMemoryBarrier(vectorBuffer, srcAccessMask, dstAccessMask, srcStageMask, dstStageMask, commandBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void VulkanStructInitializer::getTextureData(Texture* texture, VkFormat format, vectorVectorVec4& vectorResult0, vectorVectorFloat& vectorResult1)
{
	if ((format != VK_FORMAT_R32G32B32A32_SFLOAT) &&
		(format != VK_FORMAT_R32_SFLOAT) &&
		(format != VK_FORMAT_R16_SFLOAT))
	{
		cout << "ERROR: no proper format given in getTextureData" << endl;
		return;
	}

	// TODO: remove texture at the end of the method, remove index variable
	static int index = 0;
	Texture* debugSummedAreaResult = textureM->buildTexture(
		move(string("debugSummedAreaResult" + to_string(index))),
		format,
		VkExtent3D({ uint32_t(texture->getWidth()), uint32_t(texture->getHeight()), uint32_t(1) }),
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_VIEW_TYPE_2D,
		0);
	index++;

	VkCommandBuffer commandBufferTexture;
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), &commandBufferTexture);
	coreM->beginCommandBuffer(commandBufferTexture);
	
	// Transition source image to proper layout (VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.srcAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
	imageMemoryBarrier.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
	imageMemoryBarrier.oldLayout           = texture->getImageLayout(); //VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.image               = texture->getImage();
	imageMemoryBarrier.subresourceRange    = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT,
																	 0,
																	 1,
																	 0,
																	 (texture->getImageViewType() == VK_IMAGE_VIEW_TYPE_CUBE) ? uint32_t(6) : uint32_t(1)};

	vkCmdPipelineBarrier(
		commandBufferTexture,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);

	// Otherwise use image copy (requires us to manually flip components)
	VkImageCopy imageCopyRegion{};
	imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.srcSubresource.layerCount = 1;
	imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopyRegion.dstSubresource.layerCount = 1;
	imageCopyRegion.extent.width              = texture->getWidth();
	imageCopyRegion.extent.height             = texture->getHeight();
	imageCopyRegion.extent.depth              = 1;

	// Issue the copy command
	vkCmdCopyImage(
		commandBufferTexture,
		texture->getImage(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		debugSummedAreaResult->getImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageCopyRegion);

	// Transition texture image back to previous layout (VK_IMAGE_LAYOUT_GENERAL)
	imageMemoryBarrier.srcAccessMask    = VK_ACCESS_MEMORY_READ_BIT;
	imageMemoryBarrier.dstAccessMask    = VK_ACCESS_TRANSFER_READ_BIT;
	imageMemoryBarrier.oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.newLayout        = texture->getImageLayout(); // VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.image            = texture->getImage();
	imageMemoryBarrier.subresourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT,
																  0,
																  1,
																  0,
																  (texture->getImageViewType() == VK_IMAGE_VIEW_TYPE_CUBE) ? uint32_t(6) : uint32_t(1)};

	vkCmdPipelineBarrier(
		commandBufferTexture,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&imageMemoryBarrier);

	coreM->endCommandBuffer(commandBufferTexture);
	coreM->submitCommandBuffer(coreM->getLogicalDeviceGraphicsQueue(), &commandBufferTexture);

	VkImageSubresource subResource{};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkSubresourceLayout subResourceLayout;

	vkGetImageSubresourceLayout(coreM->getLogicalDevice(), debugSummedAreaResult->getImage(), &subResource, &subResourceLayout);

	// Map image memory and access it
	const char* data;
	vkMapMemory(coreM->getLogicalDevice(), debugSummedAreaResult->getMem(), 0, VK_WHOLE_SIZE, 0, (void**)&data);
	data += subResourceLayout.offset;

	uint width  = debugSummedAreaResult->getWidth();
	uint height = debugSummedAreaResult->getHeight();
	
	switch (format)
	{
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		{
			vectorResult0.resize(width);
			forI(width)
			{
				vectorResult0[i].resize(height);
			}

			float* value = (float*)data;
			for (uint y = 0; y < height; ++y)
			{
				for (uint x = 0; x < width; ++x)
				{
					vectorResult0[x][y] = vec4(value[0], value[1], value[2], value[3]);
					value += 4;
				}
			}
			break;
		}
		case VK_FORMAT_R32_SFLOAT:
		{
			vectorResult1.resize(width);
			forI(width)
			{
				vectorResult1[i].resize(height);
			}

			float* value = (float*)data;
			for (uint y = 0; y < height; ++y)
			{
				for (uint x = 0; x < width; ++x)
				{
					vectorResult1[x][y] = value[0];
					value++;
				}
			}
			break;
		}
		case VK_FORMAT_R16_SFLOAT:
		{
			vectorResult1.resize(width);
			forI(width)
			{
				vectorResult1[i].resize(height);
			}

			for (uint y = 0; y < height; ++y)
			{
				for (uint x = 0; x < width; ++x)
				{
					glm::detail::hdata temp = (*(glm::detail::hdata*)data);
					data += 2;
				}
			}
			break;
		}
	}

	vkUnmapMemory(coreM->getLogicalDevice(), debugSummedAreaResult->getMem());
	textureM->removeElement(move(string(debugSummedAreaResult->getName())));
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint VulkanStructInitializer::computeNumElementPerThread(uint textureSize, float numElementStep, uint minNumberElement) // = 8, numElementStep= 4.0
{
	// Find next multiple of numElementStep, which will be a multiple of the size of the texture
	// (the texture is assumed to be square and with size a multiple of 32)
	const VkPhysicalDeviceLimits& physicalDeviceLimits = coreM->getPhysicalDeviceProperties().limits;
	float maxComputeWorkGroupInvocations               = float(physicalDeviceLimits.maxComputeWorkGroupInvocations);
	float numElementPerThread                          = float(textureSize * textureSize) / maxComputeWorkGroupInvocations;
	float integerPart;
	float fractional                                   = glm::modf(numElementPerThread / numElementStep, integerPart);
	uint numElementPerThreadFinal                      = uint((integerPart + 1.0f) * numElementStep);

	return glm::max(minNumberElement, numElementPerThreadFinal); // Establish a minimum number of elements per thread
}

/////////////////////////////////////////////////////////////////////////////////////////////
