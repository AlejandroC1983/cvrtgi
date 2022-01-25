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
#include "../../include/rastertechnique/clusterizationinitaabbtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialclusterizationinitaabb.h"
#include "../../include/core/coremanager.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/rastertechnique/clusterizationtechnique.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterizationInitAABBTechnique::ClusterizationInitAABBTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_prefixSumCompleted(false)
	, m_bufferPrefixSumTechnique(nullptr)
	, m_clusterizationBuffer(nullptr)
	, m_materialClusterizationInitAABB(nullptr)
	, m_clusterNumber(-1)
	, m_voxelizationSize(-1)
{
	m_numElementPerLocalWorkgroupThread = 32;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationInitAABBTechnique::init()
{
	m_clusterizationBuffer = bufferM->buildBuffer(
		move(string("clusterizationBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();
	
	MultiTypeUnorderedMap* attributeMaterialAABB = new MultiTypeUnorderedMap();
	attributeMaterialAABB->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationAddUpCodeChunk), string(m_computeShaderThreadMapping)));
	m_material                                   = materialM->buildMaterial(move(string("MaterialClusterizationInitAABB")), move(string("MaterialClusterizationInitAABB")), attributeMaterialAABB);
	m_materialClusterizationInitAABB             = static_cast<MaterialClusterizationInitAABB*>(m_material);
	m_vectorMaterialName.push_back("MaterialClusterizationInitAABB");
	m_vectorMaterial.push_back(m_material);

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_voxelizationSize                    = technique->getVoxelizedSceneWidth();
	m_clusterNumber                       = ClusterizationTechnique::getNumberSuperVoxel(m_voxelizationSize);
	m_materialClusterizationInitAABB->setNumCluster(m_clusterNumber);

	m_bufferPrefixSumTechnique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_bufferPrefixSumTechnique->refPrefixSumComplete().connect<ClusterizationInitAABBTechnique, &ClusterizationInitAABBTechnique::slotPrefixSumComplete>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationInitAABBTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;
	m_signalClusterizationInitAABBCompletion.emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationInitAABBTechnique::slotPrefixSumComplete()
{
	m_active           = true;
	m_bufferNumElement = m_clusterNumber;

	bufferM->resize(m_clusterizationBuffer, nullptr, m_clusterNumber * sizeof(ClusterData));

	obtainDispatchWorkGroupCount();
	m_materialClusterizationInitAABB->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialClusterizationInitAABB->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialClusterizationInitAABB->setNumCluster(m_clusterNumber);
	m_materialClusterizationInitAABB->setNumThreadExecuted(m_numThreadExecuted);

	m_prefixSumCompleted = true;
	m_newPassRequested   = true;
	m_active             = true;
	m_needsToRecord      = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
