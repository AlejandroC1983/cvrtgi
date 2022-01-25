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
#include "../../include/core/instance.h"
#include "../../include/core/coremanager.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Instance::Instance():
	  m_dbgCreateDebugReportCallback(nullptr)
	, m_dbgDestroyDebugReportCallback(nullptr)
	, m_debugReportCallback(nullptr)
	, m_dbgReportCreateInfo({})
	, m_instance(VK_NULL_HANDLE)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Instance::getInstanceLayerProperties(vector<LayerProperties>& arrayLayerPropertyList)
{
	uint32_t                  instanceLayerCount; // Stores number of layers supported by instance
	vector<VkLayerProperties> layerProperties;    // Vector to store layer properties

	VkResult result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);
	assert(result == VK_SUCCESS);

	if (instanceLayerCount == 0)
	{
		return; // Warning: VK_INCOMPLETE
	}

	layerProperties.resize(instanceLayerCount);
	result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());

	LayerProperties layerProps;
	uint32_t extensionCount;

	for (VkLayerProperties& globalLayerProp : layerProperties)
	{
		layerProps.properties = globalLayerProp;
		layerProps.extensions.clear();

		result = vkEnumerateInstanceExtensionProperties(layerProps.properties.layerName, &extensionCount, NULL);

		if ((result == VK_SUCCESS) && (extensionCount > 0))
		{
			layerProps.extensions.resize(extensionCount);
			result = vkEnumerateInstanceExtensionProperties(layerProps.properties.layerName, &extensionCount, layerProps.extensions.data());
		}

		if (result != VK_SUCCESS)
		{
			continue;
		}

		arrayLayerPropertyList.push_back(layerProps);
	}
	printLayerAndExtensionData(arrayLayerPropertyList);
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Instance::printLayerAndExtensionData(const vector<LayerProperties>& arrayLayerPropertyList)
{
	cout << "\nInstanced Layers" << endl;
	cout << "===================" << endl;

	for (int i = 0; i < arrayLayerPropertyList.size(); ++i)
	{
		const LayerProperties* layerProperty = &arrayLayerPropertyList[i];
		cout << "\n" << layerProperty->properties.description << "\n\t|\n\t|---[Layer Name]--> " << layerProperty->properties.layerName << "\n";
		for (const VkExtensionProperties& extensionProperty : layerProperty->extensions)
		{
			cout << "\t\t|\n\t\t|---[Layer Extension]--> " << extensionProperty.extensionName << "\n";
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Instance::areLayersSupported(vector<const char*>& arrayLayerNames, const vector<LayerProperties>& arrayLayerPropertyList)
{
	uint32_t checkCount = (uint32_t)arrayLayerNames.size();
	uint32_t layerCount = (uint32_t)arrayLayerPropertyList.size();
	vector<const char*> unsupportLayerNames;
	VkBool32 isSupported;
	for (uint32_t i = 0; i < checkCount; i++)
	{
		isSupported = 0;
		for (uint32_t j = 0; j < layerCount; j++)
		{
			if (!strcmp(arrayLayerNames[i], arrayLayerPropertyList[j].properties.layerName))
			{
				isSupported = 1;
			}
		}

		if (!isSupported)
		{
			cout << "No Layer support found, removed from layer: " << arrayLayerNames[i] << endl;
			unsupportLayerNames.push_back(arrayLayerNames[i]);
		}
		else
		{
			cout << "Layer supported: " << arrayLayerNames[i] << endl;
		}
	}

	for (auto i : unsupportLayerNames)
	{
		auto it = find(arrayLayerNames.begin(), arrayLayerNames.end(), i);
		if (it != arrayLayerNames.end())
		{
			arrayLayerNames.erase(it);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Instance::createInstance(vector<const char*>& arrayLayer, vector<const char*>& arrayExtensionName, char const*const applicationName)
{
	// Define the Vulkan application structure 
	VkApplicationInfo appInfo = {};
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext              = NULL;
	appInfo.pApplicationName   = applicationName;
	appInfo.applicationVersion = 1;
	appInfo.pEngineName        = applicationName;
	appInfo.engineVersion      = 1;
	appInfo.apiVersion         = VK_MAKE_VERSION(1, 0, 0); // VK_API_VERSION is now deprecated, use VK_MAKE_VERSION instead.

	// Define the Vulkan instance create info structure 
	VkInstanceCreateInfo instInfo = {};
	instInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pNext            = &m_dbgReportCreateInfo;
	instInfo.flags            = 0;
	instInfo.pApplicationInfo = &appInfo;

	// Specify the list of layer name to be enabled.
	instInfo.enabledLayerCount   = (uint32_t)arrayLayer.size();
	instInfo.ppEnabledLayerNames = arrayLayer.size() ? arrayLayer.data() : NULL;

	// Specify the list of extensions to be used in the application.
	instInfo.enabledExtensionCount   = (uint32_t)arrayExtensionName.size();
	instInfo.ppEnabledExtensionNames = arrayExtensionName.size() ? arrayExtensionName.data() : NULL;

	VkResult result = vkCreateInstance(&instInfo, NULL, &m_instance);
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Instance::destroyInstance()
{
	vkDestroyInstance(m_instance, NULL);

	m_instance                      = VK_NULL_HANDLE;
	m_dbgCreateDebugReportCallback  = nullptr;
	m_dbgDestroyDebugReportCallback = nullptr;
	m_debugReportCallback           = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugFunction(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject, size_t location, int32_t msgCode,
	const char *layerPrefix, const char *msg, void *userData)
{

	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		cout << "[VK_DEBUG_REPORT] ERROR: [" << layerPrefix << "] Code" << msgCode << ":" << msg << endl;
	}
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		cout << "[VK_DEBUG_REPORT] WARNING: [" << layerPrefix << "] Code" << msgCode << ":" << msg << endl;
	}
	else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		cout << "[VK_DEBUG_REPORT] INFORMATION: [" << layerPrefix << "] Code" << msgCode << ":" << msg << endl;
	}
	else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		cout << "[VK_DEBUG_REPORT] PERFORMANCE: [" << layerPrefix << "] Code" << msgCode << ":" << msg << endl;
	}
	else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		cout << "[VK_DEBUG_REPORT] DEBUG: [" << layerPrefix << "] Code" << msgCode << ":" << msg << endl;
	}
	else
	{
		return VK_FALSE;
	}

	fflush(stdout);
	return VK_FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkResult Instance::createDebugReportCallback()
{
	// Get vkCreateDebugReportCallbackEXT API
	m_dbgCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(coreM->getInstance(), "vkCreateDebugReportCallbackEXT");
	if (!m_dbgCreateDebugReportCallback)
	{
		cout << "Error: GetInstanceProcAddr unable to locate vkCreateDebugReportCallbackEXT function." << endl;
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	cout << "GetInstanceProcAddr loaded dbgCreateDebugReportCallback function\n";

	// Get vkDestroyDebugReportCallbackEXT API
	m_dbgDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(coreM->getInstance(), "vkDestroyDebugReportCallbackEXT");
	if (!m_dbgDestroyDebugReportCallback)
	{
		cout << "Error: GetInstanceProcAddr unable to locate vkDestroyDebugReportCallbackEXT function." << endl;
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	cout << "GetInstanceProcAddr loaded dbgDestroyDebugReportCallback function\n";

	// Define the debug report control structure, provide the reference of 'debugFunction'
	// , this function prints the debug information on the console.
	m_dbgReportCreateInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	m_dbgReportCreateInfo.pfnCallback = debugFunction;
	m_dbgReportCreateInfo.pUserData   = NULL;
	m_dbgReportCreateInfo.pNext       = NULL;
	m_dbgReportCreateInfo.flags       = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		                                VK_DEBUG_REPORT_ERROR_BIT_EXT   | VK_DEBUG_REPORT_DEBUG_BIT_EXT;

	// Create the debug report callback and store the handle into 'debugReportCallback'
	VkResult result = m_dbgCreateDebugReportCallback(coreM->getInstance(), &m_dbgReportCreateInfo, NULL, &m_debugReportCallback);
	if (result == VK_SUCCESS)
	{
		cout << "Debug report callback object created successfully\n";
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Instance::destroyDebugReportCallback()
{
	m_dbgDestroyDebugReportCallback(coreM->getInstance(), m_debugReportCallback, NULL);
	m_debugReportCallback = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////
