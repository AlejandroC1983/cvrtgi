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
#include "../../include/rastertechnique/clusterizationpreparetechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialclusterizationprepare.h"
#include "../../include/core/coremanager.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/scene/scene.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/util/bufferverificationhelper.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterizationPrepareTechnique::ClusterizationPrepareTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_prefixSumCompleted(false)
	, m_voxelizationSize(-1)
	, m_numOccupiedVoxel(0)
	, m_bufferPrefixSumTechnique(nullptr)
	, m_meanCurvatureBuffer(nullptr)
	, m_meanNormalBuffer(nullptr)
	, m_clusterizationPrepareDebugBuffer(nullptr)
{
	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationPrepareTechnique::init()
{
	// Shader storage buffer mapping the elements of voxelHashedPositionCompactedBuffer and
	// used to tag if a voxel is visible
	m_meanCurvatureBuffer = bufferM->buildBuffer(
		move(string("meanCurvatureBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_meanNormalBuffer = bufferM->buildBuffer(
		move(string("meanNormalBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationPrepareDebugBuffer = bufferM->buildBuffer(
		move(string("clusterizationPrepareDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();
	
	MultiTypeUnorderedMap* attributeMaterialAddUp = new MultiTypeUnorderedMap();
	attributeMaterialAddUp->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationPrepareCodeChunk), string(m_computeShaderThreadMapping)));
	m_material = materialM->buildMaterial(move(string("MaterialClusterizationPrepare")), move(string("MaterialClusterizationPrepare")), attributeMaterialAddUp);

	m_vectorMaterialName.push_back("MaterialClusterizationPrepare");
	m_vectorMaterial.push_back(m_material);

	BBox3D& box   = sceneM->refBox();
	vec3 max3D;
	box.getCenteredBoxMinMax(m_sceneMin, max3D);

	m_sceneExtent = max3D - m_sceneMin;
	vec4 extent   = vec4(m_sceneExtent.x, m_sceneExtent.y, m_sceneExtent.z, 0.0f);
	vec4 min      = vec4(m_sceneMin.x,    m_sceneMin.y,    m_sceneMin.z,    0.0f);
	vec4 max      = vec4(max3D.x,         max3D.y,         max3D.z,         0.0f);

	MaterialClusterizationPrepare* materialCasted = static_cast<MaterialClusterizationPrepare*>(m_material);

	materialCasted->setSceneMin(min);
	materialCasted->setSceneExtent(extent);
	materialCasted->setSceneMax(max);
	
	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_voxelizationSize = technique->getVoxelizedSceneWidth();
	materialCasted->setVoxelSize(float(m_voxelizationSize));

	m_bufferPrefixSumTechnique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_bufferPrefixSumTechnique->refPrefixSumComplete().connect<ClusterizationPrepareTechnique, &ClusterizationPrepareTechnique::slotPrefixSumComplete>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationPrepareTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;
	m_signalClusterizationPrepareCompletion.emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationPrepareTechnique::slotPrefixSumComplete()
{
	m_active           = true;
	m_numOccupiedVoxel = m_bufferPrefixSumTechnique->getFirstIndexOccupiedElement();
	m_bufferNumElement = m_numOccupiedVoxel;
	int bufferSize     = m_numOccupiedVoxel * sizeof(uint);

	bufferM->resize(m_meanCurvatureBuffer,              nullptr, bufferSize);

	// TODO: optimization use an uint buffer, use pack16 to store 2 3-dimension normal values in 3 uint elements
	bufferM->resize(m_meanNormalBuffer,                 nullptr, bufferSize * 3);
	bufferM->resize(m_clusterizationPrepareDebugBuffer, nullptr, 40000); // Just for 1000 threads

	obtainDispatchWorkGroupCount();

	MaterialClusterizationPrepare* materialCasted = static_cast<MaterialClusterizationPrepare*>(m_material);
	materialCasted->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	materialCasted->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	materialCasted->setNumOccupiedVoxel(m_bufferNumElement);

	m_prefixSumCompleted = true;
	m_newPassRequested   = true;
	m_active             = true;
	m_needsToRecord      = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
