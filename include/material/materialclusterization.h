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

#ifndef _MATERIALCLUSTERIZATION_H_
#define _MATERIALCLUSTERIZATION_H_

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

class MaterialClusterization: public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialClusterization(string &&name) : Material(move(name), move(string("MaterialClusterization")))
	, m_localWorkGroupsXDimension(0)
	, m_localWorkGroupsYDimension(0)
	, m_numElement(0)
	, m_dummyValue(0)
	, m_superPixelNumber(0)
	, m_numOccupiedVoxel(0)
	, m_clusterizationStep(0.0f)
	, m_voxelSize(0.0f)
	, m_pushConstantIterationStep(vec4(-1.0f, -1.0f, -1.0f, -1.0f))
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

		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/clusterizationfirstpart.comp",  &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/clusterizationsecondpart.comp", &sizeSecondPart);

		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);

		string firstPart     = string((const char*)shaderFirstPartCode);
		string secondPart    = string((const char*)shaderSecondPartCode);
		string shaderCode    = firstPart;
		shaderCode          += m_computeShaderThreadMapping;
		shaderCode          += secondPart;
		m_shaderResourceName = "clusterization";
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
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numElement),                move(string("myMaterialData")), move(string("numElement")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_dummyValue),                move(string("myMaterialData")), move(string("dummyValue")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_superPixelNumber),          move(string("myMaterialData")), move(string("superPixelNumber")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numOccupiedVoxel),          move(string("myMaterialData")), move(string("numOccupiedVoxel")));
		exposeStructField(ResourceInternalType::RIT_FLOAT,        (void*)(&m_clusterizationStep),        move(string("myMaterialData")), move(string("clusterizationStep")));
		exposeStructField(ResourceInternalType::RIT_FLOAT,        (void*)(&m_voxelSize),                 move(string("myMaterialData")), move(string("voxelSize")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneMin),                  move(string("myMaterialData")), move(string("sceneMin")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneExtent),               move(string("myMaterialData")), move(string("sceneExtent")));

		assignShaderStorageBuffer(move(string("voxelClusterOwnerIndexBuffer")),                move(string("voxelClusterOwnerIndexBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelClusterOwnerDistanceBuffer")),             move(string("voxelClusterOwnerDistanceBuffer")),             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterCoordinatesBuffer")),       move(string("clusterizationCenterCoordinatesBuffer")),       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterPositiveDirectionBuffer")), move(string("clusterizationCenterPositiveDirectionBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterNegativeDirectionBuffer")), move(string("clusterizationCenterNegativeDirectionBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterValueBuffer")),             move(string("clusterizationCenterValueBuffer")),             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationCenterCountsBuffer")),            move(string("clusterizationCenterCountsBuffer")),            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelOccupiedBuffer")),                         move(string("voxelOccupiedBuffer")),                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("meanCurvatureBuffer")),                         move(string("meanCurvatureBuffer")),                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("meanNormalBuffer")),                            move(string("meanNormalBuffer")),                            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationDebugBuffer")),                   move(string("clusterizationDebugBuffer")),                   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionIndexBuffer")),                      move(string("IndirectionIndexBuffer")),                      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionRankBuffer")),                       move(string("IndirectionRankBuffer")),                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")),          move(string("voxelHashedPositionCompactedBuffer")),          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationDebugVoxelIndexBuffer")),         move(string("clusterizationDebugVoxelIndexBuffer")),         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		pushConstantExposeStructField(ResourceInternalType::RIT_FLOAT_VEC4, (void*)(&m_pushConstantIterationStep), move(string("myPushConstant")), move(string("clusterizationStep")));
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_numElement, NumElement)
	SET(uint, m_dummyValue, DummyValue)
	SET(uint, m_superPixelNumber, SuperPixelNumber)
	SET(uint, m_numOccupiedVoxel, NumOccupiedVoxel)
	SET(float, m_clusterizationStep, ClusterizationStep)
	SET(float, m_voxelSize, VoxelSize)
	SET(vec4, m_sceneMin, SceneMin)
	SET(vec4, m_sceneExtent, SceneExtent)
	SET(vec4, m_pushConstantIterationStep, PushConstantIterationStep)

protected:
	string m_computeShaderThreadMapping; //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	uint   m_localWorkGroupsXDimension;  //!< Number of dispatched local workgroups in the x dimension
	uint   m_localWorkGroupsYDimension;  //!< Number of dispatched local workgroups in the y dimension
	uint   m_numElement;                 //!< Number of elements to process
	uint   m_dummyValue;                 //!< Padding dummy alue to match 4 32-bit element sets
	uint   m_superPixelNumber;           //!< Number of superpixels for the clusterization
	uint   m_numOccupiedVoxel;           //!< Number of occupied voxels
	float  m_clusterizationStep;         //!< Clusterization step
	float  m_voxelSize;                  //!< Float value of the voxelization 3D texture size
	vec4   m_sceneMin;                   //!< Scene minimum aabb value
	vec4   m_sceneExtent;                //!< Scene extent aabb value
	vec4   m_pushConstantIterationStep;  //!< Clusterization step exposed as push constant for the shader
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALCLUSTERIZATION_H_
