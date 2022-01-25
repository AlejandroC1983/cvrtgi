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
#include "../../include/core/surface.h"
#include "../../include/core/gpupipeline.h"
#include "../../include/core/coremanager.h"
#include "../../include/core/swapchain.h"
#include "../../include/core/input.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Surface::Surface():
	  m_fpGetPhysicalDeviceSurfaceSupportKHR(VK_NULL_HANDLE)
	, m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_NULL_HANDLE)
	, m_fpGetPhysicalDeviceSurfaceFormatsKHR(VK_NULL_HANDLE)
	, m_fpGetPhysicalDeviceSurfacePresentModesKHR(VK_NULL_HANDLE)
	, m_fpDestroySurfaceKHR(VK_NULL_HANDLE)
	, m_surfaceCapabilities({})
	, m_surface(VK_NULL_HANDLE)
	, m_format(VK_FORMAT_UNDEFINED)
	, m_preTransform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	, m_width(0)
	, m_height(0)
	, m_graphicsQueueWithPresentIndex(UINT_MAX)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::getSupportedFormats()
{
	// Get the list of VkFormats that are supported:
	uint32_t formatCount;
	VkResult result = m_fpGetPhysicalDeviceSurfaceFormatsKHR(coreM->getPhysicalDevice(), m_surface, &formatCount, NULL);
	assert(result == VK_SUCCESS);
	m_arraySurfaceFormats.clear();
	m_arraySurfaceFormats.resize(formatCount);

	// Get VkFormats in allocated objects
	result = m_fpGetPhysicalDeviceSurfaceFormatsKHR(coreM->getPhysicalDevice(), m_surface, &formatCount, &m_arraySurfaceFormats[0]);
	assert(result == VK_SUCCESS);

	// In case it’s a VK_FORMAT_UNDEFINED, then surface has no preferred format. We use BGRA 32 bit format
	if (formatCount == 1 && m_arraySurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		m_format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		assert(formatCount >= 1);
		m_format = m_arraySurfaceFormats[0].format;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkResult Surface::createSurface()
{
	// Construct the surface description:
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext     = NULL;
	createInfo.hinstance = m_connection;
	createInfo.hwnd      = m_window;

	VkResult result = vkCreateWin32SurfaceKHR(coreM->getInstance(), &createInfo, NULL, &m_surface);
#else  // _WIN32

	VkXcbSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext      = NULL;
	createInfo.connection = rendererObj->connection;
	createInfo.window     = rendererObj->window;

	VkResult result = vkCreateXcbSurfaceKHR(CoreManager::m_instance, &createInfo, NULL, &surface);
#endif // _WIN32

	assert(result == VK_SUCCESS);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Surface::getGraphicsQueueWithPresentationSupport()
{
	uint32_t queueCount = uint32_t(coreM->getQueueFamilyProps().size());
	// Iterate over each queue and get presentation status for each.
	VkBool32* supportsPresent = (VkBool32 *)malloc(queueCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < queueCount; i++)
	{
		m_fpGetPhysicalDeviceSurfaceSupportKHR(coreM->getPhysicalDevice(), i, m_surface, &supportsPresent[i]);
	}

	// Search for a graphics queue and a present queue in the array of queue
	// families, try to find one that supports both
	m_graphicsQueueWithPresentIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex  = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++)
	{
		if ((coreM->getQueueFamilyProps()[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (m_graphicsQueueWithPresentIndex == UINT32_MAX)
			{
				m_graphicsQueueWithPresentIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				m_graphicsQueueWithPresentIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (uint32_t i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	free(supportsPresent);

	// Generate error if could not find both a graphics and a present queue
	if (m_graphicsQueueWithPresentIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
	{
		return  false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::getSurfaceCapabilitiesAndPresentMode()
{
	VkResult result = m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR(coreM->getPhysicalDevice(), m_surface, &m_surfaceCapabilities);
	assert(result == VK_SUCCESS);

	uint32_t presentModeCount;
	result = m_fpGetPhysicalDeviceSurfacePresentModesKHR(coreM->getPhysicalDevice(), m_surface, &presentModeCount, NULL);
	assert(result == VK_SUCCESS);

	m_arrayPresentMode.clear();
	m_arrayPresentMode.resize(presentModeCount);
	assert(m_arrayPresentMode.size() >= 1);

	result = m_fpGetPhysicalDeviceSurfacePresentModesKHR(coreM->getPhysicalDevice(), m_surface, &presentModeCount, &m_arrayPresentMode[0]);
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

// MS-Windows event handling function:
LRESULT CALLBACK Surface::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			coreM->setEndApplicationMessage(true);
			break;
		}
		case WM_PAINT:
		{
			ValidateRect(hWnd, NULL);
			//return 1;
			break;
		}
		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED)
			{
				int width = lParam & 0xffff;
				int height = (lParam & 0xffff0000) >> 16;
				coreM->setSwapChainExtent(width, height);
				coreM->resize();
			}
			break;

		default:
		{
			break;
		}
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::createPresentationWindow(bool fullscreen)
{
#ifdef _WIN32
	assert((m_width > 0) || (m_height > 0));

	WNDCLASSEX  winInfo;

	sprintf(m_name, "viewport");
	memset(&winInfo, 0, sizeof(WNDCLASSEX));
	// Initialize the window class structure:
	winInfo.cbSize        = sizeof(WNDCLASSEX);
	winInfo.style         = CS_HREDRAW | CS_VREDRAW;
	winInfo.lpfnWndProc   = WndProc;
	winInfo.cbClsExtra    = 0;
	winInfo.cbWndExtra    = 0;
	winInfo.hInstance     = m_connection; // hInstance
	winInfo.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	winInfo.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winInfo.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winInfo.lpszMenuName  = NULL;
	winInfo.lpszClassName = m_name;
	winInfo.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);

	// Register window class:
	if (!RegisterClassEx(&winInfo))
	{
		printf("Unexpected error trying to start the application!\n"); // It didn't work, so try to give a useful error:
		fflush(stdout);
		exit(1);
	}

	bool fullscreenSupported = true;

	int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	cout << "INFO: System screen resolution (" << screenWidth << ", " << screenHeight << ")" << endl;

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((m_width != (uint32_t)screenWidth) && (m_height != (uint32_t)screenHeight))
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				MessageBox(NULL, "Fullscreen Mode not supported!\n Switching to window mode", "Error", MB_OK | MB_ICONEXCLAMATION);
				fullscreenSupported = false;
			}
		}
	}

	DWORD dwExStyle;
	DWORD dwStyle;
	if (fullscreen && fullscreenSupported)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle   = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW     | WS_EX_WINDOWEDGE;
		dwStyle   = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	// Create window with the registered class:
	RECT windowRectangle;
	windowRectangle.left   = 0L;
	windowRectangle.top    = 0L;
	windowRectangle.right  = (fullscreen && fullscreenSupported) ? (long)screenWidth  : (long)m_width;
	windowRectangle.bottom = (fullscreen && fullscreenSupported) ? (long)screenHeight : (long)m_height;

	AdjustWindowRectEx(&windowRectangle, dwStyle, FALSE, dwExStyle);
	m_window = CreateWindowEx(0,
		m_name,                                        // class name
		m_name,                                        // app name
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0,                                          // x/y coords
		windowRectangle.right  - windowRectangle.left, // width
		windowRectangle.bottom - windowRectangle.top,  // height
		NULL,                                          // handle to parent
		NULL,                                          // handle to menu
		m_connection,                                  // hInstance
		NULL);                                         // no extra parameters

	if (!fullscreen)
	{
		// Center on screen
		int32_t x = abs(screenWidth  - windowRectangle.right)  / 2;
		int32_t y = abs(screenHeight - windowRectangle.bottom) / 2;
		SetWindowPos(m_window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if (!m_window)
	{
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}

	ShowWindow(m_window, SW_SHOW);
	SetForegroundWindow(m_window);
	SetFocus(m_window);

#else
	const xcb_setup_t *setup;
	xcb_screen_iterator_t iter;
	int scr;

	m_connection = xcb_connect(NULL, &scr);
	if (m_connection == NULL)
	{
		cout << "Cannot find a compatible Vulkan ICD.\n";
		exit(-1);
	}

	setup = xcb_get_setup(m_connection);
	iter = xcb_setup_roots_iterator(setup);
	while (scr-- > 0)
	{
		xcb_screen_next(&iter);
	}

	m_screen = iter.data;
#endif // _WIN32
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::destroyPresentationWindow()
{
	DestroyWindow(m_window);
}

/////////////////////////////////////////////////////////////////////////////////////////////

#else
void Surface::createPresentationWindow()
{
	assert(m_width > 0);
	assert(m_height > 0);

	uint32_t value_mask, value_list[32];

	m_window = xcb_generate_id(m_connection);

	value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	value_list[0] = m_screen->black_pixel;
	value_list[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE;

	xcb_create_window(m_connection, XCB_COPY_FROM_PARENT, m_window, m_screen->root, 0, 0, m_width, m_height, 0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT, m_screen->root_visual, value_mask, value_list);

	/* Magic code that will send notification when window is destroyed */
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(m_connection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(m_connection, cookie, 0);

	xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(m_connection, 0, 16, "WM_DELETE_WINDOW");
	reply = xcb_intern_atom_reply(m_connection, cookie2, 0);

	xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_window, (*reply).atom, 4, 32, 1, &(*reply).atom);
	free(reply);

	xcb_map_window(m_connection, m_window);

	// Force the x/y coordinates to 100,100 results are identical in consecutive runs
	const uint32_t coords[] = { 100,  100 };
	xcb_configure_window(m_connection, m_window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
	xcb_flush(m_connection);

	xcb_generic_event_t *e;
	while ((e = xcb_wait_for_event(m_connection)))
	{
		if ((e->response_type & ~0x80) == XCB_EXPOSE)
		{
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::destroyWindow()
{
	xcb_destroy_window(m_connection, m_window);
	xcb_disconnect(m_connection);
}

#endif // _WIN32

/////////////////////////////////////////////////////////////////////////////////////////////

bool Surface::render()
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		inputM->updateInput(msg);
	}
	RedrawWindow(m_window, NULL, NULL, RDW_INTERNALPAINT);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::createSurfaceExtensions()
{
	// Dependency on createPresentationWindow()
	VkInstance instance = coreM->getInstance();

	// Get Instance based swap chain extension function pointer
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceSupportKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
	INSTANCE_FUNC_PTR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
	INSTANCE_FUNC_PTR(instance, DestroySurfaceKHR);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::destroySurface()
{
	if (!coreM->getIsResizing())
	{
		vkDestroySurfaceKHR(coreM->getInstance(), m_surface, NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Surface::managePresentMode(VkPresentModeKHR& presentMode, uint32_t& numberOfSwapChainImages)
{
	// If mailbox mode is available, use it, as is the lowest-latency non-
	// tearing mode.  If not, try IMMEDIATE which will usually be available,
	// and is fastest (though it tears).  If not, fall back to FIFO which is
	// always available.
	presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < m_arrayPresentMode.size(); i++)
	{
		if (m_arrayPresentMode[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((presentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
			(m_arrayPresentMode[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	numberOfSwapChainImages = m_surfaceCapabilities.minImageCount + 1;
	if ((m_surfaceCapabilities.maxImageCount > 0) &&
		(numberOfSwapChainImages > m_surfaceCapabilities.maxImageCount))
	{
		// Application must settle for fewer images than desired:
		numberOfSwapChainImages = m_surfaceCapabilities.maxImageCount;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkExtent2D Surface::setSwapChainExtent()
{
	VkExtent2D swapChainExtent;
	if (m_surfaceCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// If the surface width and height is not defined, the set the equal to image size.
		swapChainExtent.width  = m_width;
		swapChainExtent.height = m_height;
	}
	else
	{
		// If the surface size is defined, then it must match the swap chain size
		// This value can differ form the window size
		swapChainExtent = m_surfaceCapabilities.currentExtent;
		m_width         = glm::min(m_width, swapChainExtent.width);
		m_height        = glm::min(m_height, swapChainExtent.height);
	}

	return swapChainExtent;
}

/////////////////////////////////////////////////////////////////////////////////////////////
