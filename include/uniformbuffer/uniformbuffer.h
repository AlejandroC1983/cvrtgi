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

#ifndef _UNIFORMBUFFER_H_
#define _UNIFORMBUFFER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/genericresource.h"
#include "../../include/uniformbuffer/cpubuffer.h"

// CLASS FORWARDING
class Buffer;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class UniformBuffer : public GenericResource
{
	friend class UniformBufferManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	UniformBuffer(string &&name);

	/** Destructor
	* @return nothing */
	virtual ~UniformBuffer();

	/** Builds the GPU uniform buffer, the CPU buffer must exist first, since the information from m_UBHostMemory needs to be passed as
	* parameter
	* @return nothing */
	void buildGPUBufer();

	/** Destroy the resources allocated by the buffer and initializes the member variables
	* @return nothing */
	void destroyResources();

public:
	/** Uploads to the GPU the buffer information present in the CPU copy, given by m_UBHostMemory
	* @return nothing */
	void uploadCPUBufferToGPU();

	GETCOPY(size_t, m_dynamicAllignment, DynamicAllignment)
	REF(VkDescriptorBufferInfo, m_bufferInfo, BufferInfo)
	GETCOPY_SET(int, m_minCellSize, MinCellSize)
	REF_PTR(Buffer, m_bufferInstance, BufferInstance)
	REF(CPUBuffer, m_CPUBuffer, CPUBuffer)

protected:
	VkBuffer                    m_vulkanBuffer;      //!< Vulkan buffer resource object handler
	VkDeviceMemory              m_deviceMemory;      //!< Buffer resource object's allocated device memory handler
	VkDescriptorBufferInfo      m_bufferInfo;        //!< Buffer info that need to supplied into write descriptor set (VkWriteDescriptorSet)
	vector<VkMappedMemoryRange> m_mappedRange;       //!< Metadata of memory mapped objects, currently the whole buffer is mapped from this uniform buffer wrapper
	uint8_t*                    m_data;              //!< Host pointer containing the mapped device address which is used to write data into.
	size_t                      m_dynamicAllignment; //!< Real cell size used to place elements
	int                         m_minCellSize;       //!< Minimum cell size requested when building the buffer
	Buffer*                     m_bufferInstance;    //!< Pointer to the buffer instance in the Vulkan buffer manager
	CPUBuffer                   m_CPUBuffer;         //!< CPU bufer mapping the information of the GPU buffer
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _UNIFORMBUFFER_H_
