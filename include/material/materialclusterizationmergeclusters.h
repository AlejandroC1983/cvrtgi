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

#ifndef _MATERIALCLUSTERIZATIONMERGECLUSTERS_H_
#define _MATERIALCLUSTERIZATIONMERGECLUSTERS_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/scene/scene.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MaterialClusterizationMergeClusters : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialClusterizationMergeClusters(string &&name) : Material(move(name), move(string("MaterialClusterizationMergeClusters")))
		, m_localWorkGroupsXDimension(0)
		, m_localWorkGroupsYDimension(0)
		, m_numThreadExecuted(0)
		, m_numCluster(0)
		, m_voxelizationSize(0)
		, m_padding0(0)
		, m_padding1(0)
		, m_padding2(0)
		, m_irradianceFieldGridDensity(0)
	{
		m_resourcesUsed = MaterialBufferResource::MBR_MATERIAL;

		BBox3D& sceneAABB = sceneM->refBox();

		vec3 min3D;
		vec3 max3D;
		sceneAABB.getCenteredBoxMinMax(min3D, max3D);

		vec3 extent3D = max3D - min3D;
		m_sceneMin    = vec4(min3D.x,    min3D.y,    min3D.z,    0.0f);
		vec4 max      = vec4(max3D.x,    max3D.y,    max3D.z,    0.0f);
		m_sceneExtent = vec4(extent3D.x, extent3D.y, extent3D.z, 0.0f);
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeFirstPart;
		size_t sizeSecondPart;

		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/clusterizationmergeclustersfirstpart.comp", &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/clusterizationmergeclusterssecondpart.comp", &sizeSecondPart);

		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);

		string firstPart     = string((const char*)shaderFirstPartCode);
		string secondPart    = string((const char*)shaderSecondPartCode);
		string shaderCode    = firstPart;
		shaderCode          += m_computeShaderThreadMapping;
		shaderCode          += secondPart;
		m_shaderResourceName = "clusterizationmergeclusters";
		m_shader             = shaderM->buildShaderC(move(string(m_shaderResourceName)), shaderCode.c_str(), m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsXDimension),    move(string("myMaterialData")), move(string("localWorkGroupsXDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsYDimension),    move(string("myMaterialData")), move(string("localWorkGroupsYDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numThreadExecuted),            move(string("myMaterialData")), move(string("numThreadExecuted")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numCluster),                   move(string("myMaterialData")), move(string("numCluster")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_voxelizationSize),             move(string("myMaterialData")), move(string("voxelizationSize")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_padding0),                     move(string("myMaterialData")), move(string("padding0")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_padding1),                     move(string("myMaterialData")), move(string("padding1")));
		exposeStructField(ResourceInternalType::RIT_INT,          (void*)(&m_irradianceFieldGridDensity),   move(string("myMaterialData")), move(string("irradianceFieldGridDensity")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneMin),                     move(string("myMaterialData")), move(string("sceneMin")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneExtent),                  move(string("myMaterialData")), move(string("sceneExtent")));
		exposeStructField(ResourceInternalType::RIT_INT_VEC4,     (void*)(&m_irradianceFieldMinCoordinate), move(string("myMaterialData")), move(string("irradianceFieldMinCoordinate")));
		exposeStructField(ResourceInternalType::RIT_INT_VEC4,     (void*)(&m_irradianceFieldMaxCoordinate), move(string("myMaterialData")), move(string("irradianceFieldMaxCoordinate")));

		assignShaderStorageBuffer(move(string("clusterizationFinalBuffer")),              move(string("clusterizationFinalBuffer")),              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationMergeClustersDebugBuffer")), move(string("clusterizationMergeClustersDebugBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelOccupiedBuffer")),                    move(string("voxelOccupiedBuffer")),                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionIndexBuffer")),                 move(string("IndirectionIndexBuffer")),                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionRankBuffer")),                  move(string("IndirectionRankBuffer")),                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")),     move(string("voxelHashedPositionCompactedBuffer")),     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("meanNormalBuffer")),                       move(string("meanNormalBuffer")),                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationDebugVoxelIndexBuffer")),    move(string("clusterizationDebugVoxelIndexBuffer")),    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelClusterOwnerIndexBuffer")),           move(string("voxelClusterOwnerIndexBuffer")),           VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("fragmentDataBuffer")),                     move(string("fragmentDataBuffer")),                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("nextFragmentIndexBuffer")),                move(string("nextFragmentIndexBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelFirstIndexCompactedBuffer")),         move(string("voxelFirstIndexCompactedBuffer")),         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	GETCOPY_SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	GETCOPY_SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_numThreadExecuted, NumThreadExecuted)
	SET(uint, m_numCluster, NumCluster)
	SET(uint, m_voxelizationSize, VoxelizationSize)
	SET(uint, m_irradianceFieldGridDensity, IrradianceFieldGridDensity)
	SET(ivec4, m_irradianceFieldMinCoordinate, IrradianceFieldMinCoordinate)
	SET(ivec4, m_irradianceFieldMaxCoordinate, IrradianceFieldMaxCoordinate)

protected:
	string m_computeShaderThreadMapping;   //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	uint   m_localWorkGroupsXDimension;    //!< Number of dispatched local workgroups in the x dimension
	uint   m_localWorkGroupsYDimension;    //!< Number of dispatched local workgroups in the y dimension
	uint   m_numThreadExecuted;            //!< Number of threads executed
	uint   m_numCluster;                   //!< Number of occupied clusters to process
	uint   m_voxelizationSize;             //!< Size of the voxelization texture
	uint   m_padding0;                     //!< Padding value to achieve four 4-byte blocks of data in the material buffer
	uint   m_padding1;                     //!< Padding value to achieve four 4-byte blocks of data in the material buffer
	uint   m_padding2;                     //!< Padding value to achieve four 4-byte blocks of data in the material buffer
	vec4   m_sceneMin;                     //!< Minimum value of the scene's aabb
	vec4   m_sceneExtent;                  //!< Extent of the scene
	int    m_irradianceFieldGridDensity;   //!< Density of the irradiance field's grid (value n means one irradiance field each n units in the voxelization volume in all dimensions)
	ivec4  m_irradianceFieldMinCoordinate; //!< Used to limit the activation of irradiance fields inside the voxelization volume, defines the minimum value (closed interval, >=) of the texture coordinates an irradiance field needs to have to be activated by the cluster AABB
	ivec4  m_irradianceFieldMaxCoordinate; //!< Used to limit the activation of irradiance fields inside the voxelization volume, defines the maximum value (closed interval, <=) of the texture coordinates an irradiance field needs to have to be activated by the cluster AABB
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALCLUSTERIZATIONMERGECLUSTERS_H_
