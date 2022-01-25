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

#ifndef _COREMANAGER_H_
#define _COREMANAGER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/singleton.h"
#include "../../include/core/gpupipeline.h"
#include "../../include/core/surface.h"
#include "../../include/texture/texture.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/util/loopmacrodefines.h"
#include "../../include/core/logicaldevice.h"
#include "../../include/core/physicaldevice.h"
#include "../../include/core/instance.h"
#include "../../include/core/swapchain.h"
#include "../../include/core/input.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES
#define coreM s_pCoreManager->instance()

/////////////////////////////////////////////////////////////////////////////////////////////

class CoreManager: public Singleton<CoreManager>
{
public:
	/** Constructor
	* @return nothing */
	CoreManager();

	/** Default destructor
	* @return nothing */
	~CoreManager();

	/** Initialize and allocate resources
	* @return nothing */
	void initialize();

	/** Prepare resources
	* @return nothing */
	void prepare();

	/** Resize window
	* @return nothing */
	void resize();

	/** Release resources
	* @return nothing */
	void deInitialize();

	/** Create swapchain color image and depth image
	* @return nothing */
	void buildSwapChainAndDepthImage();

	/** Builds the render pass, m_renderPass
	* @return nothing */
	void createRenderPass(bool includeDepth, bool clear = true);	// Render Pass creation

	/** Builds the command pools, m_graphicsCommandPool and m_computeCommandPool
	* @return nothing */
	void createCommandPools();

	/** Sets the extent of the swap chain
	* @return nothing */
	void setSwapChainExtent(uint32_t width, uint32_t height);

	/** Render primitives
	* @return nothing */
	void render();

	/** Called after render method
	* @return nothing */
	void postRender();

	/** Destroys m_renderPass
	* @return nothing */
	void destroyRenderpass();

	/** Getter of Surface::m_surface
	* @return Surface::m_surface of type VkFormat */
	VkFormat getSurfaceFormat();

	/** Getter of PhysicalDevice::m_queueFamilyProps
	* @return PhysicalDevice::m_queueFamilyProps of type vector<VkQueueFamilyProperties> */
	const vector<VkQueueFamilyProperties>& getQueueFamilyProps();

	/** Getter of Swapchain::m_arraySwapchainImages
	* @return Swapchain::m_arraySwapchainImages of type vector<VkImage> */
	const vector<VkImage>& getArraySwapchainImages();

	/** Getter of Swapchain::m_arrayFramebuffers
	* @return Swapchain::m_arrayFramebuffers of type vector<Framebuffer*> */
	const vector<Framebuffer*>& getArrayFramebuffers();

	/** Sets the extent of the swap chain images to be build
	* @return nothing */
	VkExtent2D setSwapChainExtent();

	/** Returns the result of calling Surface::managePresentMode
	* @return the result of calling Surface::managePresentMode */
	void managePresentMode(VkPresentModeKHR& presentMode, uint32_t& numberOfSwapChainImages);

	/** Returns the result of calling PhysicalDevice::memoryTypeFromProperties
	* @return the result of calling PhysicalDevice::memoryTypeFromProperties */
	bool memoryTypeFromProperties(uint32_t typeBits, VkFlags requirementsMask, const VkMemoryType* memoryTypes, uint32_t& typeIndex);

	/** Getter of PhysicalDevice::m_physicalDeviceMemoryProperties
	* @return PhysicalDevice::m_physicalDeviceMemoryProperties of type VkPhysicalDeviceMemoryProperties */
	const VkPhysicalDeviceMemoryProperties& getPhysicalDeviceMemoryProperties();

	/** Getter of Surface::m_width
	* @return Surface::m_width of type uint32_t */
	uint32_t getWidth() const;

	/** Getter of Surface::m_height
	* @return Surface::m_height of type uint32_t */
	uint32_t getHeight() const;

	/** Getter of Surface::m_preTransform
	* @return Surface::m_preTransform of type VkSurfaceTransformFlagBitsKHR */
	VkSurfaceTransformFlagBitsKHR getPreTransform() const;

	/** Getter of Instance::m_instance
	* @return copy of Instance::m_instance of type VkInstance */
	const VkInstance getInstance() const;

	/** Getter of Surface::m_surface
	* @return copy of Surface::m_surface of type VkSurfaceKHR */
	const VkSurfaceKHR getSurface() const;

	/** Getter of PhysicalDevice::m_physicalDevice
	* @return copy of PhysicalDevice::m_physicalDevice of type VkPhysicalDevice */
	const VkPhysicalDevice getPhysicalDevice() const;

	/** Getter of LogicalDevice::m_logicalDevice
	* @return copy of LogicalDevice::m_logicalDevice of type VkDevice */
	const VkDevice& getLogicalDevice() const;

	/** Getter of LogicalDevice::m_logicalDeviceGraphicsQueue
	* @return copy of LogicalDevice::m_logicalDeviceGraphicsQueue of type VkQueue */
	const VkQueue getLogicalDeviceGraphicsQueue() const;

