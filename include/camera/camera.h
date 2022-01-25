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

#ifndef _CAMERA_H_
#define _CAMERA_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_observer.hpp"
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/commonnamespace.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES
typedef Nano::Signal<void()> SignalCameraDirtyNotification;

/////////////////////////////////////////////////////////////////////////////////////////////
/** Class used to manage the scene camera, taken from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/ */
class Camera : public GenericResource, public Nano::Observer
{
	friend class CameraManager;

public:
	/** Class constructor, the parameters are the camera position, where is it looking at, and the up vector of the camera
	* @param name       [in] camera's name
	* @param cameraType [in] camera type
	* @param vPos       [in] position of the camera
	* @param vLookAt    [in] where is the camera looking at
	* @param up         [in] up vector
	* @param zNear      [in] near clip plane distance
	* @param zFar       [in] far clip plane distance
	* @param fov        [in] field of view (radians)
	* @return nothing */
	Camera(string &&name,
		   CameraType cameraType,
		   vec3 vPos,
		   vec3 vLookAt,
		   vec3 up,
		   float zNear,
		   float zFar,
		   float fov);

	/** Initialize camera settings
	* @return nothing */
	void init();

	/** update camera data
	* @param x [in] new mouse position in client window coordinates
	* @param y [in] new mouse position in client window coordinates
	* @return nothing */
	void update(int32_t x, int32_t y);

	/** updates m_viewProjection
	* @return nothing */
	void updateViewProjectionMatrix();

	/** updates the camera applying a forward movement given by the units parameter
	* @param units [in] units to move forward the camera
	* @return nothing */
	void moveForward(float units);

	/** updates the camera applying a right direction movement given by the units parameter
	* @param units [in] units to move the camera in the right direction
	* @return nothing */
	void moveRight(float units);

	/** Updates the position of the camera, allowing a smooth movment independent from the framerate
	* @param deltaTime    [in] Elapsed time since last call to the funtion
	* @param offsetFactor [in] Offset factor to scale the movement speed
	* @return nothing */
	void updateMovementState(float deltaTime, float offsetFactor);

	/** Updates the position of the camera in case it is animated
	* @param deltaTime    [in] Elapsed time since last call to the funtion
	* @return nothing */
	void updateCameraAnimation(float deltaTime);

	//REFCOPY_SET(vec3, m_vPosition, Position)
	REFCOPY(vec3, m_position, Position)
	GETCOPY(vec3, m_position, Position)
	REFCOPY_SET(vec3, m_lookAt, LookAt)
	GETCOPY(vec3, m_lookAt, LookAt)
	REFCOPY_SET(vec3, m_up, UpVector)
	REFCOPY_SET(vec3, m_right, RightVector)
	GETCOPY(float, m_horAngle, HorizontalAng)
	GETCOPY(float, m_verAngle, VerticalAng)
	GETCOPY_SET(float, m_fov, Fov)
	GET(mat4, m_view, View)
	GET(mat4, m_projection, Projection)
	GET(mat4, m_viewProjection, ViewProjection)
	REF(SignalCameraDirtyNotification, m_cameraDirtySignal, CameraDirtySignal)
	GETCOPY_SET(bool, m_useRecordedCamera, UseRecordedCamera)
	GETCOPY_SET(vec3, m_lookAtRecorded, LookAtRecorded)
	GETCOPY_SET(vec3, m_upRecorded, UpRecorded)
	GETCOPY_SET(vec3, m_rightRecorded, RightRecorded)
	GETCOPY_SET(vec3, m_positionRecorded, PositionRecorded)
	GET_ARRAY_AS_POINTER(vec4, m_arrayFrustumPlane, ArrayFrustumPlane)
	GETCOPY_SET(bool, m_isAnimated, IsAnimated)

	vec4* get()
	{
		return &m_arrayFrustumPlane[0];
	}

protected:
	/** Slot for receiving signal from the input manager when the up key is being pressed
	* @return nothing */
	void slotUpKeyPressed();

	/** Slot for receiving signal from the input manager when the down key is being pressed
	* @return nothing */
	void slotDownKeyPressed();

	/** Slot for receiving signal from the input manager when the left key is being pressed
	* @return nothing */
	void slotLeftKeyPressed();

	/** Slot for receiving signal from the input manager when the right key is being pressed
	* @return nothing */
	void slotRightKeyPressed();

	/** Slot for receiving signal from the input manager when the up key is being released
	* @return nothing */
	void slotUpKeyUp();

	/** Slot for receiving signal from the input manager when the down key is being released
	* @return nothing */
	void slotDownKeyUp();

	/** Slot for receiving signal from the input manager when the left key is being released
	* @return nothing */
	void slotLeftKeyUp();

