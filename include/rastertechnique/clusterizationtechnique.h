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

#ifndef _CLUSTERIZATIONTECHNIQUE_H_
#define _CLUSTERIZATIONTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class SceneVoxelizationTechnique;
class BufferPrefixSumTechnique;
class Camera;
class Buffer;
class MaterialClusterization;
class MaterialClusterizationNewCenter;
class MaterialClusterizationInitVoxelDistance;
class MaterialClusterizationAddUp;

// NAMESPACE

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalResetRadianceDataCompletion;
#define CLUSTERIZATION_NUM_ITERATION 10

/////////////////////////////////////////////////////////////////////////////////////////////

class ClusterizationTechnique : public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(ClusterizationTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	ClusterizationTechnique(string &&name, string&& className);

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

	/** Computes the number of super voxels (clusters) that are considered depending on the voxelization volume size
	* @param voxelizationSize [in] size of the vozxelization texture
	* @return the number of super voxels considered */
	static int getNumberSuperVoxel(int voxelizationSize);

	REF(SignalResetRadianceDataCompletion, m_signalResetRadianceDataCompletion, SignalResetRadianceDataCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	SignalResetRadianceDataCompletion        m_signalResetRadianceDataCompletion;                   //!< Signal for completion of the technique
	SceneVoxelizationTechnique*              m_sceneVoxelizationTechnique;                          //!< Pointer to ths instance of the scene voxelization technique
	uint                                     m_fragmentCounter;                                     //!< Emitted fragments counter
	bool                                     m_prefixSumCompleted;                                  //!< Flag to know if the prefix sum step has completed
	int                                      m_voxelizationSize;                                    //!< Voxelization size
	float                                    m_voxelizationSizeFloat;                               //!< Voxelization size float value
	int                                      m_numOccupiedVoxel;                                    //!< Number of occupied voxel
	BufferPrefixSumTechnique*                m_bufferPrefixSumTechnique;                            //!< Pointer to ths instance of the buffer prefix sum technique
	Buffer*                                  m_voxelClusterOwnerIndexBuffer;                        //!< Buffer used to store the cluster owner information in the clusterization process, there are as many elements as occupied voxels (the index of the cluster is used as identifier)
	Buffer*                                  m_voxelClusterOwnerDistanceBuffer;                     //!< Buffer used to store the distance to the cluster owner in the clusterization process, there are as many elements as occupied voxels (the index of the cluster is used as identifier)
	Buffer*                                  m_clusterizationCenterCoordinatesBuffer;               //!< Buffer used to store the cluster center coordinates in the clusterization process (3D texture coordinates) after the first iteration, the number of elements in this buffer is 3 x (number of clusters)
	Buffer*                                  m_clusterizationCenterPositiveDirectionBuffer;         //!< Buffer used to store the cluster positive normal direction coordinates in the clusterization process (3D normal direction) after the first iteration, the number of elements in this buffer is 3 x (number of clusters). The positive / negative is due to the fact that atomics are used to add-up all normal directions, and the nromal coordiantes are stored as uint. Normal direction compressed coordinates can't be added, so non-compressed coordinates with sign are implemented
	Buffer*                                  m_clusterizationCenterNegativeDirectionBuffer;         //!< Buffer used to store the cluster negative normal direction coordinates in the clusterization process (3D normal direction) after the first iteration, the number of elements in this buffer is 3 x (number of clusters). The positive / negative is due to the fact that atomics are used to add-up all normal directions, and the nromal coordiantes are stored as uint. Normal direction compressed coordinates can't be added, so non-compressed coordinates with sign are implemented
	Buffer*                                  m_clusterizationCenterPositivePreviousDirectionBuffer; //!< Buffer used to store the previous cluster positive normal direction coordinates in the clusterization process (3D normal direction) after the first iteration, the number of elements in this buffer is 3 x (number of clusters). The positive / negative is due to the fact that atomics are used to add-up all normal directions, and the nromal coordiantes are stored as uint. Normal direction compressed coordinates can't be added, so non-compressed coordinates with sign are implemented
	Buffer*                                  m_clusterizationCenterNegativePreviousDirectionBuffer; //!< Buffer used to store the previous cluster negative normal direction coordinates in the clusterization process (3D normal direction) after the first iteration, the number of elements in this buffer is 3 x (number of clusters). The positive / negative is due to the fact that atomics are used to add-up all normal directions, and the nromal coordiantes are stored as uint. Normal direction compressed coordinates can't be added, so non-compressed coordinates with sign are implemented
	Buffer*                                  m_clusterizationCenterCountsBuffer;                    //!< Buffer used to store the cluster center counts in the clusterization process after the first iteration (to compute the new mean normal direction and cluster center coordinates), the number of elements in this buffer is equal to the number of clusters
	Buffer*                                  m_clusterizationDebugBuffer;                           //!< Buffer for debug purposes
	Buffer*                                  m_clusterizationNewCenterDebugBuffer;                  //!< Buffer for debug purposes
	Buffer*                                  m_clusterizationAddUpDebugBuffer;                      //!< Buffer for debug purposes
	Buffer*                                  m_iterationCounterBuffer;                              //!< Buffer for iteration counter
	float                                    m_clusterizationStep;                                  //!< Clusterization step
	vec4                                     m_sceneMin;                                            //!< Scene minimum aabb value
	vec4                                     m_sceneExtent;                                         //!< Scene extent aabb value
	MaterialClusterization*                  m_materialClusterization;                              //!< Pointer to the clusterization material instance
	MaterialClusterizationNewCenter*         m_materialNewCenter;                                   //!< Pointer to the clusterization new center material instance
	uint                                     m_numClusterizationStep;                               //!< Number of iterations in the clusterization process
	uint                                     m_iterationCounterValue;                               //!< Value for the m_iterationCounterBuffer
	MaterialClusterizationInitVoxelDistance* m_materialInitVoxelDistance;                           //!< Pointer to the material instance for voxel distance initialization (required in each algorithm iteration)
	MaterialClusterizationAddUp*             m_materialAddUp;                                       //!< Pointer to the material instance for adding up cluster positions and normal directions to compute mean value
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CLUSTERIZATIONTECHNIQUE_H_