	/** Getter of LogicalDevice::m_logicalDeviceComputeQueue
	* @return copy of LogicalDevice::m_logicalDeviceComputeQueue of type VkQueue */
	const VkQueue getLogicalDeviceComputeQueue() const;

	/** Getter of PhysicalDevice::m_physicalDeviceProperties
	* @return reference to PhysicalDevice::m_physicalDeviceProperties of type VkPhysicalDeviceProperties */
	const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const;

	/** Destroys m_graphicsCommandPool and m_computeCommandPool
	* @return nothing */
	void destroyCommandPools();

	/** Getter of Surface::m_window
	* @return windows platform handle */
	const HWND getWindowPlatformHandle() const;

	/** Returns the next available index for unique identifying command buffers
	* @return next available index for unique identifying command buffers */
	uint getNextCommandBufferIndex();

	/** Initialize some hardware limits information that might be of common use
	* @return nothing */
	void initHardwareLimitValues();

	/** Utility method to show some hardware limits information
	* @return nothing */
	void showHardwareLimits();

	/** Returns the values of SwapChain::m_width and SwapChain::m_height as an ivec2
	* @return nothing */
	ivec2 getSwapChainDimensions();

	static void allocCommandBuffer(const VkDevice* device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo = NULL);
	static void beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo = NULL);
	static void endCommandBuffer(VkCommandBuffer cmdBuf);
	static void submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* cmdBufList, const VkSubmitInfo* submitInfo = NULL, const VkFence& fence = VK_NULL_HANDLE);

	GETCOPY(bool, m_isPrepared, IsPrepared)
	GETCOPY(bool, m_isResizing, IsResizing)
	REF(VkRenderPass, m_renderPass, RenderPass)
	GETCOPY(VkCommandPool, m_graphicsCommandPool, GraphicsCommandPool)
	GETCOPY(VkCommandPool, m_computeCommandPool, ComputeCommandPool)
	GETCOPY_SET(bool, m_reachedFirstRaster, ReachedFirstRaster)
	GET(VkQueryPool, m_graphicsQueueQueryPool, GraphicsQueueQueryPool)
	GET(VkQueryPool, m_computeQueueQueryPool, ComputeQueueQueryPool)
	GETCOPY(uint, m_maxComputeWorkGroupInvocations, MaxComputeWorkGroupInvocations)
	GETCOPY(uint, m_maxComputeSharedMemorySize, MaxComputeSharedMemorySize)
	GETCOPY(uvec3, m_maxComputeWorkGroupCount, MaxComputeWorkGroupCount)
	GETCOPY(uvec3, m_maxComputeWorkGroupSize, MaxComputeWorkGroupSize)
	GETCOPY(uvec3, m_minComputeWorkGroupSize, MinComputeWorkGroupSize)
	GETCOPY(uint, m_maxPushConstantsSize, MaxPushConstantsSize)
	GETCOPY(uint, m_maxImageArrayLayers, MaxImageArrayLayers)
	GETCOPY(uint, m_maxImageDimension1D, MaxImageDimension1D)
	GETCOPY(uint, m_maxImageDimension2D, MaxImageDimension2D)
	GETCOPY(uint, m_maxImageDimension3D, MaxImageDimension3D)
	GETCOPY(uint, m_maxImageDimensionCube, MaxImageDimensionCube)
	GETCOPY_SET(bool, m_endApplicationMessage, EndApplicationMessage)