	/** Slot for receiving signal from the input manager when the right key is being released
	* @return nothing */
	void slotRightKeyUp();

	/** Slot for receiving signal from the input manager when a positive delta mouse wheel event happens
	* @return nothing */
	void slotMouseWheelPositiveDelta();

	/** Slot for receiving signal from the input manager when a negative delta mouse wheel event happens
	* @return nothing */
	void slotMouseWheelNegativeDelta();

	/** Slot for receiving signal from the input manager when left mouse button is pressed
	* @return nothing */
	void slotLeftMouseButtonDown();

	/** Slot for receiving signal from the input manager when left mouse button is released
	* @return nothing */
	void slotLeftMouseButtonUp();

	/** Offset m_vPosition with the offset given as parameter, recompute view and view projection matrices
	* @param offset [in] Normalized offset to apply
	* @return nothing */
	void offsetPosition(vec3 offset);

	/** Update frustum planes
	* @return nothing */
	void updateFrustumPlanes();

	vec3                          m_position;                //!< Position of the camera
	vec3                          m_lookAt;                  //!< Target point for the camera
	vec3                          m_positionPrevious;        //!< Previous value of m_vPosition
	vec3                          m_lookAtPrevious;          //!< Previous value of m_vLookAt
	vec3                          m_up;                      //!< Up vector for the camera
	vec3                          m_right;                   //!< Right direction of the camera according to its position and orientation
	float                         m_horAngle;                //!< Camera angle in the xz plane
	float                         m_verAngle;                //!< Vertical camera angle
	float                         m_fov;                     //!< Camera fov
	mat4                          m_view;                    //!< View matrix
	mat4                          m_projection;              //!< Projection matrix
	mat4                          m_viewProjection;          //!< View - projection matrix
	vec3                          m_arcBallInitialDirection; //!< Arc ball camera initial direction
	float                         m_arcBallDistance;         //!< Arc ball camera distance to target (m_vLookAt)
	bool                          m_arcBallTranslate;        //!< Flag to know when to translate arc ball cmaera
	vec3                          m_arcballForwardFinal;     //!< Arc ball forward direction
	vec3                          m_arcballForwardPrevious;  //!< Previous value of m_arcballForwardFinal
	int32_t                       m_dXPos;                   //!< Mouse update event x screen coordinate
	int32_t                       m_dYPos;                   //!< Mouse update event y screen coordinate
	int32_t                       m_previousXPos;            //!< Previous value of m_dXPos
	int32_t                       m_previousYPos;            //!< Previous value of m_dYPos
	CameraType                    m_cameraType;              //!< Camera type
	bool                          m_dirty;                   //!< Flag to control when m_vPosition or m_vLookAt values change for single notification emit
	float                         m_zNear;                   //!< Near plane distance
	float                         m_zFar;                    //!< Far plane distance
	SignalCameraDirtyNotification m_cameraDirtySignal;       //!< Signal for camera dirty
	bool                          m_movingForward;           //!< Flag to know wether the camera is moving forward
	bool                          m_movingBackward;          //!< Flag to know wether the camera is moving backward
	bool                          m_movingLeft;              //!< Flag to know wether the camera is moving left
	bool                          m_movingRight;             //!< Flag to know wether the camera is moving right
	float                         m_movementAcceleration;    //!< Variable to smooth camera movement
	float                         m_movementAccelerationMax; //!< Limit for the accumulated value at m_movementAcceleration
	vec3                          m_lastMovementDirection;   //!< To track the last direction in which the cameramoved
	bool                          m_useRecordedCamera;       //!< Flag to know whether to use recorded data for this camera
	vec3                          m_lookAtRecorded;          //!< Recorded value of target point for the camera, to be used if m_useRecordedCamera is true
	vec3                          m_upRecorded;              //!< Recorded value of up vector for the camera, to be used if m_useRecordedCamera is true
	vec3                          m_rightRecorded;           //!< Recorded value of right direction of the camera according to its position and orientation, to be used if m_useRecordedCamera is true
	vec3                          m_positionRecorded;        //!< Recorded value of the camera position, to be used if m_useRecordedCamera is true
	vec4                          m_arrayFrustumPlane[6];    //!< Frustum planes
	bool                          m_isAnimated;              //!< Flag to know if the camera is animated
	float                         m_animationElapsedTime;    //!< Animation's elapsed time
};

/////////////////////////////////////////////////////////////////////////////////////////////

// INLINE METHODS

/////////////////////////////////////////////////////////////////////////////////////////////

inline void Camera::updateViewProjectionMatrix()
{
	m_viewProjection = m_projection * m_view;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _CAMERA_H_
