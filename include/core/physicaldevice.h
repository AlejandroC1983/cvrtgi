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

#ifndef _PHYSICALDEVICE_H_
#define _PHYSICALDEVICE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class PhysicalDevice
{
public:
	/** Default constructor
	* @return nothing */
	PhysicalDevice();

	/** Builds a vector with the physical devices found for the instance given
	* @param instance [in] instance to look for physical devices
	* @return a vector with the physical devices found for the instance given */
	vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance);

	/** Builds sets in the layerPropertyList vector parameter the extension data for each leyer data in the vector
	* @param physicalDevice    [in]    physical device to look for device extensions
	* @param layerPropertyList [inout] vector to fill information for each layer information element in the vector
	* @return result of the computation (VK_SUCCESS if everything went fine) */
	VkResult getDeviceExtensionProperties(vector<LayerProperties>& layerPropertyList);

	/** Prints the extension data of the layers given as input in layerPropertyList
	* @param layerPropertyList [in] vector with the layers to print information
	* @return nothing */
	void printDeviceExtensionData(const vector<LayerProperties>& layerPropertyList);

	/** Returns a struct of type VkPhysicalDeviceProperties with the physical device properties
	* retrieved from the device given as parameter
	* @param physicalDevice [in] physical device to retrieve information from
	* @return nothing */
	void getPhysicalDeviceProperties();

	/** Returns a struct of type VkPhysicalDeviceMemoryProperties with the physical device memory properties
	* retrieved from the device given as parameter
	* @param physicalDevice [in] physical device to retrieve information from
	* @return struct of type VkPhysicalDeviceMemoryProperties with the physical device memory properties retrieved
	* from the device given as parameter
	* @return nothing */
	void retrievePhysicalDeviceMemoryProperties();

	/** Returns a vector with the available queues exposed by the physical devices
	* @param physicalDevice [in] physical device to retrieve information from
	* @return nothing */
	void getPhysicalDeviceQueuesAndProperties();

	/** Returns the handle index of the first graphics queue found in queueFamilyProps
	* @param queueFamilyProps [in] array of VkQueueFamilyProperties to retrieve graphics queue properties from
	* @return nothing */
	void getGraphicsQueueHandle();

	/** Returns the handle index of the first compute queue found in queueFamilyProps
	* @param queueFamilyProps [in] array of VkQueueFamilyProperties to retrieve compute queue properties from
	* @return nothing */
	void getComputeQueueHandle();

	/** Gets the selecred physical device features
	* @return nothing */
	void getPhysicalDeviceFeatures();

	/** Returns the handle index of the first graphics queue found in queueFamilyProps
	* @param typeBits          [in]    memory bit types
	* @param requirements_mask [in]    bit mask describing the requirements
	* @param memoryTypes       [in]    pointer to array of memory types
	* @param typeIndex         [inout] retrieved type index
	* @return true if the memory type was found, false otherwise */
	bool memoryTypeFromProperties(uint32_t typeBits, VkMemoryPropertyFlags requirementMask, const VkMemoryType* memoryTypes, uint32_t& typeIndex);

	GET(vector<VkQueueFamilyProperties>, m_queueFamilyProps, QueueFamilyProps)
	GET(VkPhysicalDeviceMemoryProperties, m_physicalDeviceMemoryProperties, PhysicalDeviceMemoryProperties)
	REF(vector<LayerProperties>, m_layerPropertyList, LayerPropertyList)
	GET_SET(VkPhysicalDevice, m_physicalDevice, PhysicalDevice)
	GETCOPY(uint32_t, m_graphicsQueueIndex, GraphicsQueueIndex)
	GETCOPY(uint32_t, m_computeQueueIndex, ComputeQueueIndex)
	GET(VkPhysicalDeviceProperties, m_physicalDeviceProperties, PhysicalDeviceProperties)

protected:
	VkPhysicalDeviceFeatures         m_physicalDeviceFeatures;         //!< Physical device features: this variable shouldn't be here!!
	vector<VkQueueFamilyProperties>  m_queueFamilyProps;               //!< Store all queue families exposed by the physical device. attributes
	uint32_t						 m_graphicsQueueIndex;             //!< Stores graphics queue index
	uint32_t						 m_computeQueueIndex;              //!< Stores graphics queue index
	VkPhysicalDevice				 m_physicalDevice;		           //!< Physical device
	VkPhysicalDeviceProperties		 m_physicalDeviceProperties;	   //!< Physical device attributes
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties; //!< Physical device memory properties
	vector<LayerProperties>          m_layerPropertyList;              //!< List pof properties of layers
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _PHYSICALDEVICE_H_
