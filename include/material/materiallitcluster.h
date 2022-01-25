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

#ifndef _MATERIALLITCLUSTER_H_
#define _MATERIALLITCLUSTER_H_

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

class MaterialLitCluster: public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialLitCluster(string &&name) : Material(move(name), move(string("MaterialLitCluster")))
		, m_sceneMin(vec4(0.0f))
		, m_sceneMax(vec4(0.0f))
		, m_sceneExtent(vec4(0.0f))
		, m_localWorkGroupsXDimension(0)
		, m_localWorkGroupsYDimension(0)
		, m_numOccupiedVoxel(0)
		, m_voxelSize(0.0f)
		, m_numThreadExecuted(0)
		, m_numAddUpElementPerThread(0)
		, m_numAddUpStepThread(0)
		, m_numCluster(0)
		, m_numElementPerLocalWorkgroupThread(0)
		, m_numThreadPerLocalWorkgroup(0)
		, m_localSizeX(1)
		, m_localSizeY(1)
	{
		m_resourcesUsed                     = MaterialBufferResource::MBR_MATERIAL;
		m_numElementPerLocalWorkgroupThread = 1;
		m_numThreadPerLocalWorkgroup        = 64;

		buildShaderThreadMapping();
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeFirstPart;
		size_t sizeSecondPart;

		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/litclusterfirstpart.comp",  &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/litclustersecondpart.comp", &sizeSecondPart);

		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);

		string firstPart  = string((const char*)shaderFirstPartCode);
		string secondPart = string((const char*)shaderSecondPartCode);
		string shaderCode = firstPart;
		shaderCode       += m_computeShaderThreadMapping;
		shaderCode       += secondPart;

		m_shaderResourceName = "litcluster";
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
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numAddUpElementPerThread),    move(string("myMaterialData")), move(string("numAddUpElementPerThread")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numAddUpStepThread),          move(string("myMaterialData")), move(string("numAddUpStepThread")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numCluster),                  move(string("myMaterialData")), move(string("numCluster")));
		
		assignShaderStorageBuffer(move(string("IndirectionIndexBuffer")),             move(string("IndirectionIndexBuffer")),             VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("IndirectionRankBuffer")),              move(string("IndirectionRankBuffer")),              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")), move(string("voxelHashedPositionCompactedBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("litTestClusterBuffer")),               move(string("litTestClusterBuffer")),               VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("litClusterDebugBuffer")),              move(string("litClusterDebugBuffer")),              VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelClusterOwnerIndexBuffer")),       move(string("voxelClusterOwnerIndexBuffer")),       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterizationFinalBuffer")),          move(string("clusterizationFinalBuffer")),          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("litTestVoxelBuffer")),                 move(string("litTestVoxelBuffer")),                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelOccupiedBuffer")),                move(string("voxelOccupiedBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("lightBounceVoxelIrradianceBuffer")),   move(string("lightBounceVoxelIrradianceBuffer")),   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("lightBounceProcessedVoxelBuffer")),    move(string("lightBounceProcessedVoxelBuffer")),    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		//assignTextureToSampler(move(string("shadowmappingvoxeltexture")), move(string("shadowmappingvoxeltexture")), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_NEAREST);
		assignTextureToSampler(move(string("mainCameradistanceShadowMappingTexture")), move(string("mainCameradistanceShadowMappingTexture")), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_FILTER_NEAREST);
	}

	/** Set the number of compute workgroups to dispatch in x and y dimension (m_localWorkGroupsXDimension and
	* m_localWorkGroupsYDimension) taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread
	* and m_numThreadPerLocalWorkgroup values (and physical device limits)
	* @param bufferNumElement [in] number of elements to process
	* @return nothing */
	void obtainDispatchWorkGroupCount(uint  bufferNumElement)
	{
		uvec3 maxComputeWorkGroupCount     = coreM->getMaxComputeWorkGroupCount();
		uint maxLocalWorkGroupXDimension   = maxComputeWorkGroupCount.x;
		uint maxLocalWorkGroupYDimension   = maxComputeWorkGroupCount.y;
		uint numElementPerLocalWorkgroup   = m_numThreadPerLocalWorkgroup * m_numElementPerLocalWorkgroupThread;
		m_numThreadExecuted                = uint(ceil(float(bufferNumElement) / float(m_numElementPerLocalWorkgroupThread)));
		float numLocalWorkgroupsToDispatch = ceil(float(bufferNumElement) / float(numElementPerLocalWorkgroup));

		if (numLocalWorkgroupsToDispatch <= maxLocalWorkGroupYDimension)
		{
			m_localWorkGroupsXDimension = 1;
			m_localWorkGroupsYDimension = uint(numLocalWorkgroupsToDispatch);
		}
		else
		{
			float integerPart;
			float fractional            = glm::modf(float(numLocalWorkgroupsToDispatch) / float(maxLocalWorkGroupYDimension), integerPart);
			m_localWorkGroupsXDimension = uint(ceil(integerPart + 1.0f));
			m_localWorkGroupsYDimension = uint(ceil(float(numLocalWorkgroupsToDispatch) / float(m_localWorkGroupsXDimension)));
		}
	}

	/** Set in m_computeShaderThreadMapping the compute shader code for mapping the execution threads
	* @return nothing */
	void buildShaderThreadMapping()
	{
		uvec3 minComputeWorkGroupSize = coreM->getMinComputeWorkGroupSize();
		float tempValue               = float(m_numThreadPerLocalWorkgroup) / float(minComputeWorkGroupSize.y);

		if (tempValue >= 1.0f)
		{
			m_localSizeX = int(ceil(tempValue));
			m_localSizeY = minComputeWorkGroupSize.y;
		}
		else
		{
			m_localSizeX = m_numThreadPerLocalWorkgroup;
			m_localSizeY = 1;
		}

		m_computeShaderThreadMapping += "\n\n";
		m_computeShaderThreadMapping += "layout(local_size_x = " + to_string(m_localSizeX) + ", local_size_y = " + to_string(m_localSizeY) + ", local_size_z = 1) in;\n\n";
		m_computeShaderThreadMapping += "void main()\n";
		m_computeShaderThreadMapping += "{\n";
		m_computeShaderThreadMapping += "\tconst uint ELEMENT_PER_THREAD         = " + to_string(m_numElementPerLocalWorkgroupThread) + ";\n";
		m_computeShaderThreadMapping += "\tconst uint THREAD_PER_LOCAL_WORKGROUP = " + to_string(m_numThreadPerLocalWorkgroup) + ";\n";
		m_computeShaderThreadMapping += "\tconst uint LOCAL_SIZE_X_VALUE         = " + to_string(m_localSizeX) + ";\n";
		m_computeShaderThreadMapping += "\tconst uint LOCAL_SIZE_Y_VALUE         = " + to_string(m_localSizeY) + ";\n";
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	SET(mat4, m_shadowViewProjection, ShadowViewProjection)
	SET(vec4, m_sceneMin, SceneMin)
	SET(vec4, m_sceneMax, SceneMax)
	SET(vec4, m_sceneExtent, SceneExtent)
	SET(vec4, m_lightPosition, LightPosition)
	GETCOPY_SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	GETCOPY_SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_numOccupiedVoxel, NumOccupiedVoxel)
	SET(float, m_voxelSize, VoxelSize)
	SET(vec4, m_lightForwardEmitterRadiance, LightForwardEmitterRadiance)
	GETCOPY_SET(uint, m_numThreadExecuted, NumThreadExecuted)
	SET(uint, m_numAddUpElementPerThread, NumAddUpElementPerThread)
	SET(uint, m_numAddUpStepThread, NumAddUpStepThread)
	SET(uint, m_numCluster, NumCluster)
	GETCOPY_SET(uint, m_numElementPerLocalWorkgroupThread, NumElementPerLocalWorkgroupThread)
	GETCOPY_SET(uint, m_numThreadPerLocalWorkgroup, NumThreadPerLocalWorkgroup)

