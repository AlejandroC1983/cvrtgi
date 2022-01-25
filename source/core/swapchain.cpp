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
#include "../../include/core/gpupipeline.h"
#include "../../include/core/coremanager.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/framebuffer/framebuffermanager.h"
#include "../../include/core/logicaldevice.h"
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/core/coreenum.h"

// NAMESPACE
using namespace coreenum;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

SwapChain::SwapChain():
	  m_fpQueuePresentKHR(nullptr)
	, m_fpAcquireNextImageKHR(nullptr)
	, m_fpCreateSwapchainKHR(nullptr)
	, m_fpDestroySwapchainKHR(nullptr)
	, m_fpGetSwapchainImagesKHR(nullptr)
	, m_swapChain(VK_NULL_HANDLE)
	, m_desiredNumberOfSwapChainImages(UINT32_MAX)
	, m_swapChainExtent({ windowWidth, windowHeight })
	, m_swapchainPresentMode(VK_PRESENT_MODE_FIFO_KHR)
	, m_width(windowWidth)
	, m_height(windowHeight)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

VkResult SwapChain::createSwapChainExtensions()
{
	// Dependency on createPresentationWindow()
	VkDevice device = coreM->getLogicalDevice();

	// Get Device based swap chain extension function pointer
	DEVICE_FUNC_PTR(device, CreateSwapchainKHR);
	DEVICE_FUNC_PTR(device, DestroySwapchainKHR);
	DEVICE_FUNC_PTR(device, GetSwapchainImagesKHR);
	DEVICE_FUNC_PTR(device, AcquireNextImageKHR);
	DEVICE_FUNC_PTR(device, QueuePresentKHR);

	return VK_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SwapChain::createSwapChain()
{
	// The real size of textures is the size of the swapchain surface made, which can be
	// different from the size of the built window in some cases
	m_swapChainExtent = coreM->setSwapChainExtent();
	m_width           = glm::min(m_width,  m_swapChainExtent.width);
	m_height          = glm::min(m_height, m_swapChainExtent.height);

	cout << "INFO: Final swapchain resolution (" << m_width << ", " << m_height << ")" << endl;

	// // After the previous method, set the new extent, gather necessary information for present mode.
	coreM->managePresentMode(m_swapchainPresentMode, m_desiredNumberOfSwapChainImages);
	createSwapChainColorImages(); // Create the swap chain images
	createColorImageView(); // Get the create color image drawing surfaces
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SwapChain::createSwapChainColorImages()
{
	VkResult  result;
	VkSwapchainKHR oldSwapchain = m_swapChain;

	VkSwapchainCreateInfoKHR swapChainInfo = {};
	swapChainInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.pNext					= NULL;
	swapChainInfo.surface				= coreM->getSurface();
	swapChainInfo.minImageCount			= m_desiredNumberOfSwapChainImages;
	swapChainInfo.imageFormat			= coreM->getSurfaceFormat();
	swapChainInfo.imageExtent.width		= m_width;
	swapChainInfo.imageExtent.height	= m_height;
	swapChainInfo.preTransform			= coreM->getPreTransform();
	swapChainInfo.compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.imageArrayLayers		= 1;
	swapChainInfo.presentMode			= m_swapchainPresentMode;
	swapChainInfo.oldSwapchain			= oldSwapchain;
	swapChainInfo.clipped				= true;
	swapChainInfo.imageColorSpace		= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapChainInfo.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapChainInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.queueFamilyIndexCount = 0;
	swapChainInfo.pQueueFamilyIndices	= NULL;

	result = m_fpCreateSwapchainKHR(coreM->getLogicalDevice(), &swapChainInfo, NULL, &m_swapChain);
	assert(result == VK_SUCCESS);

	uint32_t swapchainImageCount;            //!< Number of buffer image used for swap chain

	// Create the swapchain object
	result = m_fpGetSwapchainImagesKHR(coreM->getLogicalDevice(), m_swapChain, &swapchainImageCount, NULL);
	assert(result == VK_SUCCESS);

	m_arraySwapchainImages.clear();
	// Get the number of images the swapchain has
	m_arraySwapchainImages.resize(swapchainImageCount);
	assert(m_arraySwapchainImages.size() >= 1);

	// Retrieve the swapchain image surfaces 
	result = m_fpGetSwapchainImagesKHR(coreM->getLogicalDevice(), m_swapChain, &swapchainImageCount, &m_arraySwapchainImages[0]);
	assert(result == VK_SUCCESS);

	if (oldSwapchain != VK_NULL_HANDLE)
	{
		m_fpDestroySwapchainKHR(coreM->getLogicalDevice(), oldSwapchain, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SwapChain::createColorImageView()
{
	m_arraySwapChainImageView.clear();
	m_arraySwapChainImageView.resize(m_arraySwapchainImages.size());
	for (uint32_t i = 0; i < m_arraySwapchainImages.size(); i++)
	{
		// Since the swapchain is not owned by us we cannot set the image layout upon setting the implementation may give
		// error, the images layout were create by the WSI implementation not by us. 
		m_arraySwapChainImageView[i] = textureM->buildImageView(VK_IMAGE_ASPECT_COLOR_BIT, m_arraySwapchainImages[i], { VK_COMPONENT_SWIZZLE_IDENTITY }, 1, coreM->getSurfaceFormat(), VK_IMAGE_VIEW_TYPE_2D);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SwapChain::destroySwapChain()
{	
	if (!coreM->getIsResizing())
	{
		// This piece code will only executes at application shutdown.
        // During resize the old swapchain image is delete in createSwapChainColorImages()
		m_fpDestroySwapchainKHR(coreM->getLogicalDevice(), m_swapChain, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SwapChain::createFrameBuffer()
{
	vector<string> arrayAttachment;
	arrayAttachment.resize(2);

	Texture* depth = textureM->getElement(move(string("DepthTexture")));

	arrayAttachment[1] = depth->getName();

	string name;
	vector<Texture*> vectorSwapChainColorImage;
	vectorSwapChainColorImage.resize(m_arraySwapchainImages.size());
	forI(m_arraySwapchainImages.size())
	{
		name = "swapchain_" + to_string(i);
		vectorSwapChainColorImage[i] = textureM->buildTextureFromExistingResources(
			move(string(name)),
			m_arraySwapchainImages[i],
			m_arraySwapChainImageView[i],
			m_width,
			m_height,
			coreM->getSurfaceFormat(),
			VkImageViewType::VK_IMAGE_VIEW_TYPE_2D);

		vectorSwapChainColorImage[i]->setIsSwapChainTex(true);
	}

	Framebuffer* fb;
	m_arrayFramebuffers.clear();
	m_arrayFramebuffers.resize(m_arraySwapchainImages.size());
	for (uint32_t i = 0; i < m_arraySwapchainImages.size(); i++)
	{
		arrayAttachment[0]     = vectorSwapChainColorImage[i]->getName();
		name                   = "swapchain_" + to_string(i);
		fb                     = framebufferM->buildFramebuffer(move(name), m_width, m_height, move(string("swapChainRenderPass")), move(vector<string>(arrayAttachment)));
		m_arrayFramebuffers[i] = fb;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SwapChain::createDepthImage()
{
	Texture* depth = textureM->buildTexture(
		move(string("DepthTexture")),
		VK_FORMAT_D24_UNORM_S8_UINT,
		{ uint32_t(m_width), uint32_t(m_height), 1 },
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_VIEW_TYPE_2D,
		0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkResult SwapChain::acquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
	return m_fpAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, pImageIndex);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkResult SwapChain::queuePresent(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
	return m_fpQueuePresentKHR(queue, pPresentInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////////
