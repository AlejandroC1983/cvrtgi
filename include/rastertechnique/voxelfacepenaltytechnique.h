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

#ifndef _VOXELFACEPENALTYTECHNIQUE_H_
#define _VOXELFACEPENALTYTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class BufferPrefixSumTechnique;
class ClusterVisiblePrefixSumTechnique;

// NAMESPACE

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalVoxelFacePenaltyTechniqueCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class VoxelFacePenaltyTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(VoxelFacePenaltyTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	VoxelFacePenaltyTechnique(string &&name, string&& className);

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

	REF(SignalVoxelFacePenaltyTechniqueCompletion, m_signalVoxelFacePenaltyTechniqueCompletion, SignalVoxelFacePenaltyTechniqueCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	/** Slot to receive signal when the cluster visible prefis sum step has been done
	* @return nothing */
	void ClusterVisiblePrefixSum();

	/** Slot for the keyboard signal when pressing the 3 key to remove 100 units from MaterialLightBounceVoxelIrradiance::m_formFactorVoxelToVoxelAdded
	* @return nothing */
	void slot3KeyPressed();

	/** Slot for the keyboard signal when pressing the 4 key to add 100 units from MaterialLightBounceVoxelIrradiance::m_formFactorVoxelToVoxelAdded
	* @return nothing */
	void slot4KeyPressed();

	/** Slot for the keyboard signal when pressing the 5 key to remove 100 units from MaterialLightBounceVoxelIrradiance::m_formFactorClusterToVoxelAdded
	* @return nothing */
	void slot5KeyPressed();

	/** Slot for the keyboard signal when pressing the 6 key to remove 100 units from MaterialLightBounceVoxelIrradiance::m_formFactorClusterToVoxelAdded
	* @return nothing */
	void slot6KeyPressed();

	SignalVoxelFacePenaltyTechniqueCompletion m_signalVoxelFacePenaltyTechniqueCompletion; //!< Signal for completion of the technique
	BufferPrefixSumTechnique*                 m_techniquePrefixSum;                        //!< Pointer to the instance of the prefix sum technique
	ClusterVisiblePrefixSumTechnique*         m_techniqueClusterVisiblePrefixSum;          //!< Pointer to the instance of the cluster visible prefix sum technique
	Buffer*                                   m_clusterVisibilityFacePenaltyBuffer;        //!< Pointer to the clusterVisibilityFacePenaltyBuffer buffer having, for each voxel face, the penalty to apply when light gathering is computed to avoid light leaks due to some voxels faces having visible many more clusters than their neighbours
	Buffer*                                   m_closerVoxelVisibilityFacePenaltyBuffer;    //!< Pointer to the closerVoxelVisibilityFacePenaltyBuffer buffer having, for each voxel face, the penalty to apply when light gathering is computed to avoid light leaks due to many voxels close to a voxel face adding too much irradiance
	Buffer*                                   m_clusterVisibilityFacePenaltyDebugBuffer;   //!< Pointer to the clusterVisibilityFacePenaltyDebugBuffer buffer used for debug purposes
	Buffer*                                   m_voxelNeighbourIndexBuffer;                 //!< Pointer to the voxelNeighbourIndexBuffer buffer with the information for all 26 neighbours of each voxel (-1 for those not existing neighbours)
	vec4                                      m_sceneMin;                                  //!< Minimum value of the scene's aabb
	vec4                                      m_sceneExtent;                               //!< Scene extent
	uint                                      m_numOccupiedVoxel;                          //!< Number of occupied voxels after voxelization process
	bool                                      m_prefixSumCompleted;                        //!< Flag to know if the prefix sum step has completed
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _VOXELFACEPENALTYTECHNIQUE_H_
