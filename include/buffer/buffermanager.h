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

#ifndef _BUFFERMANAGER_H_
#define _BUFFERMANAGER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/singleton.h"
#include "../../include/util/managertemplate.h"
#include "../headers.h"

// CLASS FORWARDING
class Buffer;

// NAMESPACE

// DEFINES
#define bufferM s_pBufferManager->instance()

/////////////////////////////////////////////////////////////////////////////////////////////

class BufferManager: public ManagerTemplate<Buffer>, public Singleton<BufferManager>
{
	friend class CoreManager;

public:
	/** Constructor
	* @return nothing */
	BufferManager();

	/** Destructor
	* @return nothing */
	virtual ~BufferManager();

	/** Builds a new buffer, a pointer to the buffer is returned, nullptr is returned if any errors while building it
	* @param instanceName     [in] name of the new instance (the m_sName member variable)
	* @param dataPointer      [in] void pointer to an array with the buffer data
	* @param dataSize         [in] size of the buffer data at *dataPointer
	* @param usage            [in] buffer usage
	* @param requirementsMask [in] buffer requirements
	* @return a pointer to the built texture, nullptr otherwise */
	Buffer* buildBuffer(string&& instanceName, void* dataPointer, VkDeviceSize dataSize, VkBufferUsageFlags usage, VkFlags requirementsMask);

	/** Destroys all elements in the manager
	* @return nothing */
	void destroyResources();

	/** Resizes the buffer with name guiven by instanceName, with the new size given as parameter
	* @param buffer      [in] buffer to resize
	* @param dataPointer [in] void pointer to an array with the buffer data
	* @param newSize     [in] new buffer size
	* @return true if the resizing was done properly, false otherwise */
	bool resize(Buffer* buffer, void* dataPointer, uint newSize);

	/** Prints information about each existing buffer (name and size)
	* @return nothing */
	void printWholeBufferInformation();

	/** Prints information about each existin buffer (name and size) for the buffers present in the mapData parameter
	* @param mapData [in] buffer data to print
	* @return nothing */
	void printBufferInformation(const map<string, Buffer*>& mapData);

	/** Copy size bytes from the souce buffer into the destination buffer, with an offset in source of sourceOffset
	* bytes, and an ofset in the destination of destinationOffset bytes
	* @param source            [in] source buffer to copy from
	* @param destination       [in] destination buffer to copy to
	* @param sourceOffset      [in] offset to apply to the source buffer to copy from
	* @param destinationOffset [in] offset to apply to the destination buffer to copy to
	* @param size              [in] number of bytes to copy
	* @return nothing */
	void copyBuffer(Buffer* source, Buffer* destination, int sourceOffset, int destinationOffset, int size);

protected:
	/** Builds a new VkBuffer, with size and usage given as parameter
	* @param size   [in] buffer size
	* @param usage  [in] buffer usage
	* @return a VkBuffer for the built buffer */
	VkBuffer buildBuffer(VkDeviceSize size, VkBufferUsageFlags usage);

	/** Builds memory for the buffer given as parameter, taking the memory requirements from the already built buffer
	* @param size  [in] buffer size
	* @param usage [in] buffer usage
	* @return the size of the mapped memory of the built buffer */
	VkDeviceSize buildBufferMemory(VkFlags requirementsMask, VkDeviceMemory& memory, VkBuffer& buffer);

	/** Fills the memory of a buffer, given by the memory parameter, with the data present at dataPointer, with size mappingSize,
	* the parameter of the mapped buffer memory size, mappingSize, is needed for the vkMapMemory primitive
	* @param memory      [in] buffer memory to fill
	* @param mappingSize [in] buffer mapped memory size
	* @param dataPointer [in] pointer to the data to fill the memory with
	* @param dataSize    [in] pointer to the size of the data to fill the memory with
	* @return nothing */
	void fillBufferMemory(VkDeviceMemory& memory, VkDeviceSize mappingSize, const void* dataPointer, VkDeviceSize dataSize);

	/** Builds the buffer GPU resources
	* @param buffer [in] buffer for which build the resources
	* @return nothing */
	void buildBufferResource(Buffer* buffer);
};

static BufferManager* s_pBufferManager;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _BUFFERMANAGER_H_
