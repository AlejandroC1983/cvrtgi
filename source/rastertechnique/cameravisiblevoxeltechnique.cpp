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
#include "../../include/rastertechnique/cameravisiblevoxeltechnique.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/scene/scene.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"
#include "../../include/material/materialcameravisiblevoxel.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/rastertechnique/shadowmappingvoxeltechnique.h"
#include "../../include/util/bufferverificationhelper.h"
#include "../../include/rastertechnique/litclustertechnique.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

CameraVisibleVoxelTechnique::CameraVisibleVoxelTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))	
	, m_cameraVisibleVoxelBuffer(nullptr)
	, m_cameraVisibleVoxelCompactedBuffer(nullptr)
	, m_cameraVisibleVoxelDebugBuffer(nullptr)
	, m_cameraVisibleCounterBuffer(nullptr)
	, m_numOccupiedVoxel(0)
	, m_bufferPrefixSumTechnique(nullptr)
	, m_prefixSumCompleted(false)
	, m_mainCamera(nullptr)
	, m_cameraVisibleVoxelNumber(0)
	, m_litClusterProcessResultsTechnique(nullptr)
	, m_lightBounceOnProgress(false)
	, m_cameraDirtyWhileComputation(false)
{
	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_executeCommand                    = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraVisibleVoxelTechnique::init()
{
	m_mainCamera    = cameraM->getElement(move(string("maincamera")));
	m_mainCamera->refCameraDirtySignal().connect<CameraVisibleVoxelTechnique, &CameraVisibleVoxelTechnique::slotCameraDirty>(this);

	m_cameraVisibleVoxelBuffer = bufferM->buildBuffer(
		move(string("cameraVisibleVoxelBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_cameraVisibleVoxelCompactedBuffer = bufferM->buildBuffer(
		move(string("cameraVisibleVoxelCompactedBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_cameraVisibleVoxelDebugBuffer = bufferM->buildBuffer(
		move(string("cameraVisibleVoxelDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	uint tempInitValue = 0;

	m_cameraVisibleCounterBuffer = bufferM->buildBuffer(
		move(string("cameraVisibleCounterBuffer")),
		(void*)(&tempInitValue),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	buildShaderThreadMapping();

	MultiTypeUnorderedMap* attributeMaterialAddUp = new MultiTypeUnorderedMap();
	attributeMaterialAddUp->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_cameraVisibleVoxelCodeChunk), string(m_computeShaderThreadMapping)));
	m_material = materialM->buildMaterial(move(string("MaterialCameraVisibleVoxel")), move(string("MaterialCameraVisibleVoxel")), attributeMaterialAddUp);

	m_vectorMaterialName.push_back("MaterialCameraVisibleVoxel");
	m_vectorMaterial.push_back(m_material);

	BBox3D& box             = sceneM->refBox();
	vec3 min3D;
	vec3 max3D;
	box.getCenteredBoxMinMax(min3D, max3D);

	vec3 extent3D = max3D - min3D;
	vec4 min      = vec4(min3D.x,    min3D.y,    min3D.z,    0.0f);
	vec4 max      = vec4(max3D.x,    max3D.y,    max3D.z,    0.0f);
	vec4 extent   = vec4(extent3D.x, extent3D.y, extent3D.z, 0.0f);

	MaterialCameraVisibleVoxel* materialCasted = static_cast<MaterialCameraVisibleVoxel*>(m_material);

	materialCasted->setSceneMin(min);
	materialCasted->setSceneMax(max);
	materialCasted->setSceneExtent(extent);
	
	m_bufferPrefixSumTechnique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_bufferPrefixSumTechnique->refPrefixSumComplete().connect<CameraVisibleVoxelTechnique, &CameraVisibleVoxelTechnique::slotPrefixSumComplete>(this);

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	materialCasted->setVoxelSize(float(technique->getVoxelizedSceneWidth()));

	LitClusterTechnique* m_LitClusterTechnique = static_cast<LitClusterTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("LitClusterTechnique"))));
	m_LitClusterTechnique->refSignalLitClusterCompletion().connect<CameraVisibleVoxelTechnique, &CameraVisibleVoxelTechnique::slotCameraDirty>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraVisibleVoxelTechnique::postCommandSubmit()
{
	uint tempUint = 0;
	m_cameraVisibleCounterBuffer->getContent((void*)(&m_cameraVisibleVoxelNumber));
	m_cameraVisibleCounterBuffer->setContent(&tempUint);
	m_signalCameraVisibleVoxelCompletion.emit();

	//cout << "CameraVisibleVoxelTechnique m_cameraVisibleVoxelNumber=" << m_cameraVisibleVoxelNumber << endl;

	m_active         = false;
	m_executeCommand = false;
	m_needsToRecord  = (m_vectorCommand.size() != m_usedCommandBufferNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraVisibleVoxelTechnique::slotPrefixSumComplete()
{
	m_numOccupiedVoxel = m_bufferPrefixSumTechnique->getFirstIndexOccupiedElement();
	m_bufferNumElement = m_numOccupiedVoxel;

	bufferM->resize(m_cameraVisibleVoxelBuffer, nullptr, m_bufferNumElement * sizeof(uint));
	bufferM->resize(m_cameraVisibleVoxelCompactedBuffer, nullptr, m_bufferNumElement * sizeof(uint));
	bufferM->resize(m_cameraVisibleVoxelDebugBuffer, nullptr, 100 * m_bufferNumElement * sizeof(uint));

	obtainDispatchWorkGroupCount();

	MaterialCameraVisibleVoxel* materialCasted = static_cast<MaterialCameraVisibleVoxel*>(m_material);
	materialCasted->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	materialCasted->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	materialCasted->setNumOccupiedVoxel(m_numOccupiedVoxel);
	materialCasted->setNumThreadExecuted(m_numThreadExecuted);

	m_prefixSumCompleted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraVisibleVoxelTechnique::slotCameraDirty()
{
	if (m_prefixSumCompleted && m_lightBounceOnProgress && (m_mainCamera == cameraM->refMainCamera()))
	{
		// If the main camera is active and there's a camera dirty event while light bounce
		// computations ongoing, set a flag to do an extra last pass to properly update light bounce for
		// visible voxels from camera
		m_cameraDirtyWhileComputation = true;
	}

	if (m_prefixSumCompleted && !m_lightBounceOnProgress)
	{
		MaterialCameraVisibleVoxel* materialCasted = static_cast<MaterialCameraVisibleVoxel*>(m_material);
		mat4 viewprojectionMatrix                  = m_mainCamera->getProjection() * m_mainCamera->getView();
		m_cameraPosition                           = m_mainCamera->getPosition();
		m_cameraForward                            = m_mainCamera->getLookAt();

		materialCasted->setShadowViewProjection(viewprojectionMatrix);
		materialCasted->setLightPosition(vec4(m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z, 0.0f));
		materialCasted->setLightForwardEmitterRadiance(vec4(m_cameraForward.x, m_cameraForward.y, m_cameraForward.z, 0.0f));

		m_lightBounceOnProgress = true;
		m_active                = true;
	}

	if (!m_lightBounceOnProgress)
	{
		cout << "Tried new pass in CameraVisibleVoxelTechnique" << endl;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void CameraVisibleVoxelTechnique::showVisibleVoxelData()
{
	vectorUint8 vectorCameraVisibleVoxelBuffer;

	m_cameraVisibleVoxelCompactedBuffer->getContentCopy(vectorCameraVisibleVoxelBuffer);

	uint* pCameraVisibleVoxelBuffer       = (uint*)(vectorCameraVisibleVoxelBuffer.data());
	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	int voxelizationSize                  = technique->getVoxelizedSceneWidth();

	uvec3 coordinates;

	forI(m_cameraVisibleVoxelNumber)
	{
		coordinates = BufferVerificationHelper::unhashValue(pCameraVisibleVoxelBuffer[i], voxelizationSize);
		if (coordinates == uvec3(43, 75, 56))
		{
			cout << "THE VOXEL IS VISIBLE" << endl;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
