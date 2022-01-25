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

#ifndef _MATERIALCAMERAVISIBLEVOXEL_H_
#define _MATERIALCAMERAVISIBLEVOXEL_H_

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

class MaterialCameraVisibleVoxel: public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialCameraVisibleVoxel(string &&name) : Material(move(name), move(string("MaterialCameraVisibleVoxel")))
		, m_sceneMin(vec4(0.0f))
		, m_sceneMax(vec4(0.0f))
		, m_sceneExtent(vec4(0.0f))
		, m_localWorkGroupsXDimension(0)
		, m_localWorkGroupsYDimension(0)
		, m_numOccupiedVoxel(0)
		, m_voxelSize(0.0f)
		, m_numThreadExecuted(0)
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

		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/cameravisiblevoxelfirstpart.comp",  &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/cameravisiblevoxelsecondpart.comp", &sizeSecondPart);

		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);

		string firstPart  = string((const char*)shaderFirstPartCode);
		string secondPart = string((const char*)shaderSecondPartCode);
		string shaderCode = firstPart;
		shaderCode       += m_computeShaderThreadMapping;
		shaderCode       += secondPart;

		m_shaderResourceName = "cameravisiblevoxel";
		m_shader             = shaderM->buildShaderC(move(string(m_shaderResourceName)), shaderCode.c_str(), m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_FLOAT_MAT4,   (void*)(&m_shadowViewProjection),        move(string("myMaterialData")), move(string("shadowViewProjection")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneMin),                    move(string("myMaterialData")), move(string("sceneMin")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneMax),                    move(string("myMaterialData")), move(string("sceneMax")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneExtent),                 move(string("myMaterialData")), move(string("sceneExtent")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_lightPosition),               move(string("myMaterialData")), move(string("lightPosition")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsXDimension),   move(string("myMaterialData")), move(string("localWorkGroupsXDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsYDimension),   move(string("myMaterialData")), move(string("localWorkGroupsYDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numOccupiedVoxel),            move(string("myMaterialData")), move(string("numOccupiedVoxel")));
		exposeStructField(ResourceInternalType::RIT_FLOAT,        (void*)(&m_voxelSize),                   move(string("myMaterialData")), move(string("voxelSize")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_lightForwardEmitterRadiance), move(string("myMaterialData")), move(string("lightForwardEmitterRadiance")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numThreadExecuted),           move(string("myMaterialData")), move(string("numThreadExecuted")));
		
		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")), move(string("voxelHashedPositionCompactedBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelOccupiedBuffer")),                move(string("voxelOccupiedBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("cameraVisibleVoxelDebugBuffer")),      move(string("cameraVisibleVoxelDebugBuffer")),      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("cameraVisibleVoxelBuffer")),           move(string("cameraVisibleVoxelBuffer")),           VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("cameraVisibleVoxelCompactedBuffer")),  move(string("cameraVisibleVoxelCompactedBuffer")),  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("cameraVisibleCounterBuffer")),         move(string("cameraVisibleCounterBuffer")),         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionIndexBuffer")),             move(string("IndirectionIndexBuffer")),             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionRankBuffer")),              move(string("IndirectionRankBuffer")),              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		//assignShaderStorageBuffer(move(string("lightBounceVoxelIrradianceBuffer")),   move(string("lightBounceVoxelIrradianceBuffer")),   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("lightBounceProcessedVoxelBuffer")),    move(string("lightBounceProcessedVoxelBuffer")),    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		assignTextureToSampler(move(string("distanceShadowMappingTexture")), move(string("distanceShadowMappingTexture")), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_NEAREST);
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	SET(mat4, m_shadowViewProjection, ShadowViewProjection)
	SET(vec4, m_sceneMin, SceneMin)
	SET(vec4, m_sceneMax, SceneMax)
	SET(vec4, m_sceneExtent, SceneExtent)
	SET(vec4, m_lightPosition, LightPosition)
	SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_numOccupiedVoxel, NumOccupiedVoxel)
	SET(float, m_voxelSize, VoxelSize)
	SET(vec4, m_lightForwardEmitterRadiance, LightForwardEmitterRadiance)
	SET(uint, m_numThreadExecuted, NumThreadExecuted)

protected:
	string  m_computeShaderThreadMapping;  //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	mat4    m_shadowViewProjection;        //!< Shadow mapping view projection matrix
	vec4    m_sceneMin;                    //!< Minimum value of the scene's aabb
	vec4    m_sceneMax;                    //!< Maximum value of the scene's aabb
	vec4    m_sceneExtent;                 //!< Extent of the scene
	vec4    m_lightPosition;               //!< Light position in world coordinates
	uint    m_localWorkGroupsXDimension;   //!< Number of dispatched local workgroups in the x dimension
	uint    m_localWorkGroupsYDimension;   //!< Number of dispatched local workgroups in the y dimension
	uint    m_numOccupiedVoxel;            //!< Number of occupied voxels
	float   m_voxelSize;                   //!< 3D voxel texture size
	vec4    m_lightForwardEmitterRadiance; //!< Light forward direction in xyz components, emitter radiance in w component
	uint    m_numThreadExecuted;           //!< Number of threads executed by the whole dispatch
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALCAMERAVISIBLEVOXEL_H_
