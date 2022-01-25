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

#ifndef _CAMERAVISIBLEVOXELTECHNIQUE_H_
#define _CAMERAVISIBLEVOXELTECHNIQUE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/rastertechnique/bufferprocesstechnique.h"

// CLASS FORWARDING
class Buffer;
class Camera;
class BufferPrefixSumTechnique;
class LitClusterProcessResultsTechnique;

// NAMESPACE

// DEFINES
typedef Nano::Signal<void()> SignalCameraVisibleVoxelCompletion;

/////////////////////////////////////////////////////////////////////////////////////////////

class CameraVisibleVoxelTechnique: public BufferProcessTechnique
{
	DECLARE_FRIEND_REGISTERER(CameraVisibleVoxelTechnique)

protected:
	/** Parameter constructor
	* @param name      [in] technique's name
	* @param className [in] class name
	* @return nothing */
	CameraVisibleVoxelTechnique(string &&name, string&& className);

public:
	/** New shaders, textures, etc initialization should be done here, called when the tehnique is instantiated
	* @return nothing */
	virtual void init();

	/** Called inside each technique's while record and submit loop, after the call to prepare and before the
	* technique record, to update any dirty material that needs to be rebuilt as a consequence of changes in the
	* resources used by the materials present in m_vectorMaterial
	* @return nothing */
	virtual void postCommandSubmit();

	REF(SignalCameraVisibleVoxelCompletion, m_signalCameraVisibleVoxelCompletion, SignalCameraVisibleVoxelCompletion)
	GETCOPY(uint, m_cameraVisibleVoxelNumber, CameraVisibleVoxelNumber)
	GETCOPY_SET(bool, m_lightBounceOnProgress, LightBounceOnProgress)

protected:
	/** Slot to receive notification when the prefix sum of the scene voxelization has been completed
	* @return nothing */
	void slotPrefixSumComplete();

	/** Slot to receive notification when the camera values (look at / position) are dirty
	* @return nothing */
	void slotCameraDirty();

	/** For debug purposes, show the contents of m_cameraVisibleVoxelCompactedBuffer
	* @return nothing */
	void showVisibleVoxelData();

	SignalCameraVisibleVoxelCompletion m_signalCameraVisibleVoxelCompletion; //!< Signal for camera visible voxel completion
	Buffer*                            m_cameraVisibleVoxelBuffer;           //!< Shader storage buffer where to flag whether a voxel is visible from the camera
	Buffer*                            m_cameraVisibleVoxelCompactedBuffer;  //!< Buffer storage buffer where to flag whether a voxel is visible from the camera as in m_cameraVisibleVoxelBuffer but with all the visible from camera voxel hashed indices starting from index 0
	Buffer*                            m_cameraVisibleVoxelDebugBuffer;      //!< Buffer storage buffer for debug purposes
	Buffer*                            m_cameraVisibleCounterBuffer;         //!< Buffer used as atomic counter for the camera visible voxels
	uint                               m_numOccupiedVoxel;                   //!< Number of occupied voxel
	BufferPrefixSumTechnique*          m_bufferPrefixSumTechnique;           //!< Pointer to the prefix sum technique
	bool                               m_prefixSumCompleted;                 //!< Flag to know if the prefix sum step has completed
	Camera*                            m_mainCamera;                         //!< Scene main camera
	vec3                               m_cameraPosition;                     //!< Camera position in the moment the slot reset radiance data is signaled, to store the original camera positon in the moment all the simulation starts (it takes several frames and the camera poition and forward direction can change in the meantime
	vec3                               m_cameraForward;                      //!< Camera forward direction in the moment the slot reset radiance data is signaled, to store the original camera positon in the moment all the simulation starts (it takes several frames and the camera poition and forward direction can change in the meantime
	uint                               m_cameraVisibleVoxelNumber;           //!< Where to put the lst recovered camera visible voxel result from m_cameraVisibleCounterBuffer buffer
	LitClusterProcessResultsTechnique* m_litClusterProcessResultsTechnique;  //!< Pointer to the instace of the lit cluster process results technique
	bool                               m_lightBounceOnProgress;              //!< Flag to avoid several visible voxel tests in the same light bouunce and gaussian filter simulation. This flag is reset by the gaussian filtering technique ince it finishes
	bool                               m_cameraDirtyWhileComputation;        //!< Flag to track whether the camera is dirty while performming the light bounce computation process (m_lightBounceOnProgress is true)
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CAMERAVISIBLEVOXELTECHNIQUE_H_
