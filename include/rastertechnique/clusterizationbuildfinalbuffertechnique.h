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

#ifndef _CLUSTERIZATIONBUILDFINALBUFFERTECHNIQUE_H_
#define _CLUSTERIZATIONBUILDFINALBUFFERTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class BufferPrefixSumTechnique;
class MaterialClusterizationBuildFinalBuffer;

// NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterizationBuildFinalBufferCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterizationBuildFinalBufferTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterizationBuildFinalBufferTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterizationBuildFinalBufferTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalClusterizationBuildFinalBufferCompletion, m_signalClusterizationBuildFinalBufferCompletion, SignalClusterizationBuildFinalBufferCompletion)
	GETCOPY(uint, m_compactedClusterNumber, CompactedClusterNumber)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	SignalClusterizationBuildFinalBufferCompletion m_signalClusterizationBuildFinalBufferCompletion; //!< Signal for completion of the technique
	bool                                           m_prefixSumCompleted;                             //!< Flag to know if the prefix sum step has completed
	BufferPrefixSumTechnique*                      m_bufferPrefixSumTechnique;                       //!< Pointer to ths instance of the buffer prefix sum technique
	int                                            m_clusterNumber;                                  //!< Number of clusters to consider (depends on the resolution of the voxelization texture)
	int                                            m_voxelizationSize;                               //!< Voxelization texture size
	MaterialClusterizationBuildFinalBuffer*        m_materialClusterizationBuildFinalBuffer;         //!< Pointer to the instance of the clusterization build final buffer material
	Buffer*                                        m_clusterCounterBuffer;                           //!< Buffer used as atomic counter to count the number of occupied clusters in the initial buffer where all clusters are considered, clusterizationBuffer
	Buffer*                                        m_clusterizationFinalBuffer;                      //!< Buffer with the final, occupied clusters, used for all the computations. This buffer is the compacted version of clusterizationBuffer buffer (not sorted, since clusters don't need to be sorted for the current implementation)
	Buffer*                                        m_clusterizationDebugFinalBuffer;                 //!< Buffer for debug purposes
	uint                                           m_compactedClusterNumber;                         //!< Number of elements in clusterFinalBuffer buffer, meaning the number of occupied clusters
	uint                                           m_clusterCounter;                                 //!< Variable used to get and set the m_clusterCounterBuffer buffer value
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERIZATIONBUILDFINALBUFFERTECHNIQUE_H_
