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
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/core/coremanager.h"
#include "../../include/core/physicaldevice.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/util/vulkanstructinitializer.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

BufferManager::BufferManager()
{
	m_managerName = g_bufferManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////

BufferManager::~BufferManager()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferManager::destroyResources()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

Buffer* BufferManager::buildBuffer(string&& instanceName, void* dataPointer, VkDeviceSize dataSize, VkBufferUsageFlags usage, VkFlags requirementsMask)
{
	if (existsElement(move(string(instanceName))))
	{
		return getElement(move(instanceName));
	}

	Buffer* buffer = new Buffer(move(string(instanceName)));

	buffer->m_dataSize         = dataSize;
	buffer->m_dataPointer      = dataPointer;
	buffer->m_usage            = usage;
	buffer->m_requirementsMask = requirementsMask;

	buildBufferResource(buffer);

	buffer->m_descriptorBufferInfo.buffer = buffer->m_buffer;
	buffer->m_descriptorBufferInfo.offset = 0;
	buffer->m_descriptorBufferInfo.range  = buffer->m_dataSize;

	addElement(move(string(instanceName)), buffer);
	buffer->m_name = move(instanceName);
	buffer->m_ready = true;

	return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool BufferManager::resize(Buffer* buffer, void* dataPointer, uint newSize)
{
	VkBufferUsageFlags usage = buffer->m_usage;
	VkFlags requirementsMask = buffer->m_requirementsMask;

	buffer->m_ready = false;
	buffer->destroyResources();

	buffer->m_dataSize         = newSize;
	buffer->m_dataPointer      = dataPointer;
	buffer->m_usage            = usage;
	buffer->m_requirementsMask = requirementsMask;

	buildBufferResource(buffer);

	buffer->m_descriptorBufferInfo.buffer = buffer->m_buffer;
	buffer->m_descriptorBufferInfo.offset = 0;
	buffer->m_descriptorBufferInfo.range  = buffer->m_dataSize;

	buffer->m_ready = true;

	if (gpuPipelineM->getPipelineInitialized())
	{
		emitSignalElement(move(string(buffer->m_name)), ManagerNotificationType::MNT_CHANGED);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferManager::printWholeBufferInformation()
{
	printBufferInformation(m_mapElement);

	cout << endl;
	cout << endl;
	cout << "------------------------------------------------------------------------------" << endl;
	cout << "Memory used data" << endl;

	// TODO: Remove this part when possible
	map<string, Buffer*> mapMemory {
		{"IndirectionIndexBuffer",                              m_mapElement.find("IndirectionIndexBuffer")->second },
		{"IndirectionRankBuffer",                               m_mapElement.find("IndirectionRankBuffer")->second },
		{"cameraVisibleVoxelBuffer",                            m_mapElement.find("cameraVisibleVoxelBuffer")->second },
		{"cameraVisibleVoxelCompactedBuffer",                   m_mapElement.find("cameraVisibleVoxelCompactedBuffer")->second },
		{"closerVoxelVisibilityFacePenaltyBuffer",              m_mapElement.find("closerVoxelVisibilityFacePenaltyBuffer")->second },
		{"clusterCounterBuffer",                                m_mapElement.find("clusterCounterBuffer")->second },
		{"clusterVisibilityBuffer",                             m_mapElement.find("clusterVisibilityBuffer")->second },
		{"clusterVisibilityCompactedBuffer",                    m_mapElement.find("clusterVisibilityCompactedBuffer")->second },
		{"clusterVisibilityFacePenaltyBuffer",                  m_mapElement.find("clusterVisibilityFacePenaltyBuffer")->second },
		{"clusterVisibilityFirstIndexBuffer",                   m_mapElement.find("clusterVisibilityFirstIndexBuffer")->second },
		{"clusterVisibilityNumberBuffer",                       m_mapElement.find("clusterVisibilityNumberBuffer")->second },
		{"clusterizationCenterCoordinatesBuffer",               m_mapElement.find("clusterizationCenterCoordinatesBuffer")->second },
		{"clusterizationCenterCountsBuffer",                    m_mapElement.find("clusterizationCenterCountsBuffer")->second },
		{"clusterizationCenterNegativeDirectionBuffer",         m_mapElement.find("clusterizationCenterNegativeDirectionBuffer")->second },
		{"clusterizationCenterNegativePreviousDirectionBuffer", m_mapElement.find("clusterizationCenterNegativePreviousDirectionBuffer")->second },
		{"clusterizationCenterPositiveDirectionBuffer",         m_mapElement.find("clusterizationCenterPositiveDirectionBuffer")->second },
		{"clusterizationCenterPositivePreviousDirectionBuffer", m_mapElement.find("clusterizationCenterPositivePreviousDirectionBuffer")->second },
		{"clusterizationFinalBuffer",                           m_mapElement.find("clusterizationFinalBuffer")->second },
		{"fragmentCounterBuffer",                               m_mapElement.find("fragmentCounterBuffer")->second },
		{"fragmentDataBuffer",                                  m_mapElement.find("fragmentDataBuffer")->second },
		{"fragmentOccupiedCounterBuffer",                       m_mapElement.find("fragmentOccupiedCounterBuffer")->second },
		{"frustumElementCounterEmitterCameraBuffer",            m_mapElement.find("frustumElementCounterEmitterCameraBuffer")->second },
		{"frustumElementCounterMainCameraBuffer",               m_mapElement.find("frustumElementCounterMainCameraBuffer")->second },
		{"indirectCommandBufferEmitterCamera",                  m_mapElement.find("indirectCommandBufferEmitterCamera")->second },
		{"indirectCommandBufferMainCamera",                     m_mapElement.find("indirectCommandBufferMainCamera")->second },
		{"instanceDataBuffer",                                  m_mapElement.find("instanceDataBuffer")->second },
		{"iterationCounterBuffer",                              m_mapElement.find("iterationCounterBuffer")->second },
		{"lightBounceIndirectLitCounterBuffer",                 m_mapElement.find("lightBounceIndirectLitCounterBuffer")->second },
		{"lightBounceIndirectLitIndexBuffer",                   m_mapElement.find("lightBounceIndirectLitIndexBuffer")->second },
		{"lightBounceVoxelFilteredIrradianceBuffer",            m_mapElement.find("lightBounceVoxelFilteredIrradianceBuffer")->second },
		{"lightBounceVoxelIrradianceBuffer",                    m_mapElement.find("lightBounceVoxelIrradianceBuffer")->second },
		{"litClusterCounterBuffer",                             m_mapElement.find("litClusterCounterBuffer")->second },
		{"litTestClusterBuffer",                                m_mapElement.find("litTestClusterBuffer")->second },
		{"litTestVoxelBuffer",                                  m_mapElement.find("litTestVoxelBuffer")->second },
		{"litToRasterVisibleClusterBuffer",                     m_mapElement.find("litToRasterVisibleClusterBuffer")->second },
		{"litToRasterVisibleClusterCounterBuffer",              m_mapElement.find("litToRasterVisibleClusterCounterBuffer")->second },
		{"litToRasterVisibleIndexClusterBuffer",                m_mapElement.find("litToRasterVisibleIndexClusterBuffer")->second },
		{"litVisibleClusterBuffer",                             m_mapElement.find("litVisibleClusterBuffer")->second },
		{"litVisibleIndexClusterBuffer",                        m_mapElement.find("litVisibleIndexClusterBuffer")->second },
		{"meanNormalBuffer",                                    m_mapElement.find("meanNormalBuffer")->second },
		{"nextFragmentIndexBuffer",                             m_mapElement.find("nextFragmentIndexBuffer")->second },
		{"voxelClusterOwnerDistanceBuffer",                     m_mapElement.find("voxelClusterOwnerDistanceBuffer")->second },
		{"voxelClusterOwnerIndexBuffer",                        m_mapElement.find("voxelClusterOwnerIndexBuffer")->second },
		{"voxelFirstIndexBuffer",                               m_mapElement.find("voxelFirstIndexBuffer")->second },
		{"voxelFirstIndexCompactedBuffer",                      m_mapElement.find("voxelFirstIndexCompactedBuffer")->second },
		{"voxelFirstIndexEmitterCompactedBuffer",               m_mapElement.find("voxelFirstIndexEmitterCompactedBuffer")->second },
		{"voxelHashedPositionCompactedBuffer",                  m_mapElement.find("voxelHashedPositionCompactedBuffer")->second },
		{"voxelOccupiedBuffer",                                 m_mapElement.find("voxelOccupiedBuffer")->second },
		{"voxelNeighbourIndexBuffer",                           m_mapElement.find("voxelNeighbourIndexBuffer")->second },
	};

	printBufferInformation(mapMemory);

	cout << endl;
	cout << endl;
	cout << "------------------------------------------------------------------------------" << endl;
	cout << "Final memory data" << endl;

	// TODO: Remove this part when possible
	map<string, Buffer*> mapFinal {
		{"litTestClusterBuffer",                     m_mapElement.find("litTestClusterBuffer")->second},
		{"litClusterCounterBuffer",                  m_mapElement.find("litClusterCounterBuffer")->second},
		{"litVisibleClusterBuffer",                  m_mapElement.find("litVisibleClusterBuffer")->second},
		{"litToRasterVisibleClusterCounterBuffer",   m_mapElement.find("litToRasterVisibleClusterCounterBuffer")->second},
		{"litToRasterVisibleClusterBuffer",          m_mapElement.find("litToRasterVisibleClusterBuffer")->second},
		{"litToRasterVisibleIndexClusterBuffer",     m_mapElement.find("litToRasterVisibleIndexClusterBuffer")->second},
		{"alreadyRasterizedClusterBuffer",           m_mapElement.find("alreadyRasterizedClusterBuffer")->second},
		{"litVisibleIndexClusterBuffer",             m_mapElement.find("litVisibleIndexClusterBuffer")->second},
		{"clusterizationFinalBuffer",                m_mapElement.find("clusterizationFinalBuffer")->second},
		{"IndirectionIndexBuffer",                   m_mapElement.find("IndirectionIndexBuffer")->second},
		{"IndirectionRankBuffer",                    m_mapElement.find("IndirectionRankBuffer")->second},
		{"voxelHashedPositionCompactedBuffer",       m_mapElement.find("voxelHashedPositionCompactedBuffer")->second},
		{"voxelClusterOwnerIndexBuffer",             m_mapElement.find("voxelClusterOwnerIndexBuffer")->second},
		{"voxelOccupiedBuffer",                      m_mapElement.find("voxelOccupiedBuffer")->second},
		{"lightBounceVoxelIrradianceBuffer",         m_mapElement.find("lightBounceVoxelIrradianceBuffer")->second},
		{"cameraVisibleVoxelBuffer",                 m_mapElement.find("cameraVisibleVoxelBuffer")->second},
		{"cameraVisibleVoxelCompactedBuffer",        m_mapElement.find("cameraVisibleVoxelCompactedBuffer")->second},
		{"cameraVisibleCounterBuffer",               m_mapElement.find("cameraVisibleCounterBuffer")->second},
		{"lightBounceVoxelFilteredIrradianceBuffer", m_mapElement.find("lightBounceVoxelFilteredIrradianceBuffer")->second},
		{"clusterVisibilityNumberBuffer",            m_mapElement.find("clusterVisibilityNumberBuffer")->second},
		{"clusterVisibilityFirstIndexBuffer",        m_mapElement.find("clusterVisibilityFirstIndexBuffer")->second},
		{"clusterVisibilityCompactedBuffer",         m_mapElement.find("clusterVisibilityCompactedBuffer")->second},
		{"closerVoxelVisibilityFacePenaltyBuffer",   m_mapElement.find("closerVoxelVisibilityFacePenaltyBuffer")->second},
		{"clusterVisibilityFacePenaltyBuffer, ",     m_mapElement.find("clusterVisibilityFacePenaltyBuffer")->second},
		{"voxelNeighbourIndexBuffer",                m_mapElement.find("voxelNeighbourIndexBuffer")->second },
	};

	printBufferInformation(mapFinal);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferManager::printBufferInformation(const map<string, Buffer*>& mapData)
{
	int longestName = 0;

	forIT(mapData)
	{
		longestName = max(longestName, int(it->first.size()));
	}

	longestName += 5;
	int sizeByteOffset = longestName + 20;
	int sizeKBOffset = sizeByteOffset + 20;

	int i;
	string line;
	int mappingSize;
	uint totalSize = 0;
	int currentSize;

	line = "Buffer name";
	currentSize = int(line.size());

	for (i = currentSize; i < longestName; ++i)
	{
		line += " ";
	}

	line += "Size(bytes)";
	currentSize = int(line.size());

	for (i = currentSize; i < sizeByteOffset; ++i)
	{
		line += " ";
	}
	line += "Size(KB)";
	currentSize = int(line.size());

	for (i = currentSize; i < sizeKBOffset; ++i)
	{
		line += " ";
	}

	line += "Size(MB)";
	currentSize = int(line.size());

	string lineTemp;
	for (i = 0; i < (currentSize + 5); ++i)
	{
		lineTemp += "-";
	}

	cout << endl;
	cout << line << endl;
	cout << lineTemp << endl;

	forIT(mapData)
	{
		line = it->first;
		currentSize = int(it->first.size());

		for (i = currentSize; i < longestName; ++i)
		{
			line += " ";
		}

		mappingSize = int(it->second->getDataSize());
		totalSize += uint(mappingSize);
		line += to_string(mappingSize);
		currentSize = int(line.size());

		for (i = currentSize; i < sizeByteOffset; ++i)
		{
			line += " ";
		}

		line += to_string(float(mappingSize) / 1024.0f);
		currentSize = int(line.size());

		for (i = currentSize; i < sizeKBOffset; ++i)
		{
			line += " ";
		}

		line += to_string(float(mappingSize) / (1024.0f * 1024.0f));
		currentSize = int(line.size());

		cout << line << endl;
	}

	line = "Total";
	currentSize = 5;

	for (i = currentSize; i < longestName; ++i)
	{
		line += " ";
	}

	line += to_string(totalSize);
	currentSize = int(line.size());

	for (i = currentSize; i < sizeByteOffset; ++i)
	{
		line += " ";
	}

	line += to_string(float(totalSize) / 1024.0f);
	currentSize = int(line.size());

	for (i = currentSize; i < sizeKBOffset; ++i)
	{
		line += " ";
	}

	line += to_string(float(totalSize) / (1024.0f * 1024.0f));
	currentSize = int(line.size());

	cout << line << endl;

	cout << lineTemp << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferManager::copyBuffer(Buffer* source, Buffer* destination, int sourceOffset, int destinationOffset, int size)
{
	VkCommandBuffer commandBuffer;
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), &commandBuffer);
	coreM->beginCommandBuffer(commandBuffer);

	VkBufferCopy vertexCopyRegion = { static_cast<VkDeviceSize>(sourceOffset), static_cast<VkDeviceSize>(destinationOffset), static_cast<VkDeviceSize>(size) };
	vkCmdCopyBuffer(commandBuffer, source->getBuffer(), destination->getBuffer(), 1, &vertexCopyRegion);

	VulkanStructInitializer::insertBufferMemoryBarrier(destination,
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_PIPELINE_STAGE_TRANSFER_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   &commandBuffer);

	coreM->endCommandBuffer(commandBuffer);

	coreM->submitCommandBuffer(coreM->getLogicalDeviceGraphicsQueue(), &commandBuffer);

	vkFreeCommandBuffers(coreM->getLogicalDevice(), coreM->getGraphicsCommandPool(), 1, &commandBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkBuffer BufferManager::buildBuffer(VkDeviceSize size, VkBufferUsageFlags usage)
{
	// Create a staging buffer resource states using.
	// Indicate it be the source of the transfer command.
	// .usage	= VK_BUFFER_USAGE_TRANSFER_SRC_BIT
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext                 = NULL;
	bufferCreateInfo.size                  = size;
	bufferCreateInfo.usage                 = usage;
	bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices   = NULL;
	bufferCreateInfo.flags                 = 0;

	// Create a buffer resource (host-visible) -
	VkBuffer buffer;
	VkResult error = vkCreateBuffer(coreM->getLogicalDevice(), &bufferCreateInfo, NULL, &buffer);
	assert(!error);

	return buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkDeviceSize BufferManager::buildBufferMemory(VkFlags requirementsMask, VkDeviceMemory& memory, VkBuffer& buffer)
{
	VkMemoryRequirements memRqrmnt;
	vkGetBufferMemoryRequirements(coreM->getLogicalDevice(), buffer, &memRqrmnt);

	if (memRqrmnt.size == 0)
	{
		return 0;
	}

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext           = NULL;
	memAllocInfo.memoryTypeIndex = 0;
	memAllocInfo.allocationSize  = memRqrmnt.size; // is equal to sizeof(MVP)???

	bool memoryTypeResult = coreM->memoryTypeFromProperties(memRqrmnt.memoryTypeBits, requirementsMask, coreM->getPhysicalDeviceMemoryProperties().memoryTypes, memAllocInfo.memoryTypeIndex);
	assert(memoryTypeResult);

	VkResult result = vkAllocateMemory(coreM->getLogicalDevice(), &memAllocInfo, NULL, &(memory));
	assert(result == VK_SUCCESS);

	result = vkBindBufferMemory(coreM->getLogicalDevice(), buffer, memory, 0);
	assert(result == VK_SUCCESS);

	return memRqrmnt.size;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferManager::fillBufferMemory(VkDeviceMemory& memory, VkDeviceSize mappingSize, const void* dataPointer, VkDeviceSize dataSize)
{
	uint8_t* mappedPointer;
	VkResult result = vkMapMemory(coreM->getLogicalDevice(), memory, 0, mappingSize, 0, (void **)&mappedPointer);
	assert(result == VK_SUCCESS);

	memcpy(mappedPointer, dataPointer, dataSize);

	vkUnmapMemory(coreM->getLogicalDevice(), memory);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferManager::buildBufferResource(Buffer* buffer)
{
	buffer->m_buffer      = buildBuffer(buffer->m_dataSize, buffer->m_usage);
	buffer->m_mappingSize = buildBufferMemory(buffer->m_requirementsMask, buffer->m_memory, buffer->m_buffer);

	VkMappedMemoryRange mappedRange;
	mappedRange.sType     = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory    = buffer->m_memory;
	mappedRange.offset    = 0;
	mappedRange.size      = VK_WHOLE_SIZE;
	mappedRange.pNext     = nullptr;
	buffer->m_mappedRange = mappedRange;

	if (buffer->m_dataPointer != nullptr)
	{
		buffer->setContent(buffer->m_dataPointer);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
