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
#include "../../include/rastertechnique/clusterizationcomputeneighbourtechnique.h"
#include "../../include/rastertechnique/clusterizationbuildfinalbuffertechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialclusterizationcomputeneighbour.h"
#include "../../include/material/materialclusterizationmergeclusters.h"
#include "../../include/core/coremanager.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/util/bufferverificationhelper.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterizationComputeNeighbourTechnique::ClusterizationComputeNeighbourTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_clusterizationBuildFinalBufferTechnique(nullptr)
	, m_materialClusterizationComputeNeighbour(nullptr)
	, m_voxelizationSize(-1)
	, m_clusterizationNeighbourDebugBuffer(nullptr)
{
	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationComputeNeighbourTechnique::init()
{
	m_clusterizationNeighbourDebugBuffer = bufferM->buildBuffer(
		move(string("clusterizationNeighbourDebugBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();
	
	MultiTypeUnorderedMap* attributeMaterialComputeNeighbour = new MultiTypeUnorderedMap();
	attributeMaterialComputeNeighbour->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationComputeNeighbourCodeChunk), string(m_computeShaderThreadMapping)));
	m_material                                               = materialM->buildMaterial(move(string("MaterialClusterizationComputeNeighbour")), move(string("MaterialClusterizationComputeNeighbour")), attributeMaterialComputeNeighbour);
	m_materialClusterizationComputeNeighbour                 = static_cast<MaterialClusterizationComputeNeighbour*>(m_material);
	m_vectorMaterialName.push_back("MaterialClusterizationComputeNeighbour");
	m_vectorMaterial.push_back(m_material);

	m_clusterizationBuildFinalBufferTechnique = static_cast<ClusterizationBuildFinalBufferTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterizationBuildFinalBufferTechnique"))));
	m_clusterizationBuildFinalBufferTechnique->refSignalClusterizationBuildFinalBufferCompletion().connect<ClusterizationComputeNeighbourTechnique, &ClusterizationComputeNeighbourTechnique::slotClusterizationBuildFinalBuffer>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationComputeNeighbourTechnique::recordBarriers(VkCommandBuffer* commandBuffer)
{
	VulkanStructInitializer::insertBufferMemoryBarrier(bufferM->getElement(move(string("clusterizationFinalBuffer"))),
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   commandBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationComputeNeighbourTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;

	m_signalClusterizationComputeNeighbourCompletion.emit();
	
	//BufferVerificationHelper::verifyClusterFinalDataBuffer();
	//BufferVerificationHelper::findMergeCandidateCluster(3);
	BufferVerificationHelper::countNumberVoxelWithNoClusterOwner();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationComputeNeighbourTechnique::slotClusterizationBuildFinalBuffer()
{
	m_active                                           = true;
	ClusterizationBuildFinalBufferTechnique* technique = static_cast<ClusterizationBuildFinalBufferTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterizationBuildFinalBufferTechnique"))));
	int compactedClusterNumber                         = technique->getCompactedClusterNumber();
	m_bufferNumElement                                 = compactedClusterNumber * compactedClusterNumber;

	obtainDispatchWorkGroupCount();
	m_materialClusterizationComputeNeighbour->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialClusterizationComputeNeighbour->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialClusterizationComputeNeighbour->setNumThreadExecuted(m_numThreadExecuted);
	m_materialClusterizationComputeNeighbour->setNumCluster(compactedClusterNumber);
}

/////////////////////////////////////////////////////////////////////////////////////////////
