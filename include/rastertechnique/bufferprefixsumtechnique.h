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

#ifndef _BUFFERPREFIXSUMTECHNIQUE_H_
#define _BUFFERPREFIXSUMTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class Buffer;

// NAMESPACE

// DEFINES
typedef Nano::Signal<void()> SignalPrefixSumComplete;

/** Enum to control the different steps of the parallel prefix sum technique */
enum class PrefixSumStep
{
	PS_REDUCTION = 0,
	PS_SWEEPDOWN,
	PS_LAST_STEP,
	PS_FINISHED
};

/////////////////////////////////////////////////////////////////////////////////////////////

class BufferPrefixSumTechnique: public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(BufferPrefixSumTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	BufferPrefixSumTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Record command buffer
	* @param currentImage      [in] current screen image framebuffer drawing to (in case it's needed)
	* @param commandBufferID   [in] Unique identifier of the command buffer returned as parameter
	* @param commandBufferType [in] Queue to submit the command buffer recorded in the call
	* @return command buffer the technique has recorded to */
	virtual VkCommandBuffer* record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType);

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	GETCOPY(vectorUint, m_vectorPrefixSumNumElement, VectorPrefixSumNumElement)
	GET(uint, m_voxelizationWidth, VoxelizationWidth)
	GET(uint, m_firstIndexOccupiedElement, FirstIndexOccupiedElement)
	REF(SignalPrefixSumComplete, m_prefixSumComplete, PrefixSumComplete)

protected:
	/** Slot to receive notification when the scene voxelization has been performed
	* @return nothing */
	void slotVoxelizationComplete();

	/** Will retrieve from the buffer m_prefixSumPlanarBuffer, where all steps of the parallel prefix sum algorithm
	* are performed before writing the compacted buffer, the number of elements of the final compacted buffer, known once all
	* the recuction steps are performed
	* @return number of elements of the final compacted buffer */
	uint retrieveAccumulatedNumValues();

	Buffer*                 m_prefixSumPlanarBuffer;                    //!< Shader storage buffer to store all the levels of the parallel prefix sum algorithm to apply to the m_voxelFirstIndexBuffer buffer
	Buffer*                 m_voxelFirstIndexBuffer;                    //!< Shader storage buffer with the per-fragment data generated during the voxelization
	Buffer*                 m_voxelFirstIndexCompactedBuffer;           //!< Shader storage buffer with the parallel prefix sum algorithm result
	Buffer*                 m_voxelHashedPositionCompactedBuffer;       //!< Shader storage buffer with the hashed position of the 3D volume voxelization coordinates the fragment data in the same index at the m_voxelFirstIndexCompactedBuffer buffer occupied initially in the non-compacted SSBO
	Buffer*                 m_voxelFirstIndexEmitterCompactedBuffer;    //!< Shader storage buffer with the index to the first element of a linked list with all the emitters that have intersected with this voxel (for now, only the index of the emitter in the emitter buffer)
	Buffer*                 m_indirectionIndexBuffer;                   //!< Shader storage buffer for accelerating the search of elements in m_voxelFirstIndexCompactedBuffer and m_voxelHashedPositionCompactedBuffer. Contains the index to the first element in m_voxelFirstIndexCompactedBuffer and m_voxelHashedPositionCompactedBuffer for fixed 3D voxelization values, like (0,0,0), (0,0,128), (0,1,0), (0,1,128), (0,2,0), (0,2,128), ...
	Buffer*                 m_indirectionRankBuffer;                    //!< Shader storage buffer for accelerating the search of elements in m_voxelFirstIndexCompactedBuffer and m_voxelHashedPositionCompactedBuffer. Contains the amount of elements to the right in m_voxelFirstIndexCompactedBuffer for the index in m_voxelFirstIndexCompactedBuffer given at the same position at m_indirectionIndexBuffer
	Buffer*                 m_lightBounceVoxelIrradianceBuffer;         //!< Shader storage buffer with the irradiance at each major axis direction
	Buffer*                 m_lightBounceVoxelFilteredIrradianceBuffer; //!< Shader storage buffer with the filtered irradiance at each major axis direction
	Buffer*                 m_lightBounceProcessedVoxelBuffer;          //!< Shader storage buffer used ot tag whether a voxel has been processed to compute irradiance arriving at each of its six faces.
	uint                    m_firstIndexOccupiedElement;                //!< Will contain, once the reduction step of the algorithm is completed, the number of elements of the algorithm resulting compacted buffer
	uint                    m_fragmentOccupiedCounter;                  //!< Number of non-empty cells in the 3D voxelization volume
	uint                    m_voxelizationSize;                         //!< Size of the 3D volume voxelization (width * height * depth)
	bool                    m_bufferVoxelFirstIndexComplete;            //!< True when the buffer histogram has been done
	uint                    m_prefixSumPlanarBufferSize;                //!< Total buffer size for the parallel prefix sum algorithm (number of elements value)
	vectorUint              m_vectorPrefixSumNumElement;                //!< Size (number of elements) of each buffer used for the implementation of the parallel prefix sum
	uint                    m_numElementAnalyzedPreThread;              //!< Number of m_voxelFirstIndexBuffer buffer elements processed during the histogram compute dispatch per thread
	uint                    m_currentStep;                              //!< Index with the current step being done in the algorithm. At level 0, the number of elements to process is given by prefixSumNumElementBase, and from level one, by prefixSumNumElementLeveli
	uint                    m_numberStepsReduce;                        //!< Number of steps of the reduce phase of the algorithm (since the number of elements to apply prefix sum at m_voxelFirstIndexBuffer can vary from one execution to another)
	uint                    m_numberStepsDownSweep;                     //!< Number of steps of the dow sweep phase of the algorithm (since the number of elements to apply prefix sum at m_voxelFirstIndexBuffer can vary from one execution to another)
	uint                    m_currentPhase;                             //!< 0 means reduction step, 1 sweep down, 2 write final compacted buffer
	bool                    m_firstSetIsSingleElement;                  //!< Flag to know if the first set of the down sweep step is formed by a single element, step that can be avoided
	bool                    m_compactionStepDone;                       //!< Flag to know when the compaction step has been done
	uint                    m_indirectionBufferRange;                   //!< Number of elements covered by each index in the m_indirectionIndexBuffer buffer
	uint                    m_voxelizationWidth;                        //!< Width of the 3D volume voxelization
	uint                    m_voxelizationHeight;                       //!< Height of the 3D volume voxelization
	uint                    m_voxelizationDepth;                        //!< Depth of the 3D volume voxelization
	SignalPrefixSumComplete m_prefixSumComplete;                        //!< Signal to notify when the prefix sum step has been completed
	PrefixSumStep           m_currentStepEnum;                          //!< Enum to know the current step of the technique
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _BUFFERPREFIXSUMTECHNIQUE_H_
