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

#ifndef _INSTANCE_H_
#define _INSTANCE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/util/getsetmacros.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Instance
{
public:
	/** Default constructor
	* @return nothing */
	Instance();

	/** Retrieves the layer properties available, storing them in the arrayLayerPropertyList given as parameter
	* @param arrayLayerPropertyList [inout] array to store layer information
	* @return nothing */
	void getInstanceLayerProperties(vector<LayerProperties>& arrayLayerPropertyList);

	/** Prints the information present in the arrayLayerPropertyList given as parameter
	* @param arrayLayerPropertyList [in] array with the layer information to print
	* @return nothing */
	void printLayerAndExtensionData(const vector<LayerProperties>& arrayLayerPropertyList);

	/** Inspects the incoming layer names against system supported layers, theses layers are not supported then this function
	* removed it from arrayLayerNames allow
	* @param arrayLayerNames        [in] array with the layer names to know if they're supported
	* @param arrayLayerPropertyList [in] array with the layer properties to know if elements in arrayLayerNames are supported
	* @return nothing */
	void areLayersSupported(vector<const char*>& arrayLayerNames, const vector<LayerProperties>& arrayLayerPropertyList);

	/** Builds an instance with the layers and extensions given as parameter
	* @param arrayLayer      [in] array with the layer names to add to the instance
	* @param arrayExtension  [in] array with the extension names to add to the instance
	* @param applicationName [in] application name
	* @return the built instance */
	void createInstance(vector<const char*>& arrayLayer, vector<const char*>& arrayExtension, const char* applicationName);

	/** Destroys the given instance
	* @param instance [inout] instance to destroy
	* @return nothing */
	void destroyInstance();

	/** Function called when there's debug information once the debug report callback has been built
	* @param msgFlags    [in] flags for the message
	* @param objType     [in] object type
	* @param srcObject   [in] source object
	* @param location    [in] location
	* @param msgCode     [in] message code
	* @param layerPrefix [in] prefix of the layer calling
	* @param msg         [in] message
	* @param userData    [in] user data
	* @return true if the type of debug function was identified and false otherwise */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugFunction(VkFlags msgFlags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char *layerPrefix,
		const char *msg,
		void *userData);

	/** Builds the debug report callback function
	* @return result of the operation */
	VkResult createDebugReportCallback();

	/** Destroys the report callback
	* @return nothing */
	void destroyDebugReportCallback();

	GET(VkInstance, m_instance, Instance)

protected:
	PFN_vkCreateDebugReportCallbackEXT  m_dbgCreateDebugReportCallback;  //!< Function pointer to build the debugging report callback
	PFN_vkDestroyDebugReportCallbackEXT m_dbgDestroyDebugReportCallback; //!< Function pointer to destroy the debugging report callback
	VkDebugReportCallbackEXT            m_debugReportCallback;           //!< Function pointer to the debug callback function
	VkDebugReportCallbackCreateInfoEXT  m_dbgReportCreateInfo;           //!< Struct used to store flags and data when building the debug report callback
    VkInstance                          m_instance;                      //!< Vulkan instance used
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _INSTANCE_H_
