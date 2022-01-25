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

#ifndef _SWAPCHAIN_H_
#define _SWAPCHAIN_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/framebuffer/framebuffer.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class SwapChain
{
public:
	/** Default constructor
	* @return nothing */
	SwapChain();

	/** Make swap chain extent and present modes, make color images and their views
	* @return nothing */
	void createSwapChain();

	/** Make swap chain extent and present modes, make color images and their views
	* @return nothing */
	void destroySwapChain();

	/** For each swapchain image, builds a new framebuffer with a swapchain image attached to it (anf the depth image)
	* and adds it to m_arrayFramebuffers
	* @return nothing */
	void createFrameBuffer();

	/** Builds the depth image that will be used for the swapchain rasterization
	* @return nothing */
	void createDepthImage();

	/** Takes the function pointers given by fpQueuePresentKHR, fpAcquireNextImageKHR, fpCreateSwapchainKHR,
	* fpDestroySwapchainKHR and fpGetSwapchainImagesKHR
	* @return an VkResult with information about the success of the swap chain building */
	VkResult createSwapChainExtensions();

	/** Builds an image view for each image present in m_arraySwapchainImages, setting the corresponding struct member value
	* @return nothing */
	void createColorImageView();

	/** Builds the swap chain color images and retrieves them, putting a handler in the corresponding variable of m_arraySwapchainImages
	* @return nothing */
	void createSwapChainColorImages();

	/** Calls the function pointer which takes the next swapchain image to raster
	* @param device      [in]    device
	* @param swapchain   [in]    swapchain
	* @param timeout     [in]    timeout
	* @param semaphore   [in]    semaphore
	* @param fence       [in]    fence
	* @param pImageIndex [inout] pImageIndex
	* @return an VkResult with information about the success of the swap chain building */
	VkResult acquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

	/** Calls the function pointer which presents the queue of commands
	* @param queue        [in] queue
	* @param pPresentInfo [in] presentation information
	* @return an VkResult with information about the success of the swap chain building */
	VkResult queuePresent(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
	
	GET(VkSwapchainKHR, m_swapChain, SwapChain)
	GET(vector<VkImage>, m_arraySwapchainImages, ArraySwapchainImages)
	GET(vector<Framebuffer*>, m_arrayFramebuffers, ArrayFramebuffers)
	GET_SET(VkExtent2D, m_swapChainExtent, SwapChainExtent)
	GETCOPY(uint32_t, m_desiredNumberOfSwapChainImages, DesiredNumberOfSwapChainImages)

protected:
	PFN_vkQueuePresentKHR	    m_fpQueuePresentKHR;              //!< Function pointer to several swapchain functionalities
	PFN_vkAcquireNextImageKHR   m_fpAcquireNextImageKHR;          //!< Function pointer to several swapchain functionalities
	PFN_vkCreateSwapchainKHR	m_fpCreateSwapchainKHR;           //!< Function pointer to several swapchain functionalities
	PFN_vkDestroySwapchainKHR   m_fpDestroySwapchainKHR;          //!< Function pointer to several swapchain functionalities
	PFN_vkGetSwapchainImagesKHR m_fpGetSwapchainImagesKHR;        //!< Function pointer to several swapchain functionalities
	vector<Framebuffer*>        m_arrayFramebuffers;              //!< One framebuffer corresponding to each swap chain
	vector<VkImage>             m_arraySwapchainImages;           //!< The retrived drawing color swap chain images
	VkSwapchainKHR              m_swapChain;                      //!< Swap chain object
	vector<VkImageView>         m_arraySwapChainImageView;        //!< List of color swap chain image views
	uint32_t                    m_desiredNumberOfSwapChainImages; //!< Number of color images supported by the implementation
	VkExtent2D                  m_swapChainExtent;                //!< Size of the swap chain color images
	VkPresentModeKHR            m_swapchainPresentMode;           //!< Stores present mode bitwise flag for the creation of swap chain
	uint32_t                    m_width;                          //!< Swap chain default width
	uint32_t                    m_height;                         //!< Swap chain default height
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SWAPCHAIN_H_
