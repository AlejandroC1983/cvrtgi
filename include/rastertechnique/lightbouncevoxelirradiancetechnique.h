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

#ifndef _LIGTHBOUNCEVOXELIRRADIANCETECHNIQUE_H_
#define _LIGTHBOUNCEVOXELIRRADIANCETECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class Buffer;
class BufferPrefixSumTechnique;
class LitClusterProcessResultsTechnique;
class LitClusterTechnique;
class ResetClusterIrradianceDataTechnique;
class Camera;
class CameraVisibleVoxelTechnique;

// NAMESPACE

// DEFINES
// TODO: unify in a single define in common .h file and remove all duplicates
typedef Nano::Signal<void()> SignalLightBounceVoxelIrradianceCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class LightBounceVoxelIrradianceTechnique: public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(LightBounceVoxelIrradianceTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	LightBounceVoxelIrradianceTechnique(string &&name, string&& className);

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

	REF(SignalLightBounceVoxelIrradianceCompletion, m_signalLightBounceVoxelIrradianceCompletion, SignalLightBounceVoxelIrradianceCompletion)

protected:
	/** Slot to receive signal when the prefix sum step has been done
	* @return nothing */
	void slotPrefixSumComplete();

	/** Slot to receive notification when the LitClusterProcessResultsTechnique technique has completed
	* @return nothing */
	//void slotLitClusterProcessResultsCompleted();

	/** Slot to receive notification when the CameraVisibleVoxelTechnique has completed
	* @return nothing */
	void slotCameraVisibleVoxelCompleted();

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

	SignalLightBounceVoxelIrradianceCompletion  m_signalLightBounceVoxelIrradianceCompletion; //!< Signal for completion of the technique
	BufferPrefixSumTechnique*                   m_techniquePrefixSum;                         //!< Pointer to the instance of the prefix sum technique
	LitClusterProcessResultsTechnique*          m_litClusterProcessResultsTechnique;          //!< Pointer to the instace of the lit cluster process results technique
	LitClusterTechnique*                        m_litClusterTechnique;                        //!< Pointer to the lit cluster technique
	ResetClusterIrradianceDataTechnique*        m_resetClusterIrradianceDataTechnique;        //!< Pointer to the instance of the reset cluster irradiance data technique
	Buffer*                                     m_lightBounceVoxelIrradianceBuffer;           //!< Pointer to the lightBounceVoxelIrradianceBuffer buffer
	Buffer*                                     m_lightBounceVoxelDebugBuffer;                //!< Pointer to debug buffer
	Buffer*                                     m_lightBounceIndirectLitIndexBuffer;          //!< Pointer to buffer storing the indices of the voxels which received an irradiance greater than zero during the light bounce simulation
	Buffer*                                     m_lightBounceIndirectLitCounterBuffer;        //!< Pointer to buffer used as an atomic counter tho count the number of voxels which received an amount of irradiance greater than zero
	vec4                                        m_sceneMin;                                   //!< Minimum value of the scene's aabb
	vec4                                        m_sceneExtent;                                //!< Scene extent
	uint                                        m_numOccupiedVoxel;                           //!< Number of occupied voxels after voxelization process
	bool                                        m_prefixSumCompleted;                         //!< Flag to know if the prefix sum step has completed
	Camera*                                     m_mainCamera;                                 //!< Scene main camera
	CameraVisibleVoxelTechnique*                m_cameraVisibleVoxelTechnique;                //!< Pointer to the camera visible voxel technique instance
	uint                                        m_cameraVisibleVoxelNumber;                   //!< Number of visible voxel determined by the CameraVisibleVoxelTechnique technique
	uint                                        m_lightBounceIndirectLitCounter;              //!< Helper variable to take the value from m_lightBounceIndirectLitCounterBuffer
	Buffer*                                     m_lightBounceVoxelGaussianFilterDebugBuffer;  //!< Buffer for debug purposes
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _LIGTHBOUNCEIRRADIANCEFIELDTECHNIQUE_H_
