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

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/camera/camera.h"
#include "../../include/core/coremanager.h"
#include "../../include/scene/scene.h"
#include "../../include/util/mathutil.h"

#include "../../include/util/bufferverificationhelper.h"

// NAMESPACE

// DEFINES
#define ARC_BALL_POSITION_DELTA 0.25f
#define ARC_BALL_DRAG_DELTA     0.05f

// STATIC MEMBER INITIALIZATION

mat4 perspectiveCamera(float fovy, float aspect, float zNear, float zFar)
{
	mat4 result = mat4(1.0f);

	float f = cos(fovy / 2.0f) / sin(fovy / 2.0f);  // cotangent

	//result[0]  = f / aspect;
	//result[5]  = f;
	//result[10] = (zNear + zFar) / (zNear - zFar);
	//result[11] = (2.0 * zNear * zFar) / (zNear - zFar);
	//result[14] = -1.0;
	//result[15] = 0;

	result[0][0]  = f / aspect;
	result[1][1]  = f;
	result[2][2]  = (zNear + zFar) / (zNear - zFar);
	result[3][2] = (2.0f * zNear * zFar) / (zNear - zFar);
	result[2][3] = -1.0;
	result[3][3] = 0;

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

Camera::Camera(string &&name,
			   CameraType cameraType,
			   vec3 vPos,
			   vec3 vLookAt,
			   vec3 up,
			   float zNear,
			   float zFar,
			   float fov):
	GenericResource(move(name), move(string("Camera")), GenericResourceType::GRT_CAMERA)
	, m_position(vPos)
	, m_lookAt(vLookAt)
	, m_positionPrevious(m_position)
	, m_lookAtPrevious(vLookAt)
	, m_up(up)
	, m_cameraType(cameraType)
	, m_right(vec3(.0f))
	, m_horAngle(0.0f)
	, m_verAngle(0.0f)
	, m_fov(1.047f) // For the capture compared with Cyril Crassin in Sponza
	//, m_fov(0.6108f) // Usual value
	, m_arcBallInitialDirection(normalize(m_lookAt - m_position))
	, m_arcBallDistance(length(m_lookAt - m_position))
	, m_arcBallTranslate(false)
	, m_arcballForwardFinal(vec3(.0f))
	, m_dXPos(-1)
	, m_dYPos(-1)
	, m_previousXPos(-1)
	, m_previousYPos(-1)
	, m_dirty(false)
	, m_zNear(zNear)
	, m_zFar(zFar)
	, m_arcballForwardPrevious(vec3(0.0f))
	, m_movingForward(false)
	, m_movingBackward(false)
	, m_movingLeft(false)
	, m_movingRight(false)
	, m_movementAcceleration(0.0f)
	, m_movementAccelerationMax(200.0f)
	, m_useRecordedCamera(false)
	, m_isAnimated(false)
	, m_animationElapsedTime(0.0f)
{
	m_lookAt           *= -1.0f;
	m_right             = normalize(cross(vec3(0.0f, 1.0f, 0.0f), m_lookAt));
	m_up                = normalize(cross(m_lookAt, m_right)); // Up vector
	m_view              = lookAt(m_position, m_position + m_lookAt, m_up);
	m_projection        = glm::perspective(m_fov, float(coreM->getWidth()) / float(coreM->getHeight()), m_zNear, m_zFar);
	m_projection[1][1] *= -1.0f;

	updateViewProjectionMatrix();
	updateFrustumPlanes();

	if (m_cameraType == CameraType::CT_FIRST_PERSON)
	{
		m_arcBallInitialDirection = m_lookAt;
	}

	float cosVerticalAngle   = dot(m_lookAt, vec3(0.0f, 0.0f, -1.0f));
	float cosHorizontalAngle = dot(m_up,     vec3(0.0f, 1.0f,  0.0f));
	cosVerticalAngle         = glm::clamp(cosVerticalAngle,   -1.0f, 1.0f);
	cosHorizontalAngle       = glm::clamp(cosHorizontalAngle, -1.0f, 1.0f);
	m_horAngle               = acos(cosHorizontalAngle);
	m_verAngle               = acos(cosVerticalAngle);
	m_horAngle              += pi<float>();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void extractSphericalThetaPhi(vec3 direction, float& theta, float& phi)
{
	if (direction.z > 0.0f)
	{
		theta = atan(sqrt(direction.x * direction.x + direction.y * direction.y) / direction.z);
	}
	else if (direction.z == 0.0f)
	{
		theta = glm::pi<float>() * 0.5f;
	}
	else // z < 0
	{
		theta = glm::pi<float>() + atan(sqrt(direction.x * direction.x + direction.y * direction.y) / direction.z);
	}

	if ((direction.x > 0.0f) && (direction.y > 0.0f))
	{
		phi = atan(direction.y  / direction.x);
	}
	else if ((direction.x > 0.0f) && (direction.y < 0.0f))
	{
		phi = 2.0f * glm::pi<float>() + atan(direction.y / direction.x);
	}
	else if (direction.y == 0.0f)
	{
		phi = glm::pi<float>() * 0.5f + glm::sign(direction.y);
	}
	else // x < 0
	{
		phi = glm::pi<float>() + atan(direction.y / direction.x);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::init()
{
	extractSphericalThetaPhi(m_arcBallInitialDirection, m_horAngle, m_verAngle);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::update(int32_t x, int32_t y)
{
	m_dXPos        = x;
	m_dYPos        = y;
	int halfWidth  = coreM->getWidth()  / 2;
	int halfHeight = coreM->getHeight() / 2;

	switch (m_cameraType)
	{
		case CameraType::CT_FIRST_PERSON:
		{
			int horizontalOffset = abs(halfWidth - x);
			int verticalOffset   = abs(halfHeight - y);
			
			if (x > halfWidth)
			{
				m_dXPos = halfWidth - horizontalOffset;
			}
			else
			{
				m_dXPos = halfWidth + horizontalOffset;
			}

			if (y > halfHeight)
			{
				m_dYPos = halfHeight - verticalOffset;
			}
			else
			{
				m_dYPos = halfHeight + verticalOffset;
			}
			break;
		}
		case CameraType::CT_ARC_BALL:
		{
			break;
		}
	}

	if (!m_arcBallTranslate)
	{
		float cosVerticalAngle   = dot(m_lookAt, vec3(0.0f, 1.0f, 0.0f));
		float cosHorizontalAngle = dot(m_up,     vec3(1.0f, 0.0f, 0.0f));
		cosVerticalAngle         = glm::clamp(cosVerticalAngle,   -1.0f, 1.0f);
		cosHorizontalAngle       = glm::clamp(cosHorizontalAngle, -1.0f, 1.0f);
		float verticalAngle      = acos(cosVerticalAngle);
		float horizontalAngle    = acos(cosHorizontalAngle);
		m_horAngle  -= MOUSE_SPEED * float(halfWidth  - m_dXPos);
		m_verAngle  -= MOUSE_SPEED * float(halfHeight - m_dYPos);
	}

	switch (m_cameraType)
	{
		case CameraType::CT_FIRST_PERSON:
		{
			if (m_useRecordedCamera)
			{
				m_lookAt = m_lookAtRecorded;
				m_right  = m_rightRecorded;
				m_up     = m_upRecorded;

				if (m_position != m_positionRecorded)
				{
					m_position         = m_positionRecorded;
					m_positionPrevious = m_position;
					m_dirty            = true;
				}
			}
			else
			{
				m_lookAt = normalize(vec3(cos(m_verAngle) * sin(m_horAngle), sin(m_verAngle), cos(m_verAngle) * cos(m_horAngle)));
				m_right  = normalize(cross(vec3(0.0f, 1.0f, 0.0f), m_lookAt));
				m_up     = normalize(cross(m_lookAt, m_right)); // Up vector	
			}
			
			m_view = lookAt(m_position, m_position + m_lookAt, m_up);

			if (m_lookAtPrevious != m_lookAt)
			{
				m_dirty = true;
			}

			m_lookAtPrevious = m_lookAt;

			break;
		}
		case CameraType::CT_ARC_BALL:
		{
			if (m_arcBallTranslate)
			{
				int xDelta                = halfWidth  - m_dXPos;
				int yDelta                = halfHeight - m_dYPos;
				float xOffset             = xDelta > 0.0f ? 1.0f : xDelta < 0.0f ? -1.0f : 0.0f;
				float zOffset             = yDelta > 0.0f ? 1.0f : yDelta < 0.0f ? -1.0f : 0.0f;
				vec3 offsetVector         = -1.0f * m_right * xOffset * ARC_BALL_DRAG_DELTA - 1.0f * m_up * zOffset * ARC_BALL_DRAG_DELTA;
				offsetVector.y            = 0.0f; // Move in the xz plane

				if (m_useRecordedCamera)
				{
					m_lookAt = m_lookAtRecorded;

					if (m_position != m_positionRecorded)
					{
						m_position         = m_positionRecorded;
						m_positionPrevious = m_position;
						m_dirty            = true;
					}
				}
				else
				{
					m_lookAt += offsetVector;
				}

				m_position               += offsetVector;
				m_arcBallInitialDirection = normalize(m_lookAt - m_position);

				if ((m_positionPrevious != m_position) || (m_lookAtPrevious != m_lookAt))
				{
					m_dirty = true;
				}
				m_positionPrevious = m_position;
				m_lookAtPrevious   = m_lookAt;
			}

			if (!m_arcBallTranslate)
			{
				vec3 forwardRotatedXZPlane = MathUtil::rotateVector(m_horAngle, vec3(0.0f, 1.0f, 0.0f), m_arcBallInitialDirection);
				m_right                    = normalize(cross(vec3(0.0f, 1.0f, 0.0f), forwardRotatedXZPlane));
				m_arcballForwardFinal      = MathUtil::rotateVector(m_verAngle, m_right, forwardRotatedXZPlane);
				m_up                       = MathUtil::rotateVector(m_verAngle, m_right, vec3(0.0f, 1.0f, 0.0f));
			}

			m_view = lookAt(m_arcBallDistance * m_arcballForwardFinal, m_lookAt, m_up);

			if (m_arcballForwardFinal != m_arcballForwardPrevious)
			{
				m_dirty = true;
			}

			m_arcballForwardPrevious = m_arcballForwardFinal;

			break;
		}
		default:
		{
			cout << "ERROR: no camera type defined in Camera::update" << endl;
			break;
		}
	}

	m_projection        = glm::perspective(m_fov, float(coreM->getWidth()) / float(coreM->getHeight()), m_zNear, m_zFar);
	//m_projection = perspectiveCamera(m_fov, float(coreM->getWidth()) / float(coreM->getHeight()), m_zNear, m_zFar);
	m_projection[1][1] *= -1.0f;

	inputM->setCursorPos(halfWidth, halfHeight);
	updateViewProjectionMatrix();
	updateFrustumPlanes();

	m_previousXPos = m_dXPos;
	m_previousYPos = m_dYPos;

	// Notify if m_position or m_lookAt values changed
	if (m_dirty)
	{
		m_cameraDirtySignal.emit();
		m_dirty = false;
	}

	BBox3D& box = sceneM->refBox();
	vec3 max3D;
	vec3 min3D;
	box.getCenteredBoxMinMax(min3D, max3D);
	vec3 extent3D = max3D - min3D;
	vec3 sceneMin3D = vec3(min3D.x, min3D.y, min3D.z);
	uvec3 position0 = BufferVerificationHelper::worldToVoxelSpace(m_position, vec3(128.0f), extent3D, sceneMin3D);

	// cout << "Voxelized position=(" << position0.x << ", " << position0.y << ", " << position0.z << "), Camera position = (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::moveForward(float units)
{
	m_movingForward = (units > 0.0f);
	m_movingBackward = (units < 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::moveRight(float units)
{
	m_movingLeft = (units < 0.0f);
	m_movingRight = (units > 0.0f);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::updateMovementState(float deltaTime, float offsetFactor)
{
	vec3 positionOffset = vec3(0.0f);

	if (m_movingForward)
	{
		positionOffset += m_lookAt * offsetFactor;
	}

	if (m_movingBackward)
	{
		positionOffset -= m_lookAt * offsetFactor;
	}

	if (m_movingLeft)
	{
		positionOffset += m_right * offsetFactor;
	}

	if (m_movingRight)
	{
		positionOffset -= m_right * offsetFactor;
	}

	if(dot(positionOffset, vec3(1.0f)) != 0.0f)
	{
		m_lastMovementDirection = positionOffset;

		if (m_movementAcceleration < m_movementAccelerationMax)
		{
			m_movementAcceleration += deltaTime;
			m_movementAcceleration  = glm::clamp(m_movementAcceleration, 0.0f, m_movementAccelerationMax);
		}

		offsetPosition(normalize(positionOffset) * offsetFactor);
	}
	else
	{
		if (m_movementAcceleration > 0.0f)
		{
			m_movementAcceleration -= deltaTime;
			m_movementAcceleration  = max(0.0f, m_movementAcceleration);
		}

		if ((dot(m_lastMovementDirection, vec3(1.0f)) != 0.0f) && (m_movementAcceleration > 0.0f))
		{
			offsetPosition(normalize(m_lastMovementDirection) * offsetFactor);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::updateCameraAnimation(float deltaTime)
{
	// NOTE: This is temporal code
	//vec3 position0          = vec3(4.09118, 22.985, -5.47123);
	//vec3 position1          = vec3(4.09118, 22.985, 4.63346);
	vec3 position0          = vec3(-11.09118, 31.2, -5.47123);
	vec3 position1          = vec3(-11.09118, 31.2, 4.63346);


	//(-10.1624, 31.2414, 1.67363)
	vec3 midPoint           = (position0 + position1) * 0.5f;
	vec3 directionOffset    = vec3(0.0f, 0.0f, 5.0f);
	//float totalTime         = 100000.0f;
	//float totalTime         = 250000.0f;
	float totalTime         = 150000.0f;
	m_animationElapsedTime += deltaTime;

	if (m_animationElapsedTime > totalTime)
	{
		m_animationElapsedTime -= totalTime;
	}

	float normalizedValue = m_animationElapsedTime / totalTime;
	float value           = 2.0f * glm::pi<float>() * normalizedValue;
	float pointOffset     = sin(value);
	vec3 finalPoint       = midPoint + pointOffset * directionOffset;
	m_position            = finalPoint;
	m_view                = lookAt(m_position, m_position + m_lookAt, m_up);

	updateViewProjectionMatrix();
	updateFrustumPlanes();

	m_positionPrevious = m_position;

	m_cameraDirtySignal.emit();

	/*vec3 position0          = vec3(4.09118, 22.985, -5.47123);
	vec3 position1          = vec3(4.09118, 22.985, 4.63346);
	vec3 midPoint           = (position0 + position1) * 0.5f;
	vec3 directionOffset    = vec3(5.0f, 0.0f, 0.0f);
	//float totalTime         = 5000.0f;
	float totalTime         = 25000.0f;
	m_animationElapsedTime += deltaTime;

	if (m_animationElapsedTime > totalTime)
	{
		m_animationElapsedTime -= totalTime;
	}

	float normalizedValue = m_animationElapsedTime / totalTime;
	float value           = 2.0f * glm::pi<float>() * normalizedValue;
	float pointOffset     = sin(value);
	vec3 finalPoint       = midPoint + pointOffset * directionOffset;
	m_position            = finalPoint;
	m_view                = lookAt(m_position, m_position + m_lookAt, m_up);

	updateViewProjectionMatrix();
	updateFrustumPlanes();

	m_positionPrevious = m_position;

	m_cameraDirtySignal.emit();*/
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotUpKeyPressed()
{
	if (m_cameraType == CameraType::CT_ARC_BALL)
	{
		return;
	}

	static float temp = 1.0f;
	moveForward(sceneM->getDeltaTime() * CAMERA_SPEED * 1000000.0f * temp);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotDownKeyPressed()
{
	if (m_cameraType == CameraType::CT_ARC_BALL)
	{
		return;
	}

	static float temp = 1.0f;
	moveForward(-1.0f * sceneM->getDeltaTime() * CAMERA_SPEED * 1000000.0f * temp);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotLeftKeyPressed()
{
	if (m_cameraType == CameraType::CT_ARC_BALL)
	{
		return;
	}

	static float temp = 1.0f;
	moveRight(-1.0f * sceneM->getDeltaTime() * CAMERA_SPEED * 1000000.0f * temp);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotRightKeyPressed()
{
	if (m_cameraType == CameraType::CT_ARC_BALL)
	{
		return;
	}

	static float temp = 1.0f;
	moveRight(sceneM->getDeltaTime() * CAMERA_SPEED * 1000000.0f * temp);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotUpKeyUp()
{
	m_movingForward = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotDownKeyUp()
{
	m_movingBackward = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotLeftKeyUp()
{
	m_movingLeft = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotRightKeyUp()
{
	m_movingRight = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotMouseWheelPositiveDelta()
{
	if (m_cameraType != CameraType::CT_ARC_BALL)
	{
		return;
	}

	m_arcBallDistance -= ARC_BALL_POSITION_DELTA;
	update(m_dXPos, m_dYPos);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotMouseWheelNegativeDelta()
{
	if (m_cameraType != CameraType::CT_ARC_BALL)
	{
		return;
	}

	m_arcBallDistance += ARC_BALL_POSITION_DELTA;
	update(m_dXPos, m_dYPos);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotLeftMouseButtonDown()
{
	m_arcBallTranslate = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::slotLeftMouseButtonUp()
{
	m_arcBallTranslate = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::offsetPosition(vec3 offset)
{
	vec3 positionOffset = offset * (m_movementAcceleration / m_movementAccelerationMax);
	m_position         += positionOffset;
	m_view              = lookAt(m_position, m_position + m_lookAt, m_up);

	updateViewProjectionMatrix();
	updateFrustumPlanes();

	if (m_positionPrevious != m_position)
	{
		m_dirty = true;
	}

	m_positionPrevious = m_position;
}


/////////////////////////////////////////////////////////////////////////////////////////////

void Camera::updateFrustumPlanes()
{
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_LEFT)].x = m_viewProjection[0].w + m_viewProjection[0].x;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_LEFT)].y = m_viewProjection[1].w + m_viewProjection[1].x;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_LEFT)].z = m_viewProjection[2].w + m_viewProjection[2].x;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_LEFT)].w = m_viewProjection[3].w + m_viewProjection[3].x;

	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_RIGHT)].x = m_viewProjection[0].w - m_viewProjection[0].x;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_RIGHT)].y = m_viewProjection[1].w - m_viewProjection[1].x;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_RIGHT)].z = m_viewProjection[2].w - m_viewProjection[2].x;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_RIGHT)].w = m_viewProjection[3].w - m_viewProjection[3].x;

	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_TOP)].x = m_viewProjection[0].w - m_viewProjection[0].y;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_TOP)].y = m_viewProjection[1].w - m_viewProjection[1].y;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_TOP)].z = m_viewProjection[2].w - m_viewProjection[2].y;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_TOP)].w = m_viewProjection[3].w - m_viewProjection[3].y;

	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BOTTOM)].x = m_viewProjection[0].w + m_viewProjection[0].y;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BOTTOM)].y = m_viewProjection[1].w + m_viewProjection[1].y;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BOTTOM)].z = m_viewProjection[2].w + m_viewProjection[2].y;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BOTTOM)].w = m_viewProjection[3].w + m_viewProjection[3].y;

	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BACK)].x = m_viewProjection[0].w + m_viewProjection[0].z;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BACK)].y = m_viewProjection[1].w + m_viewProjection[1].z;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BACK)].z = m_viewProjection[2].w + m_viewProjection[2].z;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_BACK)].w = m_viewProjection[3].w + m_viewProjection[3].z;

	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_FRONT)].x = m_viewProjection[0].w - m_viewProjection[0].z;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_FRONT)].y = m_viewProjection[1].w - m_viewProjection[1].z;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_FRONT)].z = m_viewProjection[2].w - m_viewProjection[2].z;
	m_arrayFrustumPlane[static_cast<int>(FrustumPlaneIndex::FPI_FRONT)].w = m_viewProjection[3].w - m_viewProjection[3].z;

	for (auto i = 0; i < static_cast<int>(FrustumPlaneIndex::FPI_TOTAL); i++)
	{
		float length = sqrtf(m_arrayFrustumPlane[i].x * m_arrayFrustumPlane[i].x + m_arrayFrustumPlane[i].y * m_arrayFrustumPlane[i].y + m_arrayFrustumPlane[i].z * m_arrayFrustumPlane[i].z);
		m_arrayFrustumPlane[i] /= length;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