protected:
	/** Build m_graphicsQueueQueryPool and m_computeQueueQueryPool query pools
	* @return nothing */
	void initializeQueryPools();

	/** Destroys m_graphicsQueueQueryPool and m_computeQueueQueryPool query pools
	* @return nothing */
	void destroyQueryPool();

	VkCommandPool   m_graphicsCommandPool;            //!< Graphics command pool
	VkCommandPool   m_computeCommandPool;             //!< Compute command pool
	VkRenderPass    m_renderPass;                     //!< Render pass created object
	Instance        m_instance;                       //!< Instance wrapper class
	Surface         m_surface;                        //!< Surface wrapper class
	LogicalDevice   m_logicalDevice;                  //!< Logical device wrapper class
	PhysicalDevice  m_physicalDevice;                 //!< Physical device wrapper class
	SwapChain       m_swapChain;                      //!< Swap chain wrapper class
	uint32_t        m_currentColorBuffer;             //!< Current drawing surface index in use
	VkSemaphore     m_presentCompleteSemaphore;       //!< Semaphore for sync purposes
	VkSemaphore     m_drawingCompleteSemaphore;       //!< Semaphore for sync purposes
	bool            m_debugFlag;                      //!< If true, debug callback is added
	bool            m_isPrepared;                     //!< If false, a resize is being performed, or the renderer object is preparing the next frame
	bool            m_isResizing;                     //!< If true, a resize is being performed
	bool            m_reachedFirstRaster;             //!< True when the first commands for recording rasterization for the first time are reached
	static int      m_vectorRecordIndex;              //!< Index to record commands to m_arrayCommandDraw
	int             m_lastSubmittedCommand;           //!< Index of the last submitted command
	static uint     m_commandBufferIndex;             //!< Unique identifier to notify recording techniques
	VkQueryPool     m_graphicsQueueQueryPool;         //!< Query pool for performance measurements for graphics queue
	VkQueryPool     m_computeQueueQueryPool;          //!< Query pool for performance measurements for compute queue
	bool            m_queryPoolsInitialized;          //!< True if query pools have been initialized
	uint            m_maxComputeWorkGroupInvocations; //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_maxComputeSharedMemorySize;     //!< Vulkan hardware limit that might be needed by more than one technique
	uvec3           m_maxComputeWorkGroupCount;       //!< Vulkan hardware limit that might be needed by more than one technique
	uvec3           m_maxComputeWorkGroupSize;        //!< Vulkan hardware limit that might be needed by more than one technique
	uvec3           m_minComputeWorkGroupSize;        //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_maxPushConstantsSize;           //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_maxImageArrayLayers;            //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_maxImageDimension1D;            //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_maxImageDimension2D;            //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_maxImageDimension3D;            //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_maxImageDimensionCube;          //!< Vulkan hardware limit that might be needed by more than one technique
	uint            m_numAcquiredImages;              //!< Number of acquired images through PFN_vkAcquireNextImageKHR
	uint            m_numPresentedUmages;             //!< Number of presented images through PFN_vkQueuePresentKHR
	uint            m_maxImageNumberAdquired;         //!< Maximum number of images that can be aquired and have not been presented yet
	bool            m_endApplicationMessage;          //!< Flag to know when a message to end application is received
	VkFence         m_fence;                          //!< Fence used for command buffer submitting
	bool            m_firstFrameFinished;             //!< To know when the first frame has been finished, updated in postRender method
};

static CoreManager* s_pCoreManager;

/////////////////////////////////////////////////////////////////////////////////////////////

// INLINE METHODS

/////////////////////////////////////////////////////////////////////////////////////////////

inline VkFormat CoreManager::getSurfaceFormat()
{
	return m_surface.getFormat();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const vector<VkQueueFamilyProperties>& CoreManager::getQueueFamilyProps()
{
	return m_physicalDevice.getQueueFamilyProps();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const vector<VkImage>& CoreManager::getArraySwapchainImages()
{
	return m_swapChain.getArraySwapchainImages();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const vector<Framebuffer*>& CoreManager::getArrayFramebuffers()
{
	return m_swapChain.getArrayFramebuffers();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline VkExtent2D CoreManager::setSwapChainExtent()
{
	VkExtent2D extent = m_surface.setSwapChainExtent();

	return extent;
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline void CoreManager::managePresentMode(VkPresentModeKHR& presentMode, uint32_t& numberOfSwapChainImages)
{
	m_surface.managePresentMode(presentMode, numberOfSwapChainImages);
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline bool CoreManager::memoryTypeFromProperties(uint32_t typeBits, VkFlags requirementsMask, const VkMemoryType* memoryTypes, uint32_t& typeIndex)
{
	return m_physicalDevice.memoryTypeFromProperties(typeBits, requirementsMask, memoryTypes, typeIndex);
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkPhysicalDeviceMemoryProperties& CoreManager::getPhysicalDeviceMemoryProperties()
{
	return m_physicalDevice.getPhysicalDeviceMemoryProperties();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t CoreManager::getWidth() const
{
	return m_surface.getWidth();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t CoreManager::getHeight() const
{
	return m_surface.getHeight();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline VkSurfaceTransformFlagBitsKHR CoreManager::getPreTransform() const
{
	return m_surface.getPreTransform();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkInstance CoreManager::getInstance() const
{
	return m_instance.getInstance();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkSurfaceKHR CoreManager::getSurface() const
{
	return m_surface.getSurface();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkPhysicalDevice CoreManager::getPhysicalDevice() const
{
	return m_physicalDevice.getPhysicalDevice();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkDevice& CoreManager::getLogicalDevice() const
{
	return m_logicalDevice.getLogicalDevice();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkQueue CoreManager::getLogicalDeviceGraphicsQueue() const
{
	return m_logicalDevice.getLogicalDeviceGraphicsQueue();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkQueue CoreManager::getLogicalDeviceComputeQueue() const
{
	return m_logicalDevice.getLogicalDeviceComputeQueue();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const VkPhysicalDeviceProperties& CoreManager::getPhysicalDeviceProperties() const
{
	return m_physicalDevice.getPhysicalDeviceProperties();
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline const HWND CoreManager::getWindowPlatformHandle() const
{
	return m_surface.getWindow();
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _COREMANAGER_H_
