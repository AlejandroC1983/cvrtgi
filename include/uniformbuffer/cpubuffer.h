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

#ifndef _CPUBUFFER_H_
#define _CPUBUFFER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/genericresource.h"

// CLASS FORWARDING
class Buffer;

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class CPUBuffer : public GenericResource
{
public:
	/** Default constructor
	* @return nothing */
	CPUBuffer();

	/** Destructor
	* @return nothing */
	virtual ~CPUBuffer();

	/** Builds a buffer of alligned memory in CPU RAM to store all the information that will be sent to hardware, the
	*  result is set in m_UBHostMemory. Also, the member variables m_dynamicAllignment and m_UBHostMemorySize are updated.
	* @return nothing */
	void buildCPUBuffer();

	/** Builds a buffer of alligned memory in CPU RAM to store all the information that will be sent to hardware, the
	*  result is set in m_UBHostMemory. The values set in m_arrayCellStartPointer, m_arrayCellCurrentPointer, m_arrayCellOffset
	* will match the layout of the data in vecUniformBase (no cell-based approach like the one executed in the no parameter
	* buildCPUBuffer() method)
	* @param vecUniformBase [in] vector with the uniform values that will be mapped by this CPU RAM buffer
	* @return nothing */
	void buildCPUBuffer(const vectorUniformBasePtr& vecUniformBase);

	/** Wrapper function for aligned memory allocation, there is currently no standard for this in C++ that works
	* across all platforms and vendors, so this is made abstract
	* @param size      [in] Size in bytes to make the allocation
	* @param alignment [in] Size in bytes of the alignment of the memory to allocate
	* @return void pointer to the allocated aligned memory */
	void* alignedMemoryAllocation(size_t size, size_t alignment);

	/** Dealocates the aligned allocated memory given as parameter
	* @param data [in] Pointer to the memory to deallocate
	* @return nothing */
	void alignedMemoryFree(void* data);

	/** Destroy the resources allocated by the buffer and initializes the member variables
	* @return nothing */
	void destroyResources();

	/** Appends the data given as parameter to the end of the current data at the cell given by index
	* @param index [in] Index of the data to append
	* @param data  [in] Data to copy
	* @return true if there wasn't any problem appending the data, false otherwise (no enough room for the data) */
	template <class T> bool appendDataAtCell(int index, T data)
	{
		if (getAvailableRoomAtCell(index) < 0)
		{
			return false;
		}

		memcpy(m_arrayCellCurrentPointer[index], &data, sizeof(T));
		byte* p = (byte*)m_arrayCellCurrentPointer[index];
		p += sizeof(T);
		m_arrayCellCurrentPointer[index] = p;
		m_arrayCellOffset[index] += sizeof(T);
		return true;
	}

	/** Resets the data present at cell given by index parameter
	* @return nothing */
	void resetDataAtCell(int index);

	/** Returns the available space in bytes at cell given by index parameter
	* @param index [in] Index of the data to append
	* @return the available space in bytes at cell given by index parameter */
	int getAvailableRoomAtCell(int index);

	GETCOPY_SET(int, m_minCellSize, MinCellSize)
	GETCOPY_SET(int, m_numCells, NumCells)
	REF_PTR(void, m_UBHostMemory, UBHostMemory)
	GETCOPY(size_t, m_UBHostMemorySize, UBHostMemorySize)

protected:
	size_t        m_dynamicAllignment;       //!< Real cell size used to place elements
	void*         m_UBHostMemory;            //!< Pointer to the CPU aligned buffer memory
	size_t        m_UBHostMemorySize;        //!< Size in bytes of the buffer given by m_UBHostMemory
	int           m_minCellSize;             //!< Minimum cell size requested when building the buffer
	int           m_numCells;                //!< Number of cells of the buffer, each one of size
	vectorVoidPtr m_arrayCellStartPointer;   //!< Array to the starting byte of each cell in the m_UBHostMemory buffer
	vectorVoidPtr m_arrayCellCurrentPointer; //!< Array to the first available byte of each cell in the m_UBHostMemory buffer
	vectorInt     m_arrayCellOffset;         //!< Array of byte offsets inside each cell
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CPUBUFFER_H_
