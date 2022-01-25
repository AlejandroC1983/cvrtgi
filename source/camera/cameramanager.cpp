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
#include "../../include/camera/cameramanager.h"
#include "../../include/camera/camera.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/core/input.h"
#include "../../include/scene/scene.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION
int CameraManager::m_nextCameraOperateIndex = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

CameraManager::CameraManager():
	  m_mainCamera(nullptr)
	, m_operatingCamera(false)
	, m_cameraOperated(nullptr)
	, m_cyclingRecorderCameras(false)
	, m_recorderCameraIndex(0)
{
	m_managerName = g_cameraManager;

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_S);
	SignalVoid* signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_S);
	signalAdd->connect<CameraManager, &CameraManager::useNextCamera>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_D);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_D);
	signalAdd->connect<CameraManager, &CameraManager::operateNextCamera>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_9);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_9);
	signalAdd->connect<CameraManager, &CameraManager::slot9Pressed>(this);

	inputM->refEventSinglePressSignalSlot().addKeyDownSignal(KeyCode::KEY_CODE_0);
	signalAdd = inputM->refEventSinglePressSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_0);
	signalAdd->connect<CameraManager, &CameraManager::slot0Pressed>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

CameraManager::~CameraManager()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

Camera* CameraManager::buildCamera(string &&instanceName,
								   CameraType cameraType,
								   vec3 vPos,
								   vec3 vLookAt,
								   vec3 up,
								   float zNear,
								   float zFar,
								   float fov)
{
	if (existsElement(move(string(instanceName))))
	{
		return getElement(move(instanceName));
	}

	Camera* camera = new Camera(move(string(instanceName)), cameraType, vPos, vLookAt, up, zNear, zFar, fov);

	addElement(move(string(instanceName)), camera);
	camera->m_name  = move(instanceName);
	camera->m_ready = true;

	return camera;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::destroyResources()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::setAsMainCamera(Camera* camera)
{
	if (m_mainCamera != nullptr)
	{
		cleanAllCameraSignal();
	}

	m_mainCamera      = camera;
	m_cameraOperated  = nullptr;
	m_operatingCamera = false;

	assignSlotsToCamera(m_mainCamera);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::updateMovementState(float deltaTime, float offsetFactor)
{
	forIT(m_mapElement)
	{
		it->second->updateMovementState(deltaTime, offsetFactor);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::updateCameraAnimation(float deltaTime)
{
	forIT(m_mapElement)
	{
		if (it->second->getIsAnimated())
		{
			it->second->updateCameraAnimation(deltaTime);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::loadCameraRecordingData()
{
	string sceneName = sceneM->getSceneName();
	ifstream myfile;
	myfile.open("../data/scenes/temp/" + sceneName + "_recorded_camera", std::ios_base::binary);

	if (!myfile)
	{
		return;
	}

	int numElement;
	myfile.read(reinterpret_cast<char*>(&numElement), sizeof(int));
	m_vectorRecordedCamera.resize(numElement);

	forI(m_vectorRecordedCamera.size())
	{
		myfile.read(reinterpret_cast<char*>(&m_vectorRecordedCamera[i]), sizeof(RecordedCamera));
	}
	myfile.close();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::writeCameraRecordingData()
{
	if (m_vectorRecordedCamera.size() == 0)
	{
		return;
	}

	string sceneName = sceneM->getSceneName();
	ofstream myfile;
	myfile.open("../data/scenes/temp/" + sceneName + "_recorded_camera", std::ios_base::binary | std::ios_base::trunc);
	int numElement = int(m_vectorRecordedCamera.size());
	myfile.write(reinterpret_cast<char*>(&numElement), sizeof(int));

	forI(m_vectorRecordedCamera.size())
	{
		myfile.write(reinterpret_cast<char*>(&m_vectorRecordedCamera[i]), sizeof(RecordedCamera));
	}

	myfile.close();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::takeCameraRecording(vec3 position, vec3 lookAt, vec3 up, vec3 right, const mat4& view, const mat4& projection)
{
	m_vectorRecordedCamera.push_back(RecordedCamera({ position, lookAt, up, right, view, projection }));
	writeCameraRecordingData();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::useNextCamera()
{
	vector<Camera*> vectorCamera = getVectorElement();
	vector<Camera*>::iterator it = find(vectorCamera.begin(), vectorCamera.end(), m_mainCamera);

	if (it == vectorCamera.end())
	{
		cout << "ERROR trying to use next camera in CameraManager::useNextCamera" << endl;
		return;
	}

	int index         = int(distance(vectorCamera.begin(), it));
	int nextIndex     = (index + 1) % int(vectorCamera.size());
	setAsMainCamera(vectorCamera[nextIndex]);
	m_operatingCamera = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::operateNextCamera()
{
	vector<Camera*> vectorCamera = getVectorElement();
	int nextIndex                = (m_nextCameraOperateIndex + 1) % int(vectorCamera.size());
	m_cameraOperated             = vectorCamera[nextIndex];
	m_nextCameraOperateIndex++;

	cleanAllCameraSignal();
	assignSlotsToCamera(m_cameraOperated);

	m_operatingCamera = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::cleanAllCameraSignal()
{
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_UP   )->removeAll();
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_DOWN )->removeAll();
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_LEFT )->removeAll();
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_RIGHT)->removeAll();

	inputM->refEventSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_UP   )->removeAll();
	inputM->refEventSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_DOWN )->removeAll();
	inputM->refEventSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_LEFT )->removeAll();
	inputM->refEventSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_RIGHT)->removeAll();

	inputM->refEventSignalSlot().refLeftMouseButtonDown().removeAll();
	inputM->refEventSignalSlot().refLeftMouseButtonUp().removeAll();

	inputM->refEventSinglePressSignalSlot().refMouseWheelPositiveDelta().removeAll();
	inputM->refEventSinglePressSignalSlot().refMouseWheelNegativeDelta().removeAll();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::assignSlotsToCamera(Camera* camera)
{
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_UP   )->connect<Camera, &Camera::slotUpKeyPressed>   (camera);
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_DOWN )->connect<Camera, &Camera::slotDownKeyPressed> (camera);
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_LEFT )->connect<Camera, &Camera::slotLeftKeyPressed> (camera);
	inputM->refEventSignalSlot().refKeyDownSignalByKey(KeyCode::KEY_CODE_RIGHT)->connect<Camera, &Camera::slotRightKeyPressed>(camera);

	inputM->refEventSinglePressSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_UP   )->connect<Camera, &Camera::slotUpKeyUp>(camera);
	inputM->refEventSinglePressSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_DOWN )->connect<Camera, &Camera::slotDownKeyUp>(camera);
	inputM->refEventSinglePressSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_LEFT )->connect<Camera, &Camera::slotLeftKeyUp>(camera);
	inputM->refEventSinglePressSignalSlot().refKeyUpSignalByKey(KeyCode::KEY_CODE_RIGHT)->connect<Camera, &Camera::slotRightKeyUp>(camera);

	inputM->refEventSignalSlot().refLeftMouseButtonDown().connect<Camera, &Camera::slotLeftMouseButtonDown>(camera);
	inputM->refEventSignalSlot().refLeftMouseButtonUp().connect<Camera, &Camera::slotLeftMouseButtonUp>(camera);

	inputM->refEventSinglePressSignalSlot().refMouseWheelPositiveDelta().connect<Camera, &Camera::slotMouseWheelPositiveDelta>(camera);
	inputM->refEventSinglePressSignalSlot().refMouseWheelNegativeDelta().connect<Camera, &Camera::slotMouseWheelNegativeDelta>(camera);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::slot9Pressed()
{
	takeCameraRecording(m_mainCamera->getPosition(),
						m_mainCamera->getLookAt(),
						m_mainCamera->refUpVector(),
						m_mainCamera->refRightVector(),
						m_mainCamera->getView(),
						m_mainCamera->getProjection());
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraManager::slot0Pressed()
{
	if (m_cyclingRecorderCameras == false)
	{
		m_cyclingRecorderCameras = true;
		m_recorderCameraIndex = 0;
	}
	else
	{
		m_recorderCameraIndex++;
		if ((m_recorderCameraIndex) == int(m_vectorRecordedCamera.size()))
		{
			m_cyclingRecorderCameras = false;
		}
	}

	if (m_cyclingRecorderCameras)
	{
		m_mainCamera->setUseRecordedCamera(true);
		m_mainCamera->setLookAtRecorded(m_vectorRecordedCamera[m_recorderCameraIndex].m_lookAt);
		m_mainCamera->setUpRecorded(m_vectorRecordedCamera[m_recorderCameraIndex].m_up);
		m_mainCamera->setRightRecorded(m_vectorRecordedCamera[m_recorderCameraIndex].m_right);
		m_mainCamera->setPositionRecorded(m_vectorRecordedCamera[m_recorderCameraIndex].m_position);
	}
	else
	{
		m_mainCamera->setUseRecordedCamera(false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
