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

#ifndef _LOGICALDEVICE_H_
#define _LOGICALDEVICE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/util/getsetmacros.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class LogicalDevice
{
public:
	/** Default constructor
	* @return nothing */
	LogicalDevice();

	/** Builds a new logical device, result is stored in m_device
	* @param extensions        [in] extensions to add to the logical device
	* @param graphicQueueIndex [in] graphics queue index
	* @param computeQueueIndex [in] graphics queue index
	* @param physicalDevice    [in] physical device
	* @return nothing */
	void createDevice(vector<const char*>& extensions, uint32_t graphicQueueIndex, uint32_t computeQueueIndex, VkPhysicalDevice physicalDevice);

	/** Destroys the logical device used
	* @return nothing */
	void destroyDevice();

	/** Assigns to m_logicalDeviceGraphicsQueue the value of the graphics queue with presentation capabilities
	* @return nothing */
	void requestGraphicsQueue(uint32_t graphicsQueueIndex);

	/** Assigns to m_logicalDeviceComputeQueue the value of the graphics queue with presentation capabilities
	* @return nothing */
	void requestComputeQueue(uint32_t computeQueueIndex);

	GET(VkQueue, m_logicalDeviceGraphicsQueue, LogicalDeviceGraphicsQueue)
	GET(VkQueue, m_logicalDeviceComputeQueue, LogicalDeviceComputeQueue)
	GET(VkDevice, m_logicalDevice, LogicalDevice)

protected:
	VkQueue	 m_logicalDeviceGraphicsQueue; //!< Logical device graphics queue
	VkQueue	 m_logicalDeviceComputeQueue;  //!< Logical device compute queue
	VkDevice m_logicalDevice;	           //!< Logical device
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _LOGICALDEVICE_H_
