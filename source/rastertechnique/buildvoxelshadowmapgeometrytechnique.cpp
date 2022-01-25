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
#include "../../include/rastertechnique/buildvoxelshadowmapgeometrytechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialbuildvoxelshadowmapgeometry.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

BuildVoxelShadowMapGeometryTechnique::BuildVoxelShadowMapGeometryTechnique(string &&name, string&& className) : BufferProcessTechnique(move(name), move(className))
	, m_techniquePrefixSum(nullptr)
	, m_numOccupiedVoxel(0)
	, m_shadowMapGeometryVertexBuffer(nullptr)
	, m_vertexCounterBuffer(nullptr)
	, m_numUsedVertex(0)
{
	m_numElementPerLocalWorkgroupThread = 1;
	m_numThreadPerLocalWorkgroup        = 64;
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BuildVoxelShadowMapGeometryTechnique::init()
{
	// Shader storage buffer with the indices of the elements present in the buffer litHiddenVoxelBuffer
	buildShaderThreadMapping();

	m_shadowMapGeometryVertexBuffer = bufferM->buildBuffer(
		move(string("shadowMapGeometryVertexBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_vertexCounterBuffer = bufferM->buildBuffer(
		move(string("vertexCounterBuffer")),
		(void*)(&m_numUsedVertex),
		4,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	bufferM->buildBuffer(
		move(string("voxelShadowMapGeometryDebugBuffer")),
		nullptr,
		3 * 1024,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	MultiTypeUnorderedMap* attributeMaterialBuildVoxelShadowMapGeometry = new MultiTypeUnorderedMap();
	attributeMaterialBuildVoxelShadowMapGeometry->newElement<AttributeData<string>*>(new AttributeData<string>(string(g_buildVoxelShadowMapGeometryCodeChunk), string(m_computeShaderThreadMapping)));
	m_material = materialM->buildMaterial(move(string("MaterialBuildVoxelShadowMapGeometry")), move(string("MaterialBuildVoxelShadowMapGeometry")), attributeMaterialBuildVoxelShadowMapGeometry);
	m_vectorMaterialName.push_back("MaterialBuildVoxelShadowMapGeometry");
	m_vectorMaterial.push_back(m_material);

	m_techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
	m_techniquePrefixSum->refPrefixSumComplete().connect<BuildVoxelShadowMapGeometryTechnique, &BuildVoxelShadowMapGeometryTechnique::slotPrefixSumComplete>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BuildVoxelShadowMapGeometryTechnique::postCommandSubmit()
{
	m_executeCommand = false;
	m_active         = false;
	m_needsToRecord  = false;

	m_vertexCounterBuffer->getContent((void*)(&m_numUsedVertex));
	m_signalBuildVoxelShadowMapGeometryCompletion.emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BuildVoxelShadowMapGeometryTechnique::slotPrefixSumComplete()
{
	m_numOccupiedVoxel = m_techniquePrefixSum->getFirstIndexOccupiedElement();
	m_bufferNumElement = m_numOccupiedVoxel;
	m_active           = true;

	obtainDispatchWorkGroupCount();

	MaterialBuildVoxelShadowMapGeometry* materialCasted = static_cast<MaterialBuildVoxelShadowMapGeometry*>(m_material);
	materialCasted->setLocalWorkGroupsXDimension(m_localWorkGroupsXDimension);
	materialCasted->setLocalWorkGroupsYDimension(m_localWorkGroupsYDimension);
	materialCasted->setNumElement(m_numOccupiedVoxel);

	// No index buffer used, the vertex buffer has all vertices of each triangle of each voxel box (12 triangles)
	// TODO: Do two passes to know how many vertices are needed and generate a vertex buffer with the exact needed size
	bufferM->resize(m_shadowMapGeometryVertexBuffer, nullptr, m_numOccupiedVoxel * 108 * sizeof(float));
}

/////////////////////////////////////////////////////////////////////////////////////////////
