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
#include "../../include/core/physicaldevice.h"
#include "../../include/core/coremanager.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

PhysicalDevice::PhysicalDevice():
	  m_physicalDeviceFeatures({})
	, m_graphicsQueueIndex(UINT32_MAX)
	, m_computeQueueIndex(UINT32_MAX)
	, m_physicalDevice(VK_NULL_HANDLE)
	, m_physicalDeviceProperties({})
	, m_physicalDeviceMemoryProperties({})
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

vector<VkPhysicalDevice> PhysicalDevice::enumeratePhysicalDevices(VkInstance instance)
{
	uint32_t gpuDeviceCount;
	vector<VkPhysicalDevice> arrayGPU;

	VkResult result = vkEnumeratePhysicalDevices(instance, &gpuDeviceCount, NULL);
	assert(result == VK_SUCCESS);

	assert(gpuDeviceCount);

	arrayGPU.resize(gpuDeviceCount);

	result = vkEnumeratePhysicalDevices(instance, &gpuDeviceCount, arrayGPU.data());
	assert(result == VK_SUCCESS);

	return arrayGPU;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkResult PhysicalDevice::getDeviceExtensionProperties(vector<LayerProperties>& layerPropertyList)
{
	VkResult result = VK_SUCCESS; // Variable to check Vulkan API result status
	uint32_t extensionCount;
	for (LayerProperties& globalLayerProp : layerPropertyList)
	{
		result = vkEnumerateDeviceExtensionProperties(m_physicalDevice, globalLayerProp.properties.layerName, &extensionCount, NULL);
		globalLayerProp.extensions.resize(extensionCount);
		result = vkEnumerateDeviceExtensionProperties(m_physicalDevice, globalLayerProp.properties.layerName, &extensionCount, globalLayerProp.extensions.data());
	}

	//printDeviceExtensionData(layerPropertyList);
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::printDeviceExtensionData(const vector<LayerProperties>& layerPropertyList)
{
	cout << "Device extensions" << endl;
	cout << "===================" << endl;
	for (const LayerProperties& layerProperty : layerPropertyList)
	{
		cout << "\n" << layerProperty.properties.description << "\n\t|\n\t|---[Layer Name]--> " << layerProperty.properties.layerName << "\n";
		if (layerProperty.extensions.size())
		{
			for (auto j : layerProperty.extensions)
			{
				cout << "\t\t|\n\t\t|---[Device Extesion]--> " << j.extensionName << "\n";
			}
		}
		else
		{
			cout << "\t\t|\n\t\t|---[Device Extesion]--> No extension found \n";
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::getPhysicalDeviceProperties()
{
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties); // Get the physical device or GPU properties
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::retrievePhysicalDeviceMemoryProperties()
{
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties); // Get the memory properties from the physical device or GPU.
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::getPhysicalDeviceQueuesAndProperties()
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, NULL); // Query queue families count with pass NULL as second parameter.
	m_queueFamilyProps.resize(queueFamilyCount); // Allocate space to accomodate Queue properties.
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProps.data()); // Get queue family properties
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::getGraphicsQueueHandle()
{
	//	1. Get the number of Queues supported by the Physical device
	//	2. Get the properties each Queue type or Queue Family
	//			There could be 4 Queue type or Queue families supported by physical device - 
	//			Graphics Queue	- VK_QUEUE_GRAPHICS_BIT 
	//			Compute Queue	- VK_QUEUE_COMPUTE_BIT
	//			DMA				- VK_QUEUE_TRANSFER_BIT
	//			Sparse memory	- VK_QUEUE_SPARSE_BINDING_BIT
	//	3. Get the index ID for the required Queue family, this ID will act like a handle index to queue.

	bool found = false;
	// 1. Iterate number of Queues supported by the Physical device
	for (unsigned int i = 0; i < m_queueFamilyProps.size(); i++)
	{
		// 2. Get the Graphics Queue type
		//		There could be 4 Queue type or Queue families supported by physical device - 
		//		Graphics Queue		- VK_QUEUE_GRAPHICS_BIT 
		//		Compute Queue		- VK_QUEUE_COMPUTE_BIT
		//		DMA/Transfer Queue	- VK_QUEUE_TRANSFER_BIT
		//		Sparse memory		- VK_QUEUE_SPARSE_BINDING_BIT

		if (m_queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			// 3. Get the handle/index ID of graphics queue family.
			found = true;
			m_graphicsQueueIndex = i;
			break;
		}
	}

	// Assert if there is no queue found.
	assert(found);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::getComputeQueueHandle()
{
	// Try to find a dedicated compute queue, if not available, then just take the first queue family
	// with compute capabilities
	bool found = false;
	for (unsigned int i = 0; i < m_queueFamilyProps.size(); i++)
	{
		if ((m_queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && 
		   ((m_queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
		{
			found = true;
			m_computeQueueIndex = i;
			break;
		}
	}

	if (!found)
	{
		for (unsigned int i = 0; i < m_queueFamilyProps.size(); i++)
		{
			if (m_queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				found = true;
				m_computeQueueIndex = i;
				break;
			}
		}
	}
	
	assert(found);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void PhysicalDevice::getPhysicalDeviceFeatures()
{
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool PhysicalDevice::memoryTypeFromProperties(uint32_t typeBits, VkMemoryPropertyFlags requirementsMask, const VkMemoryType* memoryTypes, uint32_t& typeIndex)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++)
	{
		if ((typeBits & 1) == 1)
		{
			// Type is available, does it match user properties?
			if ((memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
			{
				typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
