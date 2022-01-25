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

#ifndef _LITCLUSTERTECHNIQUE_H_
#define _LITCLUSTERTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class Buffer;
class Camera;
class DistanceShadowMappingTechnique;
class BufferPrefixSumTechnique;
class ClusterizationBuildFinalBufferTechnique;
class LitVoxelTechnique;
class MaterialLitCluster;
class MaterialResetClusterIrradianceData;
class MaterialLitClusterProcessResults;

// NAMESPACE

// DEFINES
typedef Nano::Signal<void()> SignalLitClusterCompletion;
#define NUM_ADDUP_ELEMENT_PER_THREAD 25

/////////////////////////////////////////////////////////////////////////////////////////////

class LitClusterTechnique: public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(LitClusterTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	LitClusterTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called before rendering
	* @param dt [in] elapsed time in miliseconds since the last update call
	* @return nothing */
	virtual void prepare(float dt);

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

	/** Slot to receive notification when the camera values (look at / position) are dirty
	* @return nothing */
	void slotCameraDirty();

	REF(SignalLitClusterCompletion, m_signalLitClusterCompletion, SignalLitClusterCompletion)
	GETCOPY(vec3, m_cameraPosition, CameraPosition)
	GETCOPY(vec3, m_cameraForward, CameraForward)
	GETCOPY(float, m_emitterRadiance, EmitterRadiance)
	GETCOPY(uint, m_numAddUpElementPerThread, NumAddUpElementPerThread)
	SET(bool, m_techniqueLock, TechniqueLock)

protected:
	/** Slot to receive notification when the prefix sum of the scene voxelization has been completed
	* @return nothing */
	void slotPrefixSumComplete();

	/** Slot to receive signal when the clusterization build final buffer has completed
	* @return nothing */
	void slotClusterizationBuildFinalBufferCompletion();

	SignalLitClusterCompletion               m_signalLitClusterCompletion;              //!< Signal for lit cluster completion
	Buffer*                                  m_accumulatedIrradianceBuffer;             //!< Shader storage buffer where to store all irradiance in each light bounce step including irradiance from emitter arriving at the scene step
	Buffer*                                  m_litClusterCounterBuffer;                 //!< Buffer used as atomic counter for the visible clusters present in m_litVisibleClusterBuffer
	Buffer*                                  m_litToRasterVisibleClusterCounterBuffer;  //!< Buffer used as atomic counter to place and count the elements of m_litToRasterVisibleClusterBuffer
	Buffer*                                  m_litToRasterVisibleClusterBuffer;         //!< Buffer of visible from light test clusters that need to be rasterized
	Buffer*                                  m_alreadyRasterizedClusterBuffer;          //!< Buffer used to tag those clusters that have been already rasterized
	Buffer*                                  m_litVisibleClusterBuffer;                 //!< Buffer of visible from light test clusters
	Buffer*                                  m_litClusterDebugBuffer;                   //!< Buffer for debug purposes
	Buffer*                                  m_litTestVoxelBuffer;                      //!< Buffer of lit test voxels
	DistanceShadowMappingTechnique*          m_distanceShadowMappingTechnique;          //!< Pointer to the distance shadow mapping technique
	LitVoxelTechnique*                       m_litVoxelTechnique;                       //!< Pointer to the lit voxel technique
	BufferPrefixSumTechnique*                m_bufferPrefixSumTechnique;                //!< Pointer to the prefix sum technique
	ClusterizationBuildFinalBufferTechnique* m_clusterizationBuildFinalBufferTechnique; //!< Pointer to the clusterization build final buffer technique
	Camera*                                  m_voxelShadowMappingCamera;                //!< Camera used for voxel shadow mapping
	bool                                     m_prefixSumCompleted;                      //!< Flag to know if the prefix sum step has completed
	float                                    m_emitterRadiance;                         //!< Emitter radiance in the moment the slot reset radiance data is signaled, to store the original camera positon in the moment all the simulation starts (it takes several frames and the camera poition and forward direction can change in the meantime
	vec3                                     m_cameraPosition;                          //!< Camera position in the moment the slot reset radiance data is signaled, to store the original camera positon in the moment all the simulation starts (it takes several frames and the camera poition and forward direction can change in the meantime
	vec3                                     m_cameraForward;                           //!< Camera forward direction in the moment the slot reset radiance data is signaled, to store the original camera positon in the moment all the simulation starts (it takes several frames and the camera poition and forward direction can change in the meantime
	uvec3                                    m_accumulatedIrradiance;                   //!< Member variable to store accumulated irradiance values from m_accumulatedRadianceBuffer
	uint                                     m_totalRasterizedCluster;                  //!< Adds up all the successive values of m_litToRasterVisibleClusterCounterValue
	uint                                     m_numAddUpElementPerThread;                //!< Number of elements per thread to process when performing the add up process
	uint                                     m_numAddUpStepThread;                      //!< Number of threads that will take care of the add up step process (one single step before accumulating the final value)
	MaterialLitCluster*                      m_materialLitCluster;                      //!< Pointer to the instance of the lit cluster material

	// ResetClusterIrradianceDataTechnique
	MaterialResetClusterIrradianceData*      m_materialResetClusterIrradianceData;      //!< Pointer to the instance of the material used by this technique
	Buffer*                                  m_litTestClusterBuffer;                    //!< Buffer of lit clusters (used in litClusterTechnique, initialized and resized here to be able to attach it to the shader used in this technique which resets the buffer's information)
	Buffer*                                  m_litVisibleIndexClusterBuffer;            //!< Buffer with the index of the visible clusters from light (used in litClusterTechnique, initialized and resized here to be able to attach it to the shader used in this technique which resets the buffer's information)
	Buffer*                                  m_litToRasterVisibleIndexClusterBuffer;    //!< Buffer with the index of the visible clusters from light that need to be rasterized (used in litClusterTechnique, initialized and resized here to be able to attach it to the shader used in this technique which resets the buffer's information)
	bool                                     m_techniqueLock;                           //!< Flag to avoid the technique to request new command buffer record before all the chained techniques finish their corresponding command buffers executions
	// ResetClusterIrradianceDataTechnique

	// LitClusterProcessResultsTechnique
	MaterialLitClusterProcessResults*        m_materialLitClusterProcessResults;        //!< Pointer to the material instance
	uint                                     m_litClusterCounterValue;                  //!< Variable where to store the value of m_litClusterCounterBuffer
	uint                                     m_litToRasterVisibleClusterCounterValue;   //!< Variable where to store the value of m_litToRasterVisibleClusterCounterBuffer
	vec4                                     m_sceneExtentAndVoxelSize;                 //!< Extent of the scene in the xyz coordinates, voxelization texture size in the w coordinate
	vec4                                     m_sceneMin;
	// LitClusterProcessResultsTechnique
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _LITCLUSTERTECHNIQUE_H_
