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

#ifndef _BUFFER_H_
#define _BUFFER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/genericresource.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Buffer : public GenericResource
{
	friend class BufferManager;

protected:
	/** Parameter constructor
	* @param [in] buffer's name
	* @return nothing */
	Buffer(string &&name);

	/** Destroy the resources used by the buffer
	* @return nothing */
	void destroyResources();

public:
	/** Destructor
	* @return nothing */
	virtual ~Buffer();

	/** Copies to the address given by the data parameter the content of the buffer, if host visible
	* @return true if the copy operation was made successfully, false otherwise */
	bool getContent(void* data);

	/** Returns a vector of byte with the information of the buffer
	* @return true if the copy operation was made successfully, false otherwise */
	bool getContentCopy(vectorUint8& vectorData);

	/** Fills the memory of a buffer with the data present at dataPointer
	* The range of mapped buffer memory is invalidated to make it visible to the host. If the memory property is set
	* with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT then the driver may take care of this, otherwise for non-coherent mapped memory
	* vkInvalidateMappedMemoryRanges() needs to be called explicitly.
	* @param dataPointer [in] pointer to the data to fill the memory with (must match in size with the memory size of the buffer)
	* @return true if the set content was made successfully, false otherwise */
	bool setContent(const void* dataPointer);

	GET(VkDeviceSize, m_mappingSize, MappingSize)
	GET(VkBufferUsageFlags, m_usage, Usage)
	GET(VkFlags, m_requirementsMask, RequirementsMask)
	GET(VkDeviceMemory, m_memory, Memory)
	GET(VkBuffer, m_buffer, Buffer)
	REF_PTR(void, m_dataPointer, DataPointer)
	GET(VkDeviceSize, m_dataSize, DataSize)
	REF_PTR(uint8_t, m_mappedPointer, MappedPointer)
	GET(VkMappedMemoryRange, m_mappedRange, MappedRange)
	GET(VkDescriptorBufferInfo, m_descriptorBufferInfo, DescriptorBufferInfo)
	REF(VkDescriptorBufferInfo, m_descriptorBufferInfo, DescriptorBufferInfo)

protected:
	VkDeviceSize           m_mappingSize;          //!< Size of the memory of this uniform buffer
	VkBufferUsageFlags     m_usage;                //!< Flag with the usage of the buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT for instance for the scene transform and material uniform buffers
	VkFlags                m_requirementsMask;     //!< Enum of type VkMemoryPropertyFlagBits, to determine if the buffer is host visible, device local, or any other possivle option
	VkDeviceMemory         m_memory;               //!< Allocated buffer memory
	VkBuffer               m_buffer;               //!< Handle to the built buffer
	void*                  m_dataPointer;          //!< Alligned host memory of the buffer, to write to and the upload it to the GPU
	VkDeviceSize           m_dataSize;             //!< Size of the host memory buffer
	uint8_t*               m_mappedPointer;        //!< Host memory pointer to write buffer information
	VkMappedMemoryRange    m_mappedRange;          //!< Range of m_memory mapped
	VkDescriptorBufferInfo m_descriptorBufferInfo; //!< Struct to build a descriptor set for this buffer
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _BUFFER_H_
