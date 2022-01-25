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

#ifndef _CLUSTERIZATIONCOMPUTENEIGHBOURTECHNIQUE_H_
#define _CLUSTERIZATIONCOMPUTENEIGHBOURTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class ClusterizationBuildFinalBufferTechnique;
class MaterialClusterizationComputeNeighbour;
class MaterialClusterizationMergeClusters;

// NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterizationComputeNeighbourCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterizationComputeNeighbourTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterizationComputeNeighbourTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterizationComputeNeighbourTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	/** Called in record method after start recording command buffer, to allow any image / memory barriers
	* needed for the resources being used
	* @param commandBuffer [in] command buffer to record to
	* @return nothing */
	virtual void recordBarriers(VkCommandBuffer* commandBuffer);

	REF(SignalClusterizationComputeNeighbourCompletion, m_signalClusterizationComputeNeighbourCompletion, SignalClusterizationComputeNeighbourCompletion)

protected:
	/** Slot to receive signal when the clusterization build final buffer technique finishes
	* @return nothing */
	void slotClusterizationBuildFinalBuffer();

	SignalClusterizationComputeNeighbourCompletion m_signalClusterizationComputeNeighbourCompletion; //!< Signal for completion of the technique
	ClusterizationBuildFinalBufferTechnique*       m_clusterizationBuildFinalBufferTechnique;        //!< Pointer to ths instance of the clusterization final buffer technique
	MaterialClusterizationComputeNeighbour*        m_materialClusterizationComputeNeighbour;         //!< Pointer to the instance of the clusterization compute neighbour material
	int                                            m_voxelizationSize;                               //!< Voxelization texture size
	Buffer*                                        m_clusterizationNeighbourDebugBuffer;             //!< Buffer for debug purposes
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERIZATIONCOMPUTENEIGHBOURTECHNIQUE_H_
