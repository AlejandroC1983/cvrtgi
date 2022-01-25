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

#ifndef _CLUSTERIZATIONCOMPUTEAABBTECHNIQUE_H_
#define _CLUSTERIZATIONCOMPUTEAABBTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class BufferPrefixSumTechnique;
class MaterialClusterizationComputeAABB;
class Buffer;

// NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterizationComputeAABBCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterizationComputeAABBTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterizationComputeAABBTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterizationComputeAABBTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalClusterizationComputeAABBCompletion, m_signalClusterizationComputeAABBCompletion, SignalClusterizationComputeAABBCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	SignalClusterizationComputeAABBCompletion m_signalClusterizationComputeAABBCompletion; //!< Signal for completion of the technique
	bool                                      m_prefixSumCompleted;                        //!< Flag to know if the prefix sum step has completed
	BufferPrefixSumTechnique*                 m_bufferPrefixSumTechnique;                  //!< Pointer to ths instance of the buffer prefix sum technique
	int                                       m_occupiedVoxelNumber;                       //!< Number of occupied voxels
	int                                       m_voxelizationSize;                          //!< Voxelization texture size
	MaterialClusterizationComputeAABB*        m_materialClusterizationComputeAABB;         //!< Pointer to the instance of the material clusterization compute AABB
	Buffer*                                   m_clusterAABBDebugBuffer;                    //!< Buffer for debug purposes
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERIZATIONCOMPUTEAABBTECHNIQUE_H_
