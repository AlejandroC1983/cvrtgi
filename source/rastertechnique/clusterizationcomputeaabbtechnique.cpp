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
#include "../../include/rastertechnique/clusterizationcomputeaabbtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialclusterizationcomputeaabb.h"
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

ClusterizationComputeAABBTechnique::ClusterizationComputeAABBTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_prefixSumCompleted(false)
	, m_bufferPrefixSumTechnique(nullptr)
	, m_occupiedVoxelNumber(-1)
	, m_voxelizationSize(-1)
	, m_materialClusterizationComputeAABB(nullptr)
	, m_clusterAABBDebugBuffer(nullptr)
{
	m_numElementPerLocalWorkgroupThread = 32;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationComputeAABBTechnique::init()
{
	m_clusterAABBDebugBuffer = bufferM->buildBuffer(
		move(string("clusterAABBDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();
	
	MultiTypeUnorderedMap* attributeMaterialComputeAABB = new MultiTypeUnorderedMap();
	attributeMaterialComputeAABB->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationComputeAABBCodeChunk), string(m_computeShaderThreadMapping)));
	m_material                                          = materialM->buildMaterial(move(string("MaterialClusterizationComputeAABB")), move(string("MaterialClusterizationComputeAABB")), attributeMaterialComputeAABB);
	m_materialClusterizationComputeAABB                 = static_cast<MaterialClusterizationComputeAABB*>(m_material);
	m_vectorMaterialName.push_back("MaterialClusterizationComputeAABB");
	m_vectorMaterial.push_back(m_material);

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_voxelizationSize                    = technique->getVoxelizedSceneWidth();

	m_bufferPrefixSumTechnique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_bufferPrefixSumTechnique->refPrefixSumComplete().connect<ClusterizationComputeAABBTechnique, &ClusterizationComputeAABBTechnique::slotPrefixSumComplete>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationComputeAABBTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;
	m_signalClusterizationComputeAABBCompletion.emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationComputeAABBTechnique::slotPrefixSumComplete()
{
	m_occupiedVoxelNumber = m_bufferPrefixSumTechnique->getFirstIndexOccupiedElement();
	m_bufferNumElement    = m_occupiedVoxelNumber;

	bufferM->resize(m_clusterAABBDebugBuffer, nullptr, m_occupiedVoxelNumber * sizeof(uint) * 100);

	obtainDispatchWorkGroupCount();
	m_materialClusterizationComputeAABB = static_cast<MaterialClusterizationComputeAABB*>(m_vectorMaterial[0]);
	m_materialClusterizationComputeAABB->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialClusterizationComputeAABB->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialClusterizationComputeAABB->setOccupiedVoxelNumber(m_occupiedVoxelNumber);
	m_materialClusterizationComputeAABB->setNumThreadExecuted(m_numThreadExecuted);
	m_materialClusterizationComputeAABB->setVoxelizationSize(m_voxelizationSize);

	m_prefixSumCompleted = true;
	m_newPassRequested   = true;
    m_active             = true;
	m_needsToRecord      = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
