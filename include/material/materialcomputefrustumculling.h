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

#ifndef _MATERIALCOMPUTEFRUSTUMCULLING_H_
#define _MATERIALCOMPUTEFRUSTUMCULLING_H_

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

class MaterialComputeFrustumCulling : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialComputeFrustumCulling(string &&name) : Material(move(name), move(string("MaterialComputeFrustumCulling")))
		, m_localWorkGroupsXDimension(0)
		, m_localWorkGroupsYDimension(0)
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

		void* shaderFirstPartCode  = InputOutput::readFile("../data/vulkanshaders/computefrustumcullingfirstpart.comp", &sizeFirstPart);
		void* shaderSecondPartCode = InputOutput::readFile("../data/vulkanshaders/computefrustumcullingsecondpart.comp", &sizeSecondPart);

		assert(shaderFirstPartCode  != nullptr);
		assert(shaderSecondPartCode != nullptr);

		string firstPart     = string((const char*)shaderFirstPartCode);
		string secondPart    = string((const char*)shaderSecondPartCode);
		string shaderCode    = firstPart;
		shaderCode          += m_computeShaderThreadMapping;
		shaderCode          += secondPart;
		m_shaderResourceName = "computefrustumculling";
		m_shader             = shaderM->buildShaderC(move(string(m_shaderResourceName)), shaderCode.c_str(), m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsXDimension),       move(string("myMaterialData")), move(string("localWorkGroupsXDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_localWorkGroupsYDimension),       move(string("myMaterialData")), move(string("localWorkGroupsYDimension")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numThreadExecuted),               move(string("myMaterialData")), move(string("numThreadExecuted")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_padding),                         move(string("myMaterialData")), move(string("padding")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneLeftMainCamera),      move(string("myMaterialData")), move(string("frustumPlaneLeftMainCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneRightMainCamera),     move(string("myMaterialData")), move(string("frustumPlaneRightMainCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneTopMainCamera),       move(string("myMaterialData")), move(string("frustumPlaneTopMainCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneBottomMainCamera),    move(string("myMaterialData")), move(string("frustumPlaneBottomMainCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneBackMainCamera),      move(string("myMaterialData")), move(string("frustumPlaneBackMainCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneFrontMainCamera),     move(string("myMaterialData")), move(string("frustumPlaneFrontMainCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneLeftEmitterCamera),   move(string("myMaterialData")), move(string("frustumPlaneLeftEmitterCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneRightEmitterCamera),  move(string("myMaterialData")), move(string("frustumPlaneRightEmitterCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneTopEmitterCamera),    move(string("myMaterialData")), move(string("frustumPlaneTopEmitterCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneBottomEmitterCamera), move(string("myMaterialData")), move(string("frustumPlaneBottomEmitterCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneBackEmitterCamera),   move(string("myMaterialData")), move(string("frustumPlaneBackEmitterCamera")));
		exposeStructField(ResourceInternalType::RIT_FLOAT_VEC4,   (void*)(&m_frustumPlaneFrontEmitterCamera),  move(string("myMaterialData")), move(string("frustumPlaneFrontEmitterCamera")));

		assignShaderStorageBuffer(move(string("instanceDataBuffer")),                       move(string("instanceDataBuffer")),                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("frustumElementCounterMainCameraBuffer")),    move(string("frustumElementCounterMainCameraBuffer")),    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("frustumElementCounterEmitterCameraBuffer")), move(string("frustumElementCounterEmitterCameraBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("frustumDebugBuffer")),                       move(string("frustumDebugBuffer")),                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("indirectCommandBufferMainCamera")),          move(string("indirectCommandBufferMainCamera")),          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("indirectCommandBufferEmitterCamera")),       move(string("indirectCommandBufferEmitterCamera")),       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	SET(string, m_computeShaderThreadMapping, ComputeShaderThreadMapping)
	GETCOPY_SET(uint, m_localWorkGroupsXDimension, LocalWorkGroupsXDimension)
	GETCOPY_SET(uint, m_localWorkGroupsYDimension, LocalWorkGroupsYDimension)
	SET(uint, m_numThreadExecuted, NumThreadExecuted)
	SET(vec4, m_frustumPlaneLeftMainCamera, FrustumPlaneLeftMainCamera)
	SET(vec4, m_frustumPlaneRightMainCamera, FrustumPlaneRightMainCamera)
	SET(vec4, m_frustumPlaneTopMainCamera, FrustumPlaneTopMainCamera)
	SET(vec4, m_frustumPlaneBottomMainCamera, FrustumPlaneBottomMainCamera)
	SET(vec4, m_frustumPlaneBackMainCamera, FrustumPlaneBackMainCamera)
	SET(vec4, m_frustumPlaneFrontMainCamera, FrustumPlaneFrontMainCamera)
	SET(vec4, m_frustumPlaneLeftEmitterCamera, FrustumPlaneLeftEmitterCamera)
	SET(vec4, m_frustumPlaneRightEmitterCamera, FrustumPlaneRightEmitterCamera)
	SET(vec4, m_frustumPlaneTopEmitterCamera, FrustumPlaneTopEmitterCamera)
	SET(vec4, m_frustumPlaneBottomEmitterCamera, FrustumPlaneBottomEmitterCamera)
	SET(vec4, m_frustumPlaneBackEmitterCamera, FrustumPlaneBackEmitterCamera)
	SET(vec4, m_frustumPlaneFrontEmitterCamera, FrustumPlaneFrontEmitterCamera)

protected:
	string m_computeShaderThreadMapping;      //!< String with the thread mapping taking into account m_bufferNumElement, m_numElementPerLocalWorkgroupThread and m_numThreadPerLocalWorkgroup values (and physical device limits)
	uint   m_localWorkGroupsXDimension;       //!< Number of dispatched local workgroups in the x dimension
	uint   m_localWorkGroupsYDimension;       //!< Number of dispatched local workgroups in the y dimension
	uint   m_numThreadExecuted;               //!< Number of threads executed
	uint   m_padding;                         //!< Just foor padding to have ve4 alignment in the shader
	vec4   m_frustumPlaneLeftMainCamera;      //!< Frustum plane for the test
	vec4   m_frustumPlaneRightMainCamera;     //!< Frustum plane for the test
	vec4   m_frustumPlaneTopMainCamera;       //!< Frustum plane for the test
	vec4   m_frustumPlaneBottomMainCamera;    //!< Frustum plane for the test
	vec4   m_frustumPlaneBackMainCamera;      //!< Frustum plane for the test
	vec4   m_frustumPlaneFrontMainCamera;     //!< Frustum plane for the test
	vec4   m_frustumPlaneLeftEmitterCamera;   //!< Frustum plane for the test
	vec4   m_frustumPlaneRightEmitterCamera;  //!< Frustum plane for the test
	vec4   m_frustumPlaneTopEmitterCamera;    //!< Frustum plane for the test
	vec4   m_frustumPlaneBottomEmitterCamera; //!< Frustum plane for the test
	vec4   m_frustumPlaneBackEmitterCamera;   //!< Frustum plane for the test
	vec4   m_frustumPlaneFrontEmitterCamera;  //!< Frustum plane for the test
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALCOMPUTEFRUSTUMCULLING_H_
