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
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/core/coremanager.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/util/mathutil.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

UniformBuffer::UniformBuffer(string &&name) : GenericResource(move(name), move(string("UniformBuffer")), GenericResourceType::GRT_UNIFORMBUFFER)
	, m_vulkanBuffer(VK_NULL_HANDLE)
	, m_deviceMemory(VK_NULL_HANDLE)
	, m_bufferInfo({ VK_NULL_HANDLE, 0, 0 })
	, m_data(nullptr)
	, m_dynamicAllignment(0)
	, m_minCellSize(0)
	, m_bufferInstance(nullptr)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

UniformBuffer::~UniformBuffer()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void UniformBuffer::buildGPUBufer()
{
	size_t minUniformBufferObjectOffsetAlignment = coreM->getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
	m_dynamicAllignment                          = int32_t(ceil(float(m_minCellSize) / float(minUniformBufferObjectOffsetAlignment)) * float(minUniformBufferObjectOffsetAlignment));

	if (!MathUtil::isPowerOfTwo(uint(m_dynamicAllignment)))
	{
		m_dynamicAllignment = MathUtil::getNextPowerOfTwo(uint(m_dynamicAllignment));
	}

	// TODO: remove duplicated information, like m_vulkanBuffer
	m_bufferInstance    = bufferM->buildBuffer(move(string(m_name)), m_CPUBuffer.refUBHostMemory(), m_CPUBuffer.getUBHostMemorySize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	m_vulkanBuffer      = m_bufferInstance->getBuffer();
	m_bufferInfo.buffer = m_bufferInstance->getBuffer();
	m_bufferInfo.offset = 0;
	m_bufferInfo.range  = m_dynamicAllignment; //m_bufferInstance->getDataSize();
	m_deviceMemory      = m_bufferInstance->getMemory();
	m_data              = m_bufferInstance->refMappedPointer();
	m_mappedRange.push_back(m_bufferInstance->getMappedRange());
}

/////////////////////////////////////////////////////////////////////////////////////////////

void UniformBuffer::uploadCPUBufferToGPU()
{
	VkResult res = vkInvalidateMappedMemoryRanges(coreM->getLogicalDevice(), 1, &m_mappedRange[0]);
	memcpy(m_data, m_CPUBuffer.refUBHostMemory(), m_CPUBuffer.getUBHostMemorySize());
	VkResult result = vkFlushMappedMemoryRanges(coreM->getLogicalDevice(), 1, &m_bufferInstance->getMappedRange());
	assert(result == VK_SUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void UniformBuffer::destroyResources()
{
	m_vulkanBuffer      = VK_NULL_HANDLE;
	m_deviceMemory      = VK_NULL_HANDLE;
	m_bufferInfo        = {VK_NULL_HANDLE, 0, 0};
	m_data              = nullptr;
	m_dynamicAllignment = 0;
	m_bufferInstance    = nullptr;
	m_mappedRange.clear();
}

////////////////////////////////////////////////////////////////////////////////////////