protected:
	string  m_computeShaderThreadMapping;        //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	mat4    m_shadowViewProjection;              //!< Shadow mapping view projection matrix
	vec4    m_sceneMin;                          //!< Minimum value of the scene's aabb
	vec4    m_sceneMax;                          //!< Maximum value of the scene's aabb
	vec4    m_sceneExtent;                       //!< Extent of the scene
	vec4    m_lightPosition;                     //!< Light position in world coordinates
	uint    m_localWorkGroupsXDimension;         //!< Number of dispatched local workgroups in the x dimension
	uint    m_localWorkGroupsYDimension;         //!< Number of dispatched local workgroups in the y dimension
	uint    m_numOccupiedVoxel;                  //!< Number of occupied voxels
	float   m_voxelSize;                         //!< 3D voxel texture size
	vec4    m_lightForwardEmitterRadiance;       //!< Light forward direction in xyz components, emitter radiance in w component
	uint    m_numThreadExecuted;                 //!< Number of threads executed by the whole dispatch
	uint    m_numAddUpElementPerThread;          //!< Number of elements per thread to process when performing the add up process
	uint    m_numAddUpStepThread;                //!< Number of threads that will take care of the add up step process (one single step before accumulating the final value)
	uint    m_numCluster;                        //!< Number of cluster in the current scene
	uint    m_numElementPerLocalWorkgroupThread; //!< Number of elements per thread in each local workgroup
	uint    m_numThreadPerLocalWorkgroup;        //!< Number of threads per local workgroup
	int     m_localSizeX;                        //!< Compute shader value for local_size_x
	int     m_localSizeY;                        //!< Compute shader value for local_size_x
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALLITCLUSTER_H_
