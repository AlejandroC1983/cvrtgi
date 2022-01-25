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

#ifndef _CLUSTERIZATIONMERGECLUSTERTECHNIQUE_H_
#define _CLUSTERIZATIONMERGECLUSTERTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class ClusterizationComputeNeighbourTechnique;
class MaterialClusterizationComputeNeighbour;
class MaterialClusterizationMergeClusters;

// NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterizationMergeClusterCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterizationMergeClusterTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterizationMergeClusterTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterizationMergeClusterTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called in record method after start recording command buffer, to allow any image / memory barriers
	* needed for the resources being used
	* @param commandBuffer [in] command buffer to record to
	* @return nothing */
	virtual void recordBarriers(VkCommandBuffer* commandBuffer);

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalClusterizationMergeClusterCompletion, m_signalClusterizationMergeClusterCompletion, SignalClusterizationMergeClusterCompletion)
	GETCOPY(int, m_irradianceFieldGridDensity, IrradianceFieldGridDensity)
	GETCOPY(int, m_maxIrradianceFieldOffset, MaxIrradianceFieldOffset)

protected:
	/** Slot to receive signal when the clusterization compute neighbour technique finishes
	* @return nothing */
	void slotClusterizationComputeNeighbour();

	SignalClusterizationMergeClusterCompletion m_signalClusterizationMergeClusterCompletion; //!< Signal for completion of the technique
	ClusterizationComputeNeighbourTechnique*   m_clusterizationComputeNeighbourTechnique;    //!< Pointer to ths instance of the clusterization compute neighbour technique
	MaterialClusterizationMergeClusters*       m_materialClusterizationMergeClusters;        //!< Pointer to the instance of the clusterization merge clusters material
	int                                        m_voxelizationSize;                           //!< Voxelization texture size
	Buffer*                                    m_clusterizationNeighbourDebugBuffer;         //!< Buffer for debug purposes
	Buffer*                                    m_clusterizationMergeClustersDebugBuffer;     //!< Buffer for debug purposes
	int                                        m_irradianceFieldGridDensity;                 //!< Density of the irradiance field's grid (value n means one irradiance field each n units in the voxelization volume in all dimensions)
	int                                        m_maxIrradianceFieldOffset;                   //!< Maximum offset in voxels for an irradiance field in case is in an occupied voxel and a non-occupied voxel position is trying to be found
	uint                                       m_numberIrradianceField;                      //!< Number of irradiance field generated
	ivec3                                      m_irradianceFieldMinCoordinate;               //!< Used to limit the activation of irradiance fields inside the voxelization volume, defines the minimum value (closed interval, >=) of the texture coordinates an irradiance field needs to have to be activated by the cluster AABB
	ivec3                                      m_irradianceFieldMaxCoordinate;               //!< Used to limit the activation of irradiance fields inside the voxelization volume, defines the maximum value (closed interval, <=) of the texture coordinates an irradiance field needs to have to be activated by the cluster AABB
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERIZATIONMERGECLUSTERTECHNIQUE_H_
