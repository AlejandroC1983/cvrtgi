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

#ifndef _VULKANSTRUCTINITIALIZER_H_
#define _VULKANSTRUCTINITIALIZER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/core/coremanager.h"
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"

// CLASS FORWARDING
class Buffer;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class VulkanStructInitializer
{
public:
	/** To build a quaternion which rotates fAngleStep rads around an A axis, see 
	* Mathematics for 3D Game Programming & Computer Graphics (Charles River Media, 2002) chapter 3.2
	* @param renderPass      [in] render pass handle
	* @param framebuffer     [in] framebuffer handle
	* @param renderArea      [in] render pass render area
	* @param arrayClearValue [in] vector with the clear values to use
	* @return nothing */
	static VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass          renderPass,
													 VkFramebuffer         framebuffer,
													 VkRect2D              renderArea,
													 vector<VkClearValue>& arrayClearValue);

	// TODO: change the name of the class to a more representative one
	/** Copies the information in the source image to the destination image
	* @param source        [in] source image
	* @param destination   [in] destination image
	* @param commandBuffer [in] command buffer to record the change into
	* @return nothing */
	static void insertCopyImageCommand(Texture* source, Texture* destination, VkCommandBuffer* commandBuffer);

	/** Inserts a memory barrier command, to change image access /layout
	* @param texture                [in] source image
	* @param srcStageMask           [in] source stage mask
	* @param dstStageMask           [in] destination stage mask
	* @param oldLayout              [in] old image layout
	* @param newLayout              [in] new image layout
	* @param sourceAccessFlags      [in] source image flags
	* @param destinationAccessFlags [in] destination image flags
	* @param aspectMask             [in] aspect mask
	* @param baseArrayLayer         [in] base array layer
	* @param commandBuffer          [in] command buffer to record the change into
	* @return nothing */
	static void insertImageMemoryBarrierCommand(Texture*             texture,
												VkPipelineStageFlags srcStageMask,
												VkPipelineStageFlags dstStageMask,
												VkImageLayout        oldLayout,
												VkImageLayout        newLayout,
												VkAccessFlags        sourceAccessFlags,
												VkAccessFlags        destinationAccessFlags,
												VkImageAspectFlags   aspectMask,
												uint32_t             baseArrayLayer,
												VkCommandBuffer*     commandBuffer);

	/** Changes the access flags of the image given as parameter
	* @param texture                [in] source image
	* @param oldLayout              [in] old image layout
	* @param newLayout              [in] new image layout
	* @param sourceAccessFlags      [in] source image flags
	* @param destinationAccessFlags [in] destination image flags
	* @param commandBuffer          [in] command buffer to record the change into
	* @return nothing */
	static void insertImageAccessBarrierCommand(Texture*         texture,
												VkAccessFlags    sourceAccessFlags,
												VkAccessFlags    destinationAccessFlags,
												VkCommandBuffer* commandBuffer);

	/** Adds a memory barrier to the command buffer given as parameter, for all the buffers given as parameter in vectorBuffer
	* @param vectorBuffer  [in] vector with each of the buffers to add a barrier to
	* @param srcAccessMask [in] source access mask
	* @param dstAccessMask [in] destination access mask
	* @param srcStageMask  [in] source stage mask
	* @param dstStageMask  [in] destination stage mask
	* @param commandBuffer [in] command buffer to record the change into
	* @return nothing */
	static void insertBufferMemoryBarrier(vectorBufferPtr&     vectorBuffer,
										  VkAccessFlags        srcAccessMask,
										  VkAccessFlags        dstAccessMask,
										  VkPipelineStageFlags srcStageMask,
										  VkPipelineStageFlags dstStageMask,
										  VkCommandBuffer*     commandBuffer);

	/** Adds a memory barrier to the command buffer given as parameter, for the buffer given as parameter 
	* @param buffer        [in] buffer to add a barrier to
	* @param srcAccessMask [in] source access mask
	* @param dstAccessMask [in] destination access mask
	* @param srcStageMask  [in] source stage mask
	* @param dstStageMask  [in] destination stage mask
	* @param commandBuffer [in] command buffer to record the change into
	* @return nothing */
	static void insertBufferMemoryBarrier(Buffer*              buffer, 
										  VkAccessFlags        srcAccessMask,
										  VkAccessFlags        dstAccessMask,
										  VkPipelineStageFlags srcStageMask,
										  VkPipelineStageFlags dstStageMask,
										  VkCommandBuffer*     commandBuffer);

	/** Return a two dimensional vector with the content of the texture given as parameter
	* NOTE: only float and 4 dimensional texture format covered for now
	* @param texture       [in] texture to download to CPU
	* @param format        [in] texture format
	* @param vectorResult0 [inout] two dimensional vector with the content of the texture given as parameter if format is VK_FORMAT_R32G32B32A32_SFLOAT
	* @param vectorResult1 [inout] two dimensional vector with the content of the texture given as parameter if format is VK_FORMAT_R32_SFLOAT
	* @return nothing */
	// NOTE: regarding format, for now, only VK_FORMAT_R32G32B32A32_SFLOAT and VK_FORMAT_R32_SFLOAT are supported
	static void getTextureData(Texture* texture, VkFormat format, vectorVectorVec4& vectorResult0, vectorVectorFloat& vectorResult1);

	/** Obtain the maximum number of threads that can be dispatched in a single workgroup to cover the whole amount of
	* elements in a square texture given by textureSize. The result will be rounded to have a multiple of numElementStep
	* @param textureSize     [in] size of the square texture to compute the number of element per compute thread
	* @param numElementStep  [in] a multiple of this value will be added, to have rounded values
	* @param minNumberThread [in] minimum number of elements to consider (in case the computed number is smaller than this one
	* @return the number of elements per compute thread neccessary to traverse a square texture of size given by textureSize */
	static uint computeNumElementPerThread(uint textureSize, float numElementStep, uint minNumberElement); // = 8, numElementStep= 4.0
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _VULKANSTRUCTINITIALIZER_H_
