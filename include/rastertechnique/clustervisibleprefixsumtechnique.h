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

#ifndef _CLUSTERVISIBLEPREFIXSUMTECHNIQUE_H_
#define _CLUSTERVISIBLEPREFIXSUMTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/rastertechnique.h"

// CLASS FORWARDING
class Buffer;
class ClusterVisibilityTechnique;

// NAMESPACE

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterVisiblePrefixSumTechnique;

/** Enum to control the different steps of the parallel prefix sum technique */
// TODO: Put in helper class and reuse together with BufferPrefixSumTechnique
enum class PrefixSumStep_
{
	PS_REDUCTION = 0,
	PS_SWEEPDOWN,
	PS_LAST_STEP,
	PS_FINISHED
};

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterVisiblePrefixSumTechnique : public RasterTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterVisiblePrefixSumTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterVisiblePrefixSumTechnique(string &&name, string&& className);

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

	REF(SignalClusterVisiblePrefixSumTechnique, m_signalClusterVisiblePrefixSumTechnique, SignalClusterVisiblePrefixSumTechnique)

protected:
	/** Slot to receive signal when the cluster visibility technique finishes
	* @return nothing */
	void slotClusterVisibility();

	/** Will retrieve from the buffer m_prefixSumPlanarBuffer, where all steps of the parallel prefix sum algorithm
	* are performed before writing the compacted buffer, the number of elements of the final compacted buffer, known once all
	* the recuction steps are performed
	* @return number of elements of the final compacted buffer */
	uint retrieveAccumulatedNumValues();

	SignalClusterVisiblePrefixSumTechnique m_signalClusterVisiblePrefixSumTechnique; //!< Signal for completion of the technique
	Buffer*                                m_clusterVisibilityBuffer;                //!< Pointer to the clusterVisibilityBuffer buffer, having for each set of m_numThreadPerLocalWorkgroup elements and for each voxel face, the indices of the visible clusters from that voxel face, in sets of m_numThreadPerLocalWorkgroup elements (this buffer is latter compacted to save memory since in many cases most of the m_numThreadPerLocalWorkgroup elements will be not used)
	Buffer*                                m_clusterVisibilityCompactedBuffer;       //!< Pointer to the clusterVisibilityCompactedBuffer buffer, being the compacted version of the clusterVisibilityCompactedBuffer buffer
	Buffer*                                m_clusterVisibilityFirstIndexBuffer;      //!< Pointer to the clusterVisibilityFirstIndexBuffer buffer having, for each voxel face, the index in clusterVisibilityCompactedBuffer where the visible clusters for that voxel face start
	Buffer*                                m_prefixSumBuffer;                        //!< Pointer to the planar buffer used to store the whole prefix sum steps (reduction and sweepdown)
	ClusterVisibilityTechnique*            m_clusterVisibilityTechnique;             //!< Pointer to the instance of the cluster visibility technique
	vectorUint                             m_vectorPrefixSumNumElement;              //!< Size (number of elements) of each buffer used for the implementation of the parallel prefix sum
	uint                                   m_prefixSumPlanarBufferSize;              //!< Total buffer size for the parallel prefix sum algorithm (number of elements value)
	uint                                   m_numberStepsReduce;                      //!< Number of steps of the reduce phase of the algorithm (since the number of elements to apply prefix sum at m_voxelFirstIndexBuffer can vary from one execution to another)
	bool                                   m_firstSetIsSingleElement;                //!< Flag to know if the first set of the down sweep step is formed by a single element, step that can be avoided
	uint                                   m_numberStepsDownSweep;                   //!< Number of steps of the dow sweep phase of the algorithm (since the number of elements to apply prefix sum at m_voxelFirstIndexBuffer can vary from one execution to another)
	uint                                   m_currentStep;                            //!< Index with the current step being done in the algorithm. At level 0, the number of elements to process is given by prefixSumNumElementBase, and from level one, by prefixSumNumElementLeveli
	uint                                   m_currentPhase;                           //!< 0 means reduction step, 1 sweep down, 2 write final compacted buffer
	bool                                   m_compactionStepDone;                     //!< Flag to know when the compaction step has been done
	uint                                   m_firstIndexOccupiedElement;              //!< Will contain, once the reduction step of the algorithm is completed, the number of elements of the algorithm resulting compacted buffer
	PrefixSumStep_                         m_currentStepEnum;                        //!< Enum to know the current step of the technique
	uint                                   m_numElementAnalyzedPerThread;            //!< Number of m_voxelFirstIndexBuffer buffer elements processed during the histogram compute dispatch per thread
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERVISIBLEPREFIXSUMTECHNIQUE_H_
