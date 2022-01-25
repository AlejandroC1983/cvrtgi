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

#ifndef _MATERIALCLUSTERIZATIONADDUP_H_
#define _MATERIALCLUSTERIZATIONADDUP_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/material/materialscenevoxelization.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MaterialClusterizationAddUp: public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialClusterizationAddUp(string &&name) : Material(move(name), move(string("MaterialClusterizationAddUp")))
	, m_localWorkGroupsXDimension(0)
	, m_localWorkGroupsYDimension(0)
	, m_superPixelNumber(0)
	, m_numOccupiedVoxel(0)
	, m_voxelSize(0)
	, m_dummyValue0(0)
	, m_dummyValue1(0)
	, m_dummyValue2(0)
	{
		m_resourcesUsed = MaterialBufferResource::MBR_MATERIAL;
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeFirstPart;
		size_t sizeSecondPart;

		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/clusterizationaddupfirstpart.comp",  &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/clusterizationaddupsecondpart.comp", &sizeSecondPart);

		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);

		string firstPart     = string((const char*)shaderFirstPartCode);
		string secondPart    = string((const char*)shaderSecondPartCode);
		string shaderCode    = firstPart;
		shaderCode          += m_computeShaderThreadMapping;
		shaderCode          += secondPart;
		m_shaderResourceName = "clusterizationaddup";
		m_shader             = shaderM->buildShaderC(move(string(m_shaderResourceName)), shaderCode.c_str(), m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsXDimension), move(string("myMaterialData")), move(string("localWorkGroupsXDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsYDimension), move(string("myMaterialData")), move(string("localWorkGroupsYDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_superPixelNumber),          move(string("myMaterialData")), move(string("superPixelNumber")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numOccupiedVoxel),          move(string("myMaterialData")), move(string("numOccupiedVoxel")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_voxelSize),                 move(string("myMaterialData")), move(string("voxelSize")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_dummyValue0),               move(string("myMaterialData")), move(string("dummyValue0")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_dummyValue1),               move(string("myMaterialData")), move(string("dummyValue1")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_dummyValue2),               move(string("myMaterialData")), move(string("dummyValue2")));

		assignShaderStorageBuffer(move(string("voxelClusterOwnerIndexBuffer")),                move(string("voxelClusterOwnerIndexBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")),          move(string("voxelHashedPositionCompactedBuffer")),          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("meanNormalBuffer")),                            move(string("meanNormalBuffer")),                            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterCoordinatesBuffer")),       move(string("clusterizationCenterCoordinatesBuffer")),       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterPositiveDirectionBuffer")), move(string("clusterizationCenterPositiveDirectionBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterNegativeDirectionBuffer")), move(string("clusterizationCenterNegativeDirectionBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterCountsBuffer")),            move(string("clusterizationCenterCountsBuffer")),            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationAddUpDebugBuffer")),              move(string("clusterizationAddUpDebugBuffer")),              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_superPixelNumber, SuperPixelNumber)
	SET(uint, m_numOccupiedVoxel, NumOccupiedVoxel)
	SET(uint, m_voxelSize, VoxelSize)

protected:
	string m_computeShaderThreadMapping; //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	uint   m_localWorkGroupsXDimension;  //!< Number of dispatched local workgroups in the x dimension
	uint   m_localWorkGroupsYDimension;  //!< Number of dispatched local workgroups in the y dimension
	uint   m_superPixelNumber;           //!< Number of super pixel (clusters)
	uint   m_numOccupiedVoxel;           //!< Number of occupied voxel
	uint   m_voxelSize;                  //!< Voxelization 3D texture size
	uint   m_dummyValue0;                //!< dummy value for 4 32-bit allignment
	uint   m_dummyValue1;                //!< dummy value for 4 32-bit allignment
	uint   m_dummyValue2;                //!< dummy value for 4 32-bit allignment
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALCLUSTERIZATIONADDUP_H_
