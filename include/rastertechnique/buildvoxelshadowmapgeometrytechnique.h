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

#ifndef _BUILDVOXELSHADOWMAPGEOMETRYTECHNIQUE_H_
#define _BUILDVOXELSHADOWMAPGEOMETRYTECHNIQUE_H_

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
typedef Nano::Signal<void()> SignalBuildVoxelShadowMapGeometryCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class BuildVoxelShadowMapGeometryTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(BuildVoxelShadowMapGeometryTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	BuildVoxelShadowMapGeometryTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalBuildVoxelShadowMapGeometryCompletion, m_signalBuildVoxelShadowMapGeometryCompletion, SignalBuildVoxelShadowMapGeometryCompletion)
	GETCOPY(uint, m_numUsedVertex, NumUsedVertex)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	SignalBuildVoxelShadowMapGeometryCompletion m_signalBuildVoxelShadowMapGeometryCompletion; //!< Signal for completion of the technique
	BufferPrefixSumTechnique*                   m_techniquePrefixSum;                          //!< Pointer to the instance of the prefix sum technique
	uint                                        m_numOccupiedVoxel;                            //!< Number of occupied voxels after voxelization process
	Buffer*                                     m_shadowMapGeometryVertexBuffer;               //!< Buffer with the vertex information for the mesh built for the voxel shadow mapping technique
	Buffer*                                     m_vertexCounterBuffer;                         //!< Buffer used to counter the amount of vertices that end up used to build the geometry for the voxel shadow map
	uint                                        m_numUsedVertex;                               //!< Counter with the value from the m_vertexCounterBuffer buffer
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _BUILDVOXELSHADOWMAPGEOMETRYTECHNIQUE_H_
