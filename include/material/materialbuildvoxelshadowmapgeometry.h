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

#ifndef _MATERIALBUILDSHADOWMAPPINGGEOMETRY_H_
#define _MATERIALBUILDSHADOWMAPPINGGEOMETRY_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/shader/shadermanager.h"
#include "../../include/shader/shader.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/buffer/buffer.h"
#include "../../include/material/materialscenevoxelization.h"
#include "../../include/scene/scene.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class MaterialBuildVoxelShadowMapGeometry : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialBuildVoxelShadowMapGeometry(string &&name) : Material(move(name), move(string("MaterialBuildVoxelShadowMapGeometry")))
		, m_localWorkGroupsXDimension(0)
		, m_localWorkGroupsYDimension(0)
		, m_numElement(0)
		, m_dummyValue(0)
		
	{
		m_resourcesUsed = MaterialBufferResource::MBR_MATERIAL;

		SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
		m_voxelizationSize                    = vec4(float(technique->getVoxelizedSceneWidth()), technique->getVoxelizedSceneHeight(), technique->getVoxelizedSceneDepth(), 0.0f);
		BBox3D& sceneAABB                     = sceneM->refBox();

		vec3 min3D;
		vec3 max3D;
		sceneAABB.getCenteredBoxMinMax(min3D, max3D);

		vec3 extent3D = max3D - min3D;
		m_sceneMin    = vec4(min3D.x,    min3D.y,    min3D.z,    0.0f);
		vec4 max      = vec4(max3D.x,    max3D.y,    max3D.z,    0.0f);
		m_sceneExtent = vec4(extent3D.x, extent3D.y, extent3D.z, 0.0f);
		m_voxelSize   = vec4(m_sceneExtent.x / m_voxelizationSize.x, m_sceneExtent.y / m_voxelizationSize.y, m_sceneExtent.z / m_voxelizationSize.z, 0.0f);
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeFirstPart;
		size_t sizeSecondPart;

		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/buildvoxelshadowmapgeometryfirstpart.comp",  &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/buildvoxelshadowmapgeometrysecondpart.comp", &sizeSecondPart);

		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);

		string firstPart     = string((const char*)shaderFirstPartCode);
		string secondPart    = string((const char*)shaderSecondPartCode);
		string shaderCode    = firstPart;
		shaderCode          += m_computeShaderThreadMapping;
		shaderCode          += secondPart;
		m_shaderResourceName = "buildvoxelshadowmapgeometry";
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
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_voxelizationSize),          move(string("myMaterialData")), move(string("voxelizationSize")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneMin),                  move(string("myMaterialData")), move(string("sceneMin")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_sceneExtent),               move(string("myMaterialData")), move(string("sceneExtent")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_voxelSize),                 move(string("myMaterialData")), move(string("voxelSize")));

		assignShaderStorageBuffer(move(string("voxelHashedPositionCompactedBuffer")), move(string("voxelHashedPositionCompactedBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("shadowMapGeometryVertexBuffer")),      move(string("shadowMapGeometryVertexBuffer")),      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("vertexCounterBuffer")),                move(string("vertexCounterBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelOccupiedBuffer")),                move(string("voxelOccupiedBuffer")),                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("voxelShadowMapGeometryDebugBuffer")),  move(string("voxelShadowMapGeometryDebugBuffer")),  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_numElement, NumElement)
	SET(uint, m_dummyValue, DummyValue)

protected:
	string m_computeShaderThreadMapping; //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	uint   m_localWorkGroupsXDimension;  //!< Number of dispatched local workgroups in the x dimension
	uint   m_localWorkGroupsYDimension;  //!< Number of dispatched local workgroups in the y dimension
	uint   m_numElement;                 //!< Number of elements to process
	uint   m_dummyValue;                 //!< Padding dummy alue to match 4 32-bit element sets
	vec4   m_voxelizationSize;           //!< Size of the texture used for voxelization
	vec4   m_sceneMin;                   //!< Scene minimum value
	vec4   m_sceneExtent;                //!< Extent of the scene. Also, w value is used to distinguish between rasterization of reflectance or normal information
	vec4   m_voxelSize;                  //!< Voxel size in world space taking into account a scene that might have an AABB with different values in the x, y and z axes.
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALBUILDSHADOWMAPPINGGEOMETRY_H_
