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

#ifndef _CLUSTERIZATIONPREPARETECHNIQUE_H_
#define _CLUSTERIZATIONPREPARETECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class BufferPrefixSumTechnique;
class Buffer;

// NAMESPACE

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterizationPrepareCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterizationPrepareTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterizationPrepareTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterizationPrepareTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalClusterizationPrepareCompletion, m_signalClusterizationPrepareCompletion, SignalClusterizationPrepareCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	SignalClusterizationPrepareCompletion m_signalClusterizationPrepareCompletion; //!< Signal for completion of the technique
	bool                                  m_prefixSumCompleted;                    //!< Flag to know if the prefix sum step has completed
	int                                   m_voxelizationSize;                      //!< Voxelization size
	uint                                  m_numOccupiedVoxel;                      //!< Number of occupied voxel
	BufferPrefixSumTechnique*             m_bufferPrefixSumTechnique;              //!< Pointer to ths instance of the buffer prefix sum technique
	Buffer*                               m_meanCurvatureBuffer;                   //!< Buffer with the same number of elements as voxelHashedPositionCompacted. At each index i, the value stored in this buffer is the mean curvature of the voxel represented by voxelHashedPositionCompacted[i], needed for the first iteration of the clusterization process
	Buffer*                               m_meanNormalBuffer;                      //!< Buffer with the same number of elements as voxelHashedPositionCompacted. At each index i, the value stored in this buffer is the mean normal direction of the voxel represented by voxelHashedPositionCompacted[i], needed for the first iteration of the clusterization process
	Buffer*                               m_clusterizationPrepareDebugBuffer;      //!< Buffer for debug purposes
	vec3                                  m_sceneMin;                              //!< Scene min
	vec3                                  m_sceneExtent;                           //!< Scene extent
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERIZATIONPREPARETECHNIQUE_H_
