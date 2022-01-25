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
#include "../../include/buffer/buffer.h"
#include "../../include/core/coremanager.h"
#include "../../include/texture/texture.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

// TODO initialize properly
Buffer::Buffer(string &&name) : GenericResource(move(name), move(string("Buffer")), GenericResourceType::GRT_BUFFER)
	, m_mappingSize(0)
	, m_usage(VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM)
	, m_requirementsMask(0)
	, m_memory(VK_NULL_HANDLE)
	, m_buffer(VK_NULL_HANDLE)
	, m_dataPointer(nullptr)
	, m_dataSize(0)
	, m_mappedPointer(0)
	, m_mappedRange({ VK_STRUCTURE_TYPE_MAX_ENUM , nullptr, VK_NULL_HANDLE , 0, 0 })
	, m_descriptorBufferInfo({ VK_NULL_HANDLE , 0, 0 })
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Buffer::destroyResources()
{
	vkDestroyBuffer(coreM->getLogicalDevice(), m_buffer, nullptr);
	vkFreeMemory(coreM->getLogicalDevice(), m_memory, nullptr);

	m_mappingSize          = 0;
	m_usage                = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
	m_requirementsMask     = 0;
	m_memory               = VK_NULL_HANDLE;
	m_buffer               = VK_NULL_HANDLE;
	m_dataPointer          = nullptr;
	m_dataSize             = 0;
	m_mappedPointer        = 0;
	m_mappedRange          = { VK_STRUCTURE_TYPE_MAX_ENUM , nullptr, VK_NULL_HANDLE, 0, 0 };
	m_descriptorBufferInfo = { VK_NULL_HANDLE, 0, 0 };
}

/////////////////////////////////////////////////////////////////////////////////////////////

Buffer::~Buffer()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Buffer::getContent(void* data)
{
	void* mappedMemory;

	VkResult result = vkMapMemory(
		coreM->getLogicalDevice(),
		m_memory,
		0,
		m_mappingSize, // VK_WHOLE_SIZE could be an option
		0,
		&mappedMemory);

	bool memoryResult = (result == VK_SUCCESS);
	assert(memoryResult);
	
	memcpy(data, mappedMemory, sizeof(uint));
	vkUnmapMemory(coreM->getLogicalDevice(), m_memory);

	return memoryResult;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Buffer::getContentCopy(vectorUint8& vectorData)
{
	void* mappedMemory;
	VkResult result = vkMapMemory(coreM->getLogicalDevice(), m_memory, 0, uint(m_dataSize), 0, &mappedMemory);
	assert(result == VK_SUCCESS);
	vectorData.resize(m_dataSize);
	memcpy((void*)vectorData.data(), mappedMemory, m_dataSize);
	vkUnmapMemory(coreM->getLogicalDevice(), m_memory);
	return (result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Buffer::setContent(const void* dataPointer)
{
	bool resultToReturn = true;

	VkCommandBuffer	commandBuffer; //!< Command buffer for vertex buffer - Triangle geometry

	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), &commandBuffer);
	coreM->beginCommandBuffer(commandBuffer);

	VkResult result = vkMapMemory(coreM->getLogicalDevice(), m_memory, 0, m_mappingSize, 0, (void **)&m_mappedPointer);
	assert(result == VK_SUCCESS);
	resultToReturn &= (result == VK_SUCCESS);

	memcpy(m_mappedPointer, dataPointer, m_dataSize);

	// Invalidate the range of mapped buffer in order to make it visible to the host. If the memory property is set with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT then the driver may take care of this, otherwise for non-coherent  mapped memory vkInvalidateMappedMemoryRanges() needs to be called explicitly.
	result = vkInvalidateMappedMemoryRanges(coreM->getLogicalDevice(), 1, &m_mappedRange);
	assert(result == VK_SUCCESS);
	resultToReturn &= (result == VK_SUCCESS);

	vkUnmapMemory(coreM->getLogicalDevice(), m_memory);

	coreM->endCommandBuffer(commandBuffer);
	coreM->submitCommandBuffer(coreM->getLogicalDeviceGraphicsQueue(), &commandBuffer);
	VkCommandBuffer cmdBufs[] = { commandBuffer };
	vkFreeCommandBuffers(coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), sizeof(cmdBufs) / sizeof(VkCommandBuffer), cmdBufs);

	return resultToReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
