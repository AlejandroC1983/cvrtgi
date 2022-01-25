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
#include "../../include/rastertechnique/clusterizationmergeclustertechnique.h"
#include "../../include/rastertechnique/clusterizationcomputeneighbourtechnique.h"
#include "../../include/rastertechnique/clusterizationbuildfinalbuffertechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialclusterizationmergeclusters.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/util/vulkanstructinitializer.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES
#define IRRADIANCE_FIELD_GRID_DENSITY 5
#define MAX_FIELD_OFFSET 2

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterizationMergeClusterTechnique::ClusterizationMergeClusterTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_materialClusterizationMergeClusters(nullptr)
	, m_voxelizationSize(-1)
	, m_clusterizationNeighbourDebugBuffer(nullptr)
	, m_clusterizationMergeClustersDebugBuffer(nullptr)
	, m_irradianceFieldGridDensity(IRRADIANCE_FIELD_GRID_DENSITY)
	, m_maxIrradianceFieldOffset(MAX_FIELD_OFFSET)
	, m_numberIrradianceField(0)
{
	// If specific information for the limits for irradiance field, take those values
	m_irradianceFieldMinCoordinate = ivec3(
		gpuPipelineM->getRasterFlagValue(move(string("IRRADIANCE_FIELD_MIN_COORDINATE_X"))),
		gpuPipelineM->getRasterFlagValue(move(string("IRRADIANCE_FIELD_MIN_COORDINATE_Y"))),
		gpuPipelineM->getRasterFlagValue(move(string("IRRADIANCE_FIELD_MIN_COORDINATE_Z"))));

	m_irradianceFieldMaxCoordinate = ivec3(
		gpuPipelineM->getRasterFlagValue(move(string("IRRADIANCE_FIELD_MAX_COORDINATE_X"))),
		gpuPipelineM->getRasterFlagValue(move(string("IRRADIANCE_FIELD_MAX_COORDINATE_Y"))),
		gpuPipelineM->getRasterFlagValue(move(string("IRRADIANCE_FIELD_MAX_COORDINATE_Z"))));

	// Otherwise, use scene voxelization size to avoid any limit
	if (m_irradianceFieldMinCoordinate == ivec3(-1, -1, -1))
	{
		m_irradianceFieldMinCoordinate = ivec3(0, 0, 0);
	}

	if (m_irradianceFieldMaxCoordinate == ivec3(-1, -1, -1))
	{
		SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
		m_irradianceFieldMaxCoordinate = ivec3(
			technique->getVoxelizedSceneWidth(),
			technique->getVoxelizedSceneHeight(),
			technique->getVoxelizedSceneDepth());
	}

	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationMergeClusterTechnique::init()
{
	uint tempInitialize = 0;

	m_clusterizationMergeClustersDebugBuffer = bufferM->buildBuffer(
		move(string("clusterizationMergeClustersDebugBuffer")),
		(void*)(&tempInitialize),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Assuming each thread will take care of a whole row / column
	buildShaderThreadMapping();
	
	MultiTypeUnorderedMap* attributeMaterialMergeClusters = new MultiTypeUnorderedMap();
	attributeMaterialMergeClusters->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_voxelClusterizationMergeClustersCodeChunk), string(m_computeShaderThreadMapping)));
	m_material                                            = materialM->buildMaterial(move(string("MaterialClusterizationMergeClusters")), move(string("MaterialClusterizationMergeClusters")), attributeMaterialMergeClusters);
	m_materialClusterizationMergeClusters                 = static_cast<MaterialClusterizationMergeClusters*>(m_material);
	m_vectorMaterialName.push_back("MaterialClusterizationMergeClusters");
	m_vectorMaterial.push_back(m_materialClusterizationMergeClusters);

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_materialClusterizationMergeClusters->setVoxelizationSize(technique->getVoxelizedSceneWidth());
	m_materialClusterizationMergeClusters->setIrradianceFieldGridDensity(m_irradianceFieldGridDensity);

	m_materialClusterizationMergeClusters->setIrradianceFieldMinCoordinate(ivec4(m_irradianceFieldMinCoordinate.x, m_irradianceFieldMinCoordinate.y, m_irradianceFieldMinCoordinate.z, 0));
	m_materialClusterizationMergeClusters->setIrradianceFieldMaxCoordinate(ivec4(m_irradianceFieldMaxCoordinate.x, m_irradianceFieldMaxCoordinate.y, m_irradianceFieldMaxCoordinate.z, 0));

	m_clusterizationComputeNeighbourTechnique = static_cast<ClusterizationComputeNeighbourTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterizationComputeNeighbourTechnique"))));
	m_clusterizationComputeNeighbourTechnique->refSignalClusterizationComputeNeighbourCompletion().connect<ClusterizationMergeClusterTechnique, &ClusterizationMergeClusterTechnique::slotClusterizationComputeNeighbour>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationMergeClusterTechnique::recordBarriers(VkCommandBuffer* commandBuffer)
{
	VulkanStructInitializer::insertBufferMemoryBarrier(bufferM->getElement(move(string("voxelClusterOwnerIndexBuffer"))),
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   commandBuffer);

	VulkanStructInitializer::insertBufferMemoryBarrier(bufferM->getElement(move(string("clusterizationFinalBuffer"))),
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													   commandBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationMergeClusterTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;

	m_signalClusterizationMergeClusterCompletion.emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterizationMergeClusterTechnique::slotClusterizationComputeNeighbour()
{
	m_active                                           = true;
	ClusterizationBuildFinalBufferTechnique* technique = static_cast<ClusterizationBuildFinalBufferTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterizationBuildFinalBufferTechnique"))));
	int compactedClusterNumber                         = technique->getCompactedClusterNumber();
	m_bufferNumElement                                 = compactedClusterNumber;

	obtainDispatchWorkGroupCount();
	m_materialClusterizationMergeClusters->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	m_materialClusterizationMergeClusters->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	m_materialClusterizationMergeClusters->setNumThreadExecuted(m_numThreadExecuted);
	m_materialClusterizationMergeClusters->setNumCluster(compactedClusterNumber);

	// TODO: delete / set size to minimum when done with debug
	bufferM->resize(m_clusterizationMergeClustersDebugBuffer, nullptr, m_bufferNumElement * sizeof(uint) * 2500);
}

/////////////////////////////////////////////////////////////////////////////////////////////
