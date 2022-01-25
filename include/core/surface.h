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

#ifndef _SURFACE_H_
#define _SURFACE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/commonnamespace.h"

// CLASS FORWARDING
class CoreManager;

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Surface
{
public:
	/** Default constructor
	* @return nothing */
	Surface();

	/** Retrieves the surface supported formats using the function pointer m_fpGetPhysicalDeviceSurfaceFormatsKHR,
	* and storing the results in m_arraySurfaceFormats
	* @return nothing */
	void getSupportedFormats();

	/** Builds a surface with the window handle window and window instance connection
	* @return VK_SUCCESS if everything went ok, different value otherwise */
	VkResult createSurface();

	/** Retrieves with the function pointer m_fpGetPhysicalDeviceSurfaceSupportKHR those physical device queues that support
	* presentation, and, from those, returns the index of the first one that also supports graphics
	* @return true if the value was found, false otherwise */
	bool getGraphicsQueueWithPresentationSupport();

	/** Retrieves the information for the m_surfaceCapabilities and m_arrayPresentMode variables
	* @return nothing */
	void getSurfaceCapabilitiesAndPresentMode();
	
	/** Builds the presentation window, storing the result in the handler window
	* @param fullscreen [in] whether to build in fullscreen mode (if supported)
	* @return nothing */
	void createPresentationWindow(bool fullscreen);

	/** Destroys the built surface
	* @return nothing */
	void destroySurface();

	/** Windows procedure method for handling events.
	* @param hWnd   [in] window handler
	* @param uMsg   [in] window message
	* @param wParam [in] parameter
	* @param lParam [in] parameter
	* @return nothing */
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/** Destroy the presentation window
	* @return nothing */
	void destroyPresentationWindow();

	/** Final function which requests the actual redraw of the window
	* @return nothing */
	bool render();

	/** Fills the function pointers m_fpGetPhysicalDeviceSurfaceSupportKHR, m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR, 
	* m_fpGetPhysicalDeviceSurfaceFormatsKHR, m_fpGetPhysicalDeviceSurfacePresentModesKHR and m_fpDestroySurfaceKHR
	* @return nothing */
	void createSurfaceExtensions();

	/** Sets the most convenient values for m_swapchainPresentMode and m_desiredNumberOfSwapChainImages based on recovered data
	* @return nothing */
	void managePresentMode(VkPresentModeKHR& presentMode, uint32_t& numberOfSwapChainImages);

	/** Sets the extent of the swap chain images to be build
	* @return nothing */
	VkExtent2D setSwapChainExtent();

	GET(VkFormat, m_format, Format)
	GET(VkSurfaceKHR, m_surface, Surface)
	GET_SET(uint32_t, m_width, Width)
	GET_SET(uint32_t, m_height, Height)
	GET(uint32_t, m_graphicsQueueWithPresentIndex, GraphicsQueueWithPresentIndex)
	GET(VkSurfaceTransformFlagBitsKHR, m_preTransform, PreTransform)
	GET(HWND, m_window, Window)
	GET(VkSurfaceCapabilitiesKHR, m_surfaceCapabilities, SurfaceCapabilities)

protected:
#ifdef _WIN32
#define APP_NAME_STR_LEN 80
	HINSTANCE					                  m_connection;				                   //!< hInstance - Windows Instance
	char						                  m_name[APP_NAME_STR_LEN];                    //!< name - App name appearing on the window
	HWND						                  m_window;					                   //!< hWnd - the window handle
#else
	xcb_connection_t*			                  m_connection;                                //!< Connection
	xcb_screen_t*				                  m_screen;                                    //!< Screen
	xcb_window_t				                  m_window;                                    //!< Window
	xcb_intern_atom_reply_t*	                  m_reply;                                     //!< Reply
#endif

	PFN_vkGetPhysicalDeviceSurfaceSupportKHR      m_fpGetPhysicalDeviceSurfaceSupportKHR;      //!< Function pointer for surface functionalities
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR; //!< Function pointer for surface functionalities
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      m_fpGetPhysicalDeviceSurfaceFormatsKHR;      //!< Function pointer for surface functionalities
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR m_fpGetPhysicalDeviceSurfacePresentModesKHR; //!< Function pointer for surface functionalities
	PFN_vkDestroySurfaceKHR                       m_fpDestroySurfaceKHR;                       //!< Function pointer for surface functionalities
	VkSurfaceCapabilitiesKHR                      m_surfaceCapabilities;                       //!< Store the image surface capabilities
	vector<VkPresentModeKHR>                      m_arrayPresentMode;                          //!< Array for retrived present modes
	vector<VkSurfaceFormatKHR>                    m_arraySurfaceFormats;                       //!< Array of physical device supported formats
	VkSurfaceKHR                                  m_surface;                                   //!< The logical platform dependent surface object
	VkFormat                                      m_format;                                    //!< Format of the image 
	VkSurfaceTransformFlagBitsKHR                 m_preTransform;                              //!< Any transform applied to the surface, identity in general
	uint32_t	                                  m_width;                                     //!< Surface width
	uint32_t                                      m_height;                                    //!< Surface height
    uint32_t				                      m_graphicsQueueWithPresentIndex;             //!< Number of queue family exposed by device
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SURFACE_H_
