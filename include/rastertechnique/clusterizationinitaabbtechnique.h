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

#ifndef _CLUSTERIZATIONINITAABBTECHNIQUE_H_
#define _CLUSTERIZATIONINITAABBTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class BufferPrefixSumTechnique;
class Buffer;
class MaterialClusterizationInitAABB;

// NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////

/** Struct where to store the clusterization result and related cluster information, 260 4-byte elements
* Flags stored in w field:
* minAABB.w:         Cluster index in clusterizationFinalBuffer
* maxAABB.w:         Cluster num neighbours
* centerAABB.w:      Number of occupied voxels for this cluster within the AABB.
* meanReflectance.w: Number of total fragments in all the voxels in the cluster */
struct ClusterData
{
	ivec4 minAABB;                 //!< Cluster min AABB texture space coordinates in xyz fields. Cluster index in w field
	ivec4 maxAABB;                 //!< Cluster max AABB texture space coordinates in xyz fields. Cluster num neighbours in w field.
	ivec4 centerAABB;              //!< Cluster center AABB texture space coordinates in xyz fields. Number of occupied voxels for this cluster within the AABB in w field.
	vec4  mainDirection;           //!< Cluster main direction (voxels used for clusterization have one of the main six directions, +x, -x, +y, -y, +z, -z).
	vec4  meanReflectance;         //!< Mean reflectance value for all the fragments of all the voxels of the cluster. The w field stores the number of total fragments
	uvec4 clusterIrradiance;       //!< Irradiance for all the lit voxels of the cluster
	int   arrayNeighbourIndex[64]; //!< Indices of the cluster neighbours in the same buffer were this struct is stored
	uint  arrayVoxels[256];        //!< Array with the index in voxelHashedPositionCompacted of each voxel owned by this cluster (i.e. with the result of calling findHashedCompactedPositionIndex)
};

/////////////////////////////////////////////////////////////////////////////////////////////

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalClusterizationInitAABBCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterizationInitAABBTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterizationInitAABBTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterizationInitAABBTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalClusterizationInitAABBCompletion, m_signalClusterizationInitAABBCompletion, SignalClusterizationInitAABBCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	SignalClusterizationInitAABBCompletion m_signalClusterizationInitAABBCompletion; //!< Signal for completion of the technique
	bool                                   m_prefixSumCompleted;                     //!< Flag to know if the prefix sum step has completed
	BufferPrefixSumTechnique*              m_bufferPrefixSumTechnique;               //!< Pointer to ths instance of the buffer prefix sum technique
	Buffer*                                m_clusterizationBuffer;                   //!< Buffer with the resultas of the clusterization
	MaterialClusterizationInitAABB*        m_materialClusterizationInitAABB;         //!< Pointer to the cluster AABB init material instance
	int                                    m_clusterNumber;                          //!< Number of clusters to consider (depends on the resolution of the voxelization texture)
	int                                    m_voxelizationSize;                       //!< Voxelization texture size
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERIZATIONINITAABBTECHNIQUE_H_
