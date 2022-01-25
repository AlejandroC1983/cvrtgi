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

#ifndef _CLUSTERVISIBILITYTECHNIQUE_H_
#define _CLUSTERVISIBILITYTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class BufferPrefixSumTechnique;
class ClusterizationMergeClusterTechnique;

// NAMESPACE

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterVisibilityCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterVisibilityTechnique: public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterVisibilityTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterVisibilityTechnique(string &&name, string&& className);

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

	REF(SignalClusterVisibilityCompletion, m_clusterVisibilityCompletion, SignalClusterVisibilityCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	/** Slot to receive signal when the cluster merge step has been done
	* @return nothing */
	void slotClusterMergeComplete();

	SignalClusterVisibilityCompletion    m_clusterVisibilityCompletion;       //!< Signal for completion of the technique
	BufferPrefixSumTechnique*            m_techniquePrefixSum;                //!< Pointer to the instance of the prefix sum technique
	ClusterizationMergeClusterTechnique* m_techniqueClusterMerge;             //!< Pointer to the instance of the cluster merge technique
	Buffer*                              m_clusterVisibilityBuffer;           //!< Pointer to the clusterVisibilityBuffer buffer, having for each set of m_numThreadPerLocalWorkgroup elements and for each voxel face, the indices of the visible clusters from that voxel face, in sets of m_numThreadPerLocalWorkgroup elements (this buffer is latter compacted to save memory since in many cases most of the m_numThreadPerLocalWorkgroup elements will be not used)
	Buffer*                              m_clusterVisibilityCompactedBuffer;  //!< Pointer to the clusterVisibilityCompactedBuffer buffer, being the compacted version of the clusterVisibilityCompactedBuffer buffer
	Buffer*                              m_clusterVisibilityNumberBuffer;     //!< Pointer to the clusterVisibilityNumberBuffer buffer having, for each voxel face, the amount of visible clusters for that voxel face
	Buffer*                              m_clusterVisibilityFirstIndexBuffer; //!< Pointer to the clusterVisibilityFirstIndexBuffer buffer having, for each voxel face, the index in clusterVisibilityCompactedBuffer where the visible clusters for that voxel face start
	Buffer*                              m_clusterVisibilityDebugBuffer;      //!< Pointer to the clusterVisibilityDebugBuffer for debugging purposes
	vec4                                 m_sceneMin;                          //!< Minimum value of the scene's aabb
	vec4                                 m_sceneExtent;                       //!< Scene extent
	uint                                 m_numOccupiedVoxel;                  //!< Number of occupied voxels after voxelization process
	bool                                 m_prefixSumCompleted;                //!< Flag to know if the prefix sum step has completed
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERVISIBILITYTECHNIQUE_H_
