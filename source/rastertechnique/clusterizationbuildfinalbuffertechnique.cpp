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
#include "../../include/rastertechnique/clusterizationbuildfinalbuffertechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialclusterizationbuildfinalbuffer.h"
#include "../../include/material/materialclusterizationcomputeneighbour.h"
#include "../../include/core/coremanager.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/rastertechnique/clusterizationtechnique.h"
#include "../../include/rastertechnique/clusterizationinitaabbtechnique.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterizationBuildFinalBufferTechnique::ClusterizationBuildFinalBufferTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_prefixSumCompleted(false)
	, m_bufferPrefixSumTechnique(nullptr)
	, m_clusterNumber(-1)
	, m_voxelizationSize(-1)
	, m_materialClusterizationBuildFinalBuffer(nullptr)
	, m_clusterCounterBuffer(nullptr)
	, m_clusterizationFinalBuffer(nullptr)
	, m_clusterizationDebugFinalBuffer(nullptr)
	, m_compactedClusterNumber(0)
	, m_clusterCounter(0)
{
	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationBuildFinalBufferTechnique::init()
{
	m_clusterCounterBuffer = bufferM->buildBuffer(
		move(string("clusterCounterBuffer")),
		(void*)(&m_clusterCounter),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationFinalBuffer = bufferM->buildBuffer(
		move(string("clusterizationFinalBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_clusterizationDebugFinalBuffer = bufferM->buildBuffer(
		move(string("clusterizationDebugFinalBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	bufferM->resize(m_clusterizationDebugFinalBuffer, nullptr, 50000 * sizeof(uint) * 10);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();
	
	MultiTypeUnorderedMap* attributeMaterialBuildFinalBuffer = new MultiTypeUnorderedMap();
	attributeMaterialBuildFinalBuffer->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationBuildFinalBufferCodeChunk), string(m_computeShaderThreadMapping)));
	m_material                                               = materialM->buildMaterial(move(string("MaterialClusterizationBuildFinalBuffer")), move(string("MaterialClusterizationBuildFinalBuffer")), attributeMaterialBuildFinalBuffer);
	m_materialClusterizationBuildFinalBuffer                 = static_cast<MaterialClusterizationBuildFinalBuffer*>(m_material);
	m_vectorMaterialName.push_back("MaterialClusterizationBuildFinalBuffer");
	m_vectorMaterial.push_back(m_material);

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_voxelizationSize                    = technique->getVoxelizedSceneWidth();
	m_clusterNumber                       = ClusterizationTechnique::getNumberSuperVoxel(m_voxelizationSize);
	m_materialClusterizationBuildFinalBuffer->setClusterNumber(m_clusterNumber);
	m_materialClusterizationBuildFinalBuffer->setVoxelizationSize(m_voxelizationSize);

	m_bufferPrefixSumTechnique = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_bufferPrefixSumTechnique->refPrefixSumComplete().connect<ClusterizationBuildFinalBufferTechnique, &ClusterizationBuildFinalBufferTechnique::slotPrefixSumComplete>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationBuildFinalBufferTechnique::postCommandSubmit()
{
	if (m_materialClusterizationBuildFinalBuffer->getIsCounting())
	{
		m_materialClusterizationBuildFinalBuffer->setIsCounting(false);
		m_clusterCounterBuffer->getContent(&m_clusterCounter);
		m_compactedClusterNumber = m_clusterCounter;
		m_clusterCounter         = 0;
		m_clusterCounterBuffer->setContent(&m_clusterCounter);
		bufferM->resize(m_clusterizationFinalBuffer,      nullptr, m_compactedClusterNumber * sizeof(ClusterData));
		cout << "ClusterData size " << sizeof(ClusterData) << endl;
		cout << "The number of final occupied clusters is " << m_compactedClusterNumber << ", m_clusterizationFinalBuffer mapping size=" << m_clusterizationFinalBuffer->getMappingSize() << endl;

		// Limit for number of clusters in light bounce shader
		// TODO: Use constant and pass it to shader to avoid duplicated hardcoded values
		//if (m_compactedClusterNumber > 6400)
		//if (m_compactedClusterNumber > 8640)
		//{
		//	cout << "ERROR: more clusters generated that possible bits to map them in light bounce" << endl;
		//	assert(false);
		//}
	}
	else
	{
		m_signalClusterizationBuildFinalBufferCompletion.emit();
		// Either delete clusterizationBuffer or resize it to the smallest possible size
		Buffer* clusterizationBuffer = bufferM->getElement(move(string("clusterizationBuffer")));

		m_executeCommand = false;
		m_active         = false;
		m_needsToRecord  = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationBuildFinalBufferTechnique::slotPrefixSumComplete()
{
	m_active           = true;
	m_bufferNumElement = m_clusterNumber;

	obtainDispatchWorkGroupCount();
	m_materialClusterizationBuildFinalBuffer->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialClusterizationBuildFinalBuffer->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialClusterizationBuildFinalBuffer->setNumThreadExecuted(m_numThreadExecuted);
	m_materialClusterizationBuildFinalBuffer->setIsCounting(true);

	m_prefixSumCompleted = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
