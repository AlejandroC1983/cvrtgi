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

#ifndef _MATERIALCLUSTERVISIBILITY_H_
#define _MATERIALCLUSTERVISIBILITY_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MaterialClusterVisibility : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialClusterVisibility(string &&name) : Material(move(name), move(string("MaterialClusterVisibility")))
		, m_localWorkGroupsXDimension(0)
		, m_localWorkGroupsYDimension(0)
		, m_numThreadExecuted(0)
		, m_numberThreadPerElement(0)
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
		
		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/clustervisibilityfirstpart.comp", &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/clustervisibilitysecondpart.comp", &sizeSecondPart);
		
		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);
		
		string firstPart     = string((const char*)shaderFirstPartCode);
		string secondPart    = string((const char*)shaderSecondPartCode);
		string shaderCode    = firstPart;
		shaderCode          += m_computeShaderThreadMapping;
		shaderCode          += secondPart;
		m_shaderResourceName = "clustervisibility";
		m_shader             = shaderM->buildShaderC(move(string(m_shaderResourceName)), shaderCode.c_str(), m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsXDimension),   move(string("myMaterialData")), move(string("localWorkGroupsXDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsYDimension),   move(string("myMaterialData")), move(string("localWorkGroupsYDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numThreadExecuted),           move(string("myMaterialData")), move(string("numThreadExecuted")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numberThreadPerElement),      move(string("myMaterialData")), move(string("numberThreadPerElement")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneMinAndNumberVoxel),      move(string("myMaterialData")), move(string("sceneMinAndNumberVoxel")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneExtentAndVoxelSize),     move(string("myMaterialData")), move(string("sceneExtentAndVoxelSize")));
		
		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")), move(string("voxelHashedPositionCompactedBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelOccupiedBuffer")),                move(string("voxelOccupiedBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionIndexBuffer")),             move(string("IndirectionIndexBuffer")),             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionRankBuffer")),              move(string("IndirectionRankBuffer")),              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationFinalBuffer")),          move(string("clusterizationFinalBuffer")),          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelClusterOwnerIndexBuffer")),       move(string("voxelClusterOwnerIndexBuffer")),       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterVisibilityBuffer")),            move(string("clusterVisibilityBuffer")),            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterVisibilityNumberBuffer")),      move(string("clusterVisibilityNumberBuffer")),      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterVisibilityDebugBuffer")),       move(string("clusterVisibilityDebugBuffer")),       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	GETCOPY_SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	GETCOPY_SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_numThreadExecuted, NumThreadExecuted)
	SET(uint, m_numberThreadPerElement, NumberThreadPerElement)
	SET(vec4, m_sceneMinAndNumberVoxel, SceneMinAndNumberVoxel)
	SET(vec4, m_sceneExtentAndVoxelSize, SceneExtentAndVoxelSize)

protected:
	string m_computeShaderThreadMapping;  //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	uint   m_localWorkGroupsXDimension;   //!< Number of dispatched local workgroups in the x dimension
	uint   m_localWorkGroupsYDimension;   //!< Number of dispatched local workgroups in the y dimension
	uint   m_numThreadExecuted;           //!< Number of thread executed
	uint   m_numberThreadPerElement;      //!< Number of threads that process each element
	vec4   m_sceneMinAndNumberVoxel;      //!< Minimum value of the scene's aabb in xyz coordinates, number of occupied voxels
	vec4   m_sceneExtentAndVoxelSize;     //!< Extent of the scene in the xyz coordinates, voxelization texture size in the w coordinate
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALCLUSTERVISIBILITY_H_
