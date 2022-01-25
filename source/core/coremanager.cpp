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
#include "../../include/core/coremanager.h"
#include "../../include/core/logicaldevice.h"
#include "../../include/core/physicaldevice.h"
#include "../../include/core/instance.h"
#include "../../include/core/surface.h"
#include "../../include/texture/texturemanager.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/framebuffer/framebuffermanager.h"
#include "../../include/scene/scene.h"
#include "../../include/pipeline/pipeline.h"
#include "../../include/uniformbuffer/uniformbuffermanager.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/material/materialmanager.h"
#include "../../include/rastertechnique/rastertechniquemanager.h"
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/camera/camera.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/core/coreenum.h"
#include "../../include/rastertechnique/rastertechnique.h"

// NAMESPACE
using namespace coreenum;
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
int CoreManager::m_vectorRecordIndex   = -1;
uint CoreManager::m_commandBufferIndex = 0;

static vector<const char *> instanceExtensionNames =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
};

static vector<const char *> deviceExtensionNames =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	//VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
};

static vector<const char *> layerNames =
{
	"VK_LAYER_LUNARG_standard_validation"
};

//VK_LAYER_KHRONOS_validation

/////////////////////////////////////////////////////////////////////////////////////////////

CoreManager::CoreManager() :
	  m_graphicsCommandPool(VK_NULL_HANDLE)
	, m_computeCommandPool(VK_NULL_HANDLE)
	, m_renderPass(VK_NULL_HANDLE)
	, m_currentColorBuffer(UINT32_MAX)
	, m_presentCompleteSemaphore(VK_NULL_HANDLE)
	, m_drawingCompleteSemaphore(VK_NULL_HANDLE)
	, m_debugFlag(true)
	, m_isPrepared(false)
	, m_isResizing(false)
	, m_reachedFirstRaster(false)
	, m_lastSubmittedCommand(0)
	, m_graphicsQueueQueryPool(VK_NULL_HANDLE)
	, m_computeQueueQueryPool(VK_NULL_HANDLE)
	, m_queryPoolsInitialized(false)
	, m_maxComputeWorkGroupInvocations(0)
	, m_maxComputeSharedMemorySize(0)
	, m_maxComputeWorkGroupCount(0)
	, m_maxComputeWorkGroupSize(0)
	, m_maxPushConstantsSize(0)
	, m_maxImageArrayLayers(0)
	, m_maxImageDimension1D(0)
	, m_maxImageDimension2D(0)
	, m_maxImageDimension3D(0)
	, m_maxImageDimensionCube(0)
	, m_numAcquiredImages(0)
	, m_numPresentedUmages(0)
	, m_maxImageNumberAdquired(0)
	, m_endApplicationMessage(false)
	, m_firstFrameFinished(false)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

CoreManager::~CoreManager()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::initialize()
{
	// At application start up, enumerate instance layers
	m_instance.getInstanceLayerProperties(m_physicalDevice.refLayerPropertyList());

	// Initialize singletons
	s_pTextureManager         = Singleton<TextureManager>::init();
	s_pBufferManager          = Singleton<BufferManager>::init();
	s_pShaderManager          = Singleton<ShaderManager>::init();
	s_pFramebufferManager     = Singleton<FramebufferManager>::init();
	s_pInputSingleton         = Singleton<Input>::init();
	s_pSceneSingleton         = Singleton<Scene>::init();
	s_pUniformBufferManager   = Singleton<UniformBufferManager>::init();
	s_pMaterialManager        = Singleton<MaterialManager>::init();
	s_pRasterTechniqueManager = Singleton<RasterTechniqueManager>::init();
	s_pRenderPassManager      = Singleton<RenderPassManager>::init();
	s_pCameraManager          = Singleton<CameraManager>::init();
	s_pGPUPipeline            = Singleton<GPUPipeline>::init();

	// Assign slots for already existing singletons who manage resources
	s_pTextureManager->assignSlots();
	s_pBufferManager->assignSlots();
	s_pShaderManager->assignSlots();
	s_pFramebufferManager->assignSlots();
	s_pUniformBufferManager->assignSlots();
	s_pMaterialManager->assignSlots();
	s_pRasterTechniqueManager->assignSlots();
	s_pRenderPassManager->assignSlots();
	s_pCameraManager->assignSlots();

	char title[] = "Demo";

	m_instance.areLayersSupported(layerNames, m_physicalDevice.refLayerPropertyList()); // Check if the supplied layer are support or not
	m_instance.createInstance(layerNames, instanceExtensionNames, title); // Create the Vulkan instance with specified layer and extension names.

	// Create the debugging report if debugging is enabled
	if (m_debugFlag)
	{
		m_instance.createDebugReportCallback();
	}

	// Get the list of physical devices on the system
	vector<VkPhysicalDevice> arrayGPU = m_physicalDevice.enumeratePhysicalDevices(m_instance.getInstance());

	// This example use only one device which is available first.
	if (arrayGPU.size() > 0)
	{
		// The user define Vulkan Device object this will manage the, Physical and logical device and their queue and properties
		m_physicalDevice.setPhysicalDevice(arrayGPU[0]);
		m_physicalDevice.getDeviceExtensionProperties(m_physicalDevice.refLayerPropertyList()); // Print the devices available layer and their extension 
		m_physicalDevice.getPhysicalDeviceProperties(); // Get the physical device or GPU properties, get the memory properties from the physical device or GPU.
		m_physicalDevice.retrievePhysicalDeviceMemoryProperties();
		m_physicalDevice.getPhysicalDeviceQueuesAndProperties(); // Query the availabe queues on the physical device and their properties.
		m_physicalDevice.getGraphicsQueueHandle(); // Retrive the queue which support graphics pipeline.
		m_physicalDevice.getComputeQueueHandle();
		m_physicalDevice.getPhysicalDeviceFeatures();
		m_logicalDevice.createDevice(deviceExtensionNames, m_physicalDevice.getGraphicsQueueIndex(), m_physicalDevice.getComputeQueueIndex(),  m_physicalDevice.getPhysicalDevice()); // Create Logical Device, ensure that this device is connecte to graphics queue
	}

	VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo;
	presentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentCompleteSemaphoreCreateInfo.pNext = NULL;
	presentCompleteSemaphoreCreateInfo.flags = 0;

	VkSemaphoreCreateInfo drawingCompleteSemaphoreCreateInfo;
	drawingCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	drawingCompleteSemaphoreCreateInfo.pNext = NULL;
	drawingCompleteSemaphoreCreateInfo.flags = 0;

	vkCreateSemaphore(m_logicalDevice.getLogicalDevice(), &presentCompleteSemaphoreCreateInfo, NULL, &m_presentCompleteSemaphore);
	vkCreateSemaphore(m_logicalDevice.getLogicalDevice(), &drawingCompleteSemaphoreCreateInfo, NULL, &m_drawingCompleteSemaphore);

	// Create an empty window
	m_surface.setWidth(windowWidth);
	m_surface.setHeight(windowHeight);
	m_surface.createPresentationWindow(fullscreen);
	// Initialize swapchain
	m_swapChain.createSwapChainExtensions();
	m_surface.createSurfaceExtensions();
	m_surface.createSurface();
	m_surface.getGraphicsQueueWithPresentationSupport(); // Getting a graphics queue with presentation support
	if (m_surface.getGraphicsQueueWithPresentIndex() == UINT32_MAX)
	{
		cout << "Could not find a graphics and a present queue\nCould not find a graphics and a present queue\n";
		exit(-1);
	}
	m_surface.getSupportedFormats(); // Get the list of formats that are supported

	m_logicalDevice.requestGraphicsQueue(m_surface.getGraphicsQueueWithPresentIndex());
	m_logicalDevice.requestComputeQueue(m_physicalDevice.getComputeQueueIndex());

	createCommandPools();
	buildSwapChainAndDepthImage();
	// Use render pass and create frame buffer
	const bool includeDepth = true;
	// Create the render pass now..
	createRenderPass(includeDepth);
	m_swapChain.createFrameBuffer();

	initHardwareLimitValues();
	showHardwareLimits();

	// According to WSI swapchain
	// "Let n be the total number of images in the swapchain, m be the value of VkSurfaceCapabilitiesKHR::minImageCount,
	// and a be the number of presentable images that the application has currently acquired(i.e.images acquired with
	// vkAcquireNextImageKHR, but not yet presented with vkQueuePresentKHR).vkAcquireNextImageKHR can always succeed
	// if a <= n - m at the time vkAcquireNextImageKHR is called.vkAcquireNextImageKHR should not be called
	// if a > n - m with a timeout of UINT64_MAX; in such a case, vkAcquireNextImageKHR may block indefinitely."
	m_maxImageNumberAdquired = m_swapChain.getDesiredNumberOfSwapChainImages() - m_surface.getSurfaceCapabilities().minImageCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::resize()
{
	// If prepared then only proceed for 
	if (!m_isPrepared)
	{
		return;
	}
	
	m_isResizing = true;

	vkDeviceWaitIdle(m_logicalDevice.getLogicalDevice());
	destroyCommandPools();
	shaderM->destroyResources();

	destroyRenderpass();
	m_swapChain.destroySwapChain();
	m_surface.destroySurface();

	textureM->destroyResources();
	buildSwapChainAndDepthImage();
	m_swapChain.createFrameBuffer();
	gpuPipelineM->init();

	destroyQueryPool();
	m_queryPoolsInitialized = false;

	prepare();

	m_isResizing = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::deInitialize()
{
	materialM->destroyResources();
	gpuPipelineM->destroyResources();

	destroyCommandPools();

	m_swapChain.destroySwapChain();
	m_surface.destroySurface();

	renderPassM->destroyResources();
	textureM->destroyResources();
	bufferM->destroyResources();
	shaderM->destroyResources();
	framebufferM->destroyResources();
	sceneM->shutdown();
	uniformBufferM->destroyResources();
	

	vkDestroySemaphore(coreM->getLogicalDevice(), m_presentCompleteSemaphore, NULL);
	vkDestroySemaphore(coreM->getLogicalDevice(), m_drawingCompleteSemaphore, NULL);

	vkDestroyFence(coreM->getLogicalDevice(), m_fence, nullptr);

	destroyQueryPool();
	m_surface.destroyPresentationWindow();
	m_logicalDevice.destroyDevice();

	m_queryPoolsInitialized = false;

	if (m_debugFlag)
	{
		m_instance.destroyDebugReportCallback();
	}
	m_instance.destroyInstance();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::buildSwapChainAndDepthImage()
{
	// Get the appropriate queue to submit the command into
	m_surface.getSurfaceCapabilitiesAndPresentMode(); // use extensions and get the surface capabilities, present mode
	m_swapChain.createSwapChain(); // Create swapchain and get the color image	
	m_swapChain.createDepthImage(); // Create the depth image
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::setSwapChainExtent(uint32_t width, uint32_t height)
{
	m_swapChain.setSwapChainExtent(VkExtent2D({ width, height }));
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::prepare()
{
	if (!m_queryPoolsInitialized)
	{
		initializeQueryPools();
		m_queryPoolsInitialized = true;
	}

 	if (!m_reachedFirstRaster)
	{
		inputM->setCursorPos(coreM->getWidth() / 2, coreM->getHeight() / 2);
		m_reachedFirstRaster = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::render()
{
	vectorRasterTechniquePtr& vectorTechnique  = gpuPipelineM->refVectorRasterTechnique();
  
 	// Get the index of the next available swapchain image:
 	VkResult result = m_swapChain.acquireNextImageKHR(m_logicalDevice.getLogicalDevice(), m_swapChain.getSwapChain(),
 		UINT64_MAX, m_presentCompleteSemaphore, VK_NULL_HANDLE, &m_currentColorBuffer);
 
 	uint maxIndex                              = uint(vectorTechnique.size());
 	VkQueue computeQueue                       = m_logicalDevice.getLogicalDeviceComputeQueue();
 	VkQueue graphicsQueue                      = m_logicalDevice.getLogicalDeviceGraphicsQueue();
 	VkPipelineStageFlags pipelineStageGraphics = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkPipelineStageFlags pipelineStageCompute  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	int counterSubmit                          = 0;
	VkSemaphore* waitSemaphore                 = nullptr;
	VkSemaphore* lastSemaphore                 = nullptr;
	bool anyComputeCommandBuffer               = false;

	bool isGraphicsQueue;
	uint commandBufferID;
	CommandBufferType commandBufferType;
	VkCommandBuffer* commandBuffer;
	VkPipelineStageFlags* pipelineStageFlags;
	vectorRasterTechniquePtr vectorTechniqueMeasure;

	forI(maxIndex)
	{
 		RasterTechnique* technique = vectorTechnique[i];

		if (!technique->getActive())
		{
			continue;
		}

		technique->preRecordLoop();

		int counterSameTechniqueSubmit = 0;

		while (technique->getExecuteCommand())
		{
			technique->prepare(0.167f);

			technique->updateMaterial();

			if (technique->getNeedsToRecord())
			{
				technique->record(m_currentColorBuffer, commandBufferID, commandBufferType);
			}

			if (technique->getIsLastPipelineTechnique())
			{
				commandBuffer = technique->refVectorCommand()[m_currentColorBuffer];
			}
			else
			{
				commandBuffer = technique->refVectorCommand().back();
			}

			isGraphicsQueue = (technique->getRasterTechniqueType() == RasterTechniqueType::RTT_GRAPHICS);

			if (isGraphicsQueue)
			{
				pipelineStageFlags = &pipelineStageGraphics;
			}
			else
			{
				pipelineStageFlags = &pipelineStageCompute;
			}

			if (counterSubmit == 0)
			{
				waitSemaphore = &m_presentCompleteSemaphore;
			}
			else
			{
				waitSemaphore = lastSemaphore;
			}
 
			if (isGraphicsQueue)
			{
				VkSubmitInfo submitInfo = {};
 				submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
 				submitInfo.pNext                = NULL;
				submitInfo.waitSemaphoreCount   = waitSemaphore == nullptr ? 0 : 1;
				submitInfo.pWaitSemaphores      = waitSemaphore;
 				submitInfo.pWaitDstStageMask    = pipelineStageFlags;
 				submitInfo.commandBufferCount   = (uint32_t)1;
 				submitInfo.pCommandBuffers      = commandBuffer;
				submitInfo.pSignalSemaphores    = (i == maxIndex - 1) ? &m_drawingCompleteSemaphore : nullptr;
				submitInfo.signalSemaphoreCount = submitInfo.pSignalSemaphores == nullptr ? 0 : 1;

				result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, m_fence);
				vkWaitForFences(m_logicalDevice.getLogicalDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
				vkResetFences(m_logicalDevice.getLogicalDevice(), 1, &m_fence);

				technique->addExecutionTime();
			}
			else
			{
				VkSubmitInfo submitInfo = {};
 				submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
 				submitInfo.pNext                = NULL;
				submitInfo.waitSemaphoreCount   = 0;
				submitInfo.pWaitSemaphores      = nullptr;
 				submitInfo.pWaitDstStageMask    = pipelineStageFlags;
 				submitInfo.commandBufferCount   = (uint32_t)1;
 				submitInfo.pCommandBuffers      = commandBuffer;
 				submitInfo.signalSemaphoreCount = 0;
				submitInfo.pSignalSemaphores    = nullptr;

				// NOTE: Wait for host should be an option
				if (technique->getComputeHostSynchronize())
				{
					result = vkQueueSubmit(computeQueue, 1, &submitInfo, m_fence);
					vkWaitForFences(m_logicalDevice.getLogicalDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
					vkResetFences(m_logicalDevice.getLogicalDevice(), 1, &m_fence);
					anyComputeCommandBuffer = true;
				}

				technique->addExecutionTime();				
			}
 			assert(!result);

			technique->postCommandSubmit();
			addIfNoPresent(technique, vectorTechniqueMeasure);

			counterSubmit++;
			counterSameTechniqueSubmit++;
 		}
	}
 
 	VkPresentInfoKHR present = {};
 	present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
 	present.pNext              = NULL;
 	present.swapchainCount     = 1;
 	present.pSwapchains        = &m_swapChain.getSwapChain();
 	present.pImageIndices      = &m_currentColorBuffer;
 	present.pWaitSemaphores    = &m_drawingCompleteSemaphore;
 	present.waitSemaphoreCount = 1;
 	present.pResults           = NULL;
 
 	// Queue the image for presentation,
 	result = m_swapChain.queuePresent(m_logicalDevice.getLogicalDeviceGraphicsQueue(), &present);
 	assert(result == VK_SUCCESS);
 
 	result = vkQueueWaitIdle(graphicsQueue);
 	assert(result == VK_SUCCESS);

	if (anyComputeCommandBuffer)
	{
		result = vkQueueWaitIdle(computeQueue);
		assert(result == VK_SUCCESS);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::postRender()
{
	if (!m_firstFrameFinished)
	{
		bufferM->printWholeBufferInformation();
		m_firstFrameFinished = true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::createCommandPools()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.pNext            = NULL;
	cmdPoolInfo.queueFamilyIndex = m_surface.getGraphicsQueueWithPresentIndex();
	cmdPoolInfo.flags            = 0;

	VkResult res = vkCreateCommandPool(m_logicalDevice.getLogicalDevice(), &cmdPoolInfo, NULL, &m_graphicsCommandPool);
	assert(res == VK_SUCCESS);

	cmdPoolInfo.queueFamilyIndex = m_physicalDevice.getComputeQueueIndex();
	res = vkCreateCommandPool(m_logicalDevice.getLogicalDevice(), &cmdPoolInfo, NULL, &m_computeCommandPool);
	assert(res == VK_SUCCESS);

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = 0;
	vkCreateFence(m_logicalDevice.getLogicalDevice(), &fenceInfo, nullptr, &m_fence);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::createRenderPass(bool isDepthSupported, bool clear)
{
	VkAttachmentReference* depthReference = new VkAttachmentReference({ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
	VkPipelineBindPoint* pipelineBindPoint = new VkPipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);

	vector<VkFormat>* vectorAttachmentFormat = new vector<VkFormat>;
	vectorAttachmentFormat->push_back(getSurfaceFormat());
	vectorAttachmentFormat->push_back(VK_FORMAT_D24_UNORM_S8_UINT);

	vector<VkSampleCountFlagBits>* vectorAttachmentSamplesPerPixel = new vector<VkSampleCountFlagBits>;
	vectorAttachmentSamplesPerPixel->push_back(VK_SAMPLE_COUNT_1_BIT);
	vectorAttachmentSamplesPerPixel->push_back(VK_SAMPLE_COUNT_1_BIT);

	vector<VkImageLayout>* vectorAttachmentFinalLayout = new vector<VkImageLayout>;
	vectorAttachmentFinalLayout->push_back(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	vectorAttachmentFinalLayout->push_back(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	vector<VkAttachmentReference>* vectorColorReference = new vector<VkAttachmentReference>;
	vectorColorReference->push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	MultiTypeUnorderedMap *attributeUM = new MultiTypeUnorderedMap();
	attributeUM->newElement<AttributeData<VkAttachmentReference*>*>(new AttributeData<VkAttachmentReference*>(string(g_renderPassAttachmentDepthReference), move(depthReference)));
	attributeUM->newElement<AttributeData<VkPipelineBindPoint*>*>(new AttributeData<VkPipelineBindPoint*>(string(g_renderPassAttachmentPipelineBindPoint), move(pipelineBindPoint)));
	attributeUM->newElement<AttributeData<vector<VkFormat>*>*>(new AttributeData<vector<VkFormat>*>(string(g_renderPassAttachmentFormat), move(vectorAttachmentFormat)));
	attributeUM->newElement<AttributeData<vector<VkSampleCountFlagBits>*>*>(new AttributeData<vector<VkSampleCountFlagBits>*>(string(g_renderPassAttachmentSamplesPerPixel), move(vectorAttachmentSamplesPerPixel)));
	attributeUM->newElement<AttributeData<vector<VkImageLayout>*>*>(new AttributeData<vector<VkImageLayout>*>(string(g_renderPassAttachmentFinalLayout), move(vectorAttachmentFinalLayout)));
	attributeUM->newElement<AttributeData<vector<VkAttachmentReference>*>*>(new AttributeData<vector<VkAttachmentReference>*>(string(g_renderPassAttachmentColorReference), move(vectorColorReference)));

	RenderPass* renderPass = renderPassM->buildRenderPass(move(string("swapChainRenderPass")), attributeUM);
	m_renderPass = renderPass->getRenderPass();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::destroyRenderpass()
{
	vkDestroyRenderPass(m_logicalDevice.getLogicalDevice(), m_renderPass, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::destroyCommandPools()
{
	vkDestroyCommandPool(m_logicalDevice.getLogicalDevice(), m_graphicsCommandPool, NULL);
	vkDestroyCommandPool(m_logicalDevice.getLogicalDevice(), m_computeCommandPool, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint CoreManager::getNextCommandBufferIndex()
{
	m_commandBufferIndex++;
	return m_commandBufferIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::initHardwareLimitValues()
{
	const VkPhysicalDeviceLimits& physicalDeviceLimits = coreM->getPhysicalDeviceProperties().limits;

	m_maxComputeWorkGroupInvocations= physicalDeviceLimits.maxComputeWorkGroupInvocations;
	m_maxComputeSharedMemorySize    = physicalDeviceLimits.maxComputeSharedMemorySize;
	m_maxComputeWorkGroupCount      = uvec3(physicalDeviceLimits.maxComputeWorkGroupCount[0], physicalDeviceLimits.maxComputeWorkGroupCount[1], physicalDeviceLimits.maxComputeWorkGroupCount[2]);
	m_maxComputeWorkGroupSize       = uvec3(physicalDeviceLimits.maxComputeWorkGroupSize[0], physicalDeviceLimits.maxComputeWorkGroupSize[1], physicalDeviceLimits.maxComputeWorkGroupSize[2]);
	m_minComputeWorkGroupSize       = uvec3(128, 128, 64); // Value given by the specification
	m_maxPushConstantsSize          = physicalDeviceLimits.maxPushConstantsSize;
	m_maxImageArrayLayers           = physicalDeviceLimits.maxImageArrayLayers;
	m_maxImageDimension1D           = physicalDeviceLimits.maxImageDimension1D;
	m_maxImageDimension2D           = physicalDeviceLimits.maxImageDimension2D;
	m_maxImageDimension3D           = physicalDeviceLimits.maxImageDimension3D;
	m_maxImageDimensionCube         = physicalDeviceLimits.maxImageDimensionCube;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::showHardwareLimits()
{
	cout << "INFO: hardware limits: maxComputeWorkGroupInvocations=" << m_maxComputeWorkGroupInvocations << ", min=128" << endl;
	cout << "INFO: hardware limits: maxComputeSharedMemorySize    =" << m_maxComputeSharedMemorySize << ", min=16384" << endl;
	cout << "INFO: hardware limits: maxComputeWorkGroupCount      =(" << m_maxComputeWorkGroupCount.x << "," << m_maxComputeWorkGroupCount.y << "," << m_maxComputeWorkGroupCount.z << "), min=(65535, 65535, 65535)" << endl;
	cout << "INFO: hardware limits: maxComputeWorkGroupSize       =(" << m_maxComputeWorkGroupSize.x << "," << m_maxComputeWorkGroupSize.y << "," << m_maxComputeWorkGroupSize.z << "), min=(128, 128, 64)" << endl;
	cout << "INFO: hardware limits: maxPushConstantsSize          =" << m_maxPushConstantsSize << endl;
	cout << "INFO: hardware limits: maxImageArrayLayers           =" << m_maxImageArrayLayers << endl;
	cout << "INFO: hardware limits: maxImageDimension1D           =" << m_maxImageDimension1D << endl;
	cout << "INFO: hardware limits: maxImageDimension2D           =" << m_maxImageDimension2D << endl;
	cout << "INFO: hardware limits: maxImageDimension3D           =" << m_maxImageDimension3D << endl;
	cout << "INFO: hardware limits: maxImageDimensionCube         =" << m_maxImageDimensionCube << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

ivec2 CoreManager::getSwapChainDimensions()
{
	return ivec2(m_swapChain.getSwapChainExtent().width, m_swapChain.getSwapChainExtent().height);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::allocCommandBuffer(const VkDevice* device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo)
{
	// Dependency on the intialize SwapChain Extensions and initialize CommandPool
	VkResult result;

	// If command information is available use it as it is.
	if (commandBufferInfo)
	{
		result = vkAllocateCommandBuffers(*device, commandBufferInfo, cmdBuf);
		assert(!result);
		return;
	}

	// Default implementation, create the command buffer
	// allocation info and use the supplied parameter into it
	VkCommandBufferAllocateInfo cmdInfo = {};
	cmdInfo.sType		= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdInfo.pNext		= NULL;
	cmdInfo.commandPool = cmdPool;
	cmdInfo.level		= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdInfo.commandBufferCount = (uint32_t) sizeof(cmdBuf) / sizeof(VkCommandBuffer);;

	result = vkAllocateCommandBuffers(*device, &cmdInfo, cmdBuf);
	assert(!result);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo)
{
	// Dependency on  the initialieCommandBuffer()
	VkResult  result;
	// If the user has specified the custom command buffer use it
	if (inCmdBufInfo)
	{
		result = vkBeginCommandBuffer(cmdBuf, inCmdBufInfo);
		assert(result == VK_SUCCESS);
		return;
	}

	// Otherwise, use the default implementation.
	VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
	cmdBufInheritInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cmdBufInheritInfo.pNext					= NULL;
	cmdBufInheritInfo.renderPass			= VK_NULL_HANDLE;
	cmdBufInheritInfo.subpass				= 0;
	cmdBufInheritInfo.framebuffer			= VK_NULL_HANDLE;
	cmdBufInheritInfo.occlusionQueryEnable	= VK_FALSE;
	cmdBufInheritInfo.queryFlags			= 0;
	cmdBufInheritInfo.pipelineStatistics	= 0;
	
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext				= NULL;
	cmdBufInfo.flags				= 0;
	cmdBufInfo.pInheritanceInfo		= &cmdBufInheritInfo;

	result = vkBeginCommandBuffer(cmdBuf, &cmdBufInfo);

	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::endCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkResult  result;
	result = vkEndCommandBuffer(commandBuffer);
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* commandBuffer, const VkSubmitInfo* inSubmitInfo, const VkFence& fence)
{
	VkResult result;
	
	// If Subimt information is avialable use it as it is, this assumes that 
	// the commands are already specified in the structure, hence ignore command buffer 
	if (inSubmitInfo)
	{
		result = vkQueueSubmit(queue, 1, inSubmitInfo, fence);
		assert(!result);

		result = vkQueueWaitIdle(queue);
		assert(!result);
		return;
	}

	// Otherwise, create the submit information with specified buffer commands
	VkSubmitInfo submitInfo = {};
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext				= NULL;
	submitInfo.waitSemaphoreCount	= 0;
	submitInfo.pWaitSemaphores		= NULL;
	submitInfo.pWaitDstStageMask	= NULL;
	submitInfo.commandBufferCount	= (uint32_t)sizeof(commandBuffer)/sizeof(VkCommandBuffer);
	submitInfo.pCommandBuffers		= commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores	= NULL;

	result = vkQueueSubmit(queue, 1, &submitInfo, fence);
	assert(!result);

	result = vkQueueWaitIdle(queue);
	assert(!result);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::initializeQueryPools()
{
	// The current approach is to give each rastertechnique two indices in the query pool
	// (this could be customized by raster technique), so first GPUPipeline::init() needs to be called,
	// to know how many raster techniques there are

	const vectorRasterTechniquePtr& vectorRasterTechnique = gpuPipelineM->getVectorRasterTechnique();
	const uint numRasterTechnique                         = uint(vectorRasterTechnique.size());

	VkQueryPoolCreateInfo queryPoolCreateInfo = {};
	queryPoolCreateInfo.sType      = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolCreateInfo.pNext      = nullptr;
	queryPoolCreateInfo.queryType  = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolCreateInfo.queryCount = numRasterTechnique * 2;

	// Build graphics queue query poool
	VkResult result = vkCreateQueryPool(m_logicalDevice.getLogicalDevice(), &queryPoolCreateInfo, nullptr, &m_graphicsQueueQueryPool);
	assert(result == VK_SUCCESS);

	// The query pool needs to be reset before any query can be used
	VkCommandBuffer commandBuffer0;
	allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), &commandBuffer0);
	beginCommandBuffer(commandBuffer0);
	vkCmdResetQueryPool(commandBuffer0, m_graphicsQueueQueryPool, 0, queryPoolCreateInfo.queryCount);
	endCommandBuffer(commandBuffer0);
	submitCommandBuffer(coreM->getLogicalDeviceGraphicsQueue(), &commandBuffer0);

	// Build compute queue query poool
	result = vkCreateQueryPool(m_logicalDevice.getLogicalDevice(), &queryPoolCreateInfo, nullptr, &m_computeQueueQueryPool);
	assert(result == VK_SUCCESS);

	VkCommandBuffer commandBuffer1;
	allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getComputeCommandPool(), &commandBuffer1);
	beginCommandBuffer(commandBuffer1);
	vkCmdResetQueryPool(commandBuffer1, m_computeQueueQueryPool, 0, queryPoolCreateInfo.queryCount);
	endCommandBuffer(commandBuffer1);
	submitCommandBuffer(coreM->getLogicalDeviceComputeQueue(), &commandBuffer1);

	uint counter = 0;
	forI(numRasterTechnique)
	{
		vectorRasterTechnique[i]->setQueryIndex0(counter++);
		vectorRasterTechnique[i]->setQueryIndex1(counter++);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CoreManager::destroyQueryPool()
{
	vkDestroyQueryPool(m_logicalDevice.getLogicalDevice(), m_graphicsQueueQueryPool, nullptr);
	vkDestroyQueryPool(m_logicalDevice.getLogicalDevice(), m_computeQueueQueryPool, nullptr);
	m_graphicsQueueQueryPool = VK_NULL_HANDLE;
	m_computeQueueQueryPool  = VK_NULL_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
