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

#ifndef _MATERIALPREFIXSUM_H_
#define _MATERIALPREFIXSUM_H_

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

class MaterialPrefixSum : public Material
{
	friend class MaterialManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	MaterialPrefixSum(string &&name) : Material(move(name), move(string("MaterialPrefixSum")))
		, m_prefixSumTotalSize(0)
		, m_numElementBase(0)
		, m_numElementLevel0(0)
		, m_numElementLevel1(0)
		, m_numElementLevel2(0)
		, m_numElementLevel3(0)
		, m_currentStep(0)
		, m_numberStepsReduce(0)
		, m_numberStepsDownSweep(0)
		, m_currentPhase(0)
	{
		m_resourcesUsed = MaterialBufferResource::MBR_MATERIAL;
	}

public:
	/** Loads the shader to be used. This could be substituted
	* by reading from a file, where the shader details would be present
	* @return true if the shader was loaded successfully, and false otherwise */
	bool loadShader()
	{
		size_t sizeTemp;
		void* computeShaderCode = InputOutput::readFile("../data/vulkanshaders/prefixsum.comp", &sizeTemp);

		m_shaderResourceName = "prefixsum";
		m_shader = shaderM->buildShaderC(move(string(m_shaderResourceName)), (const char*)computeShaderCode, m_materialSurfaceType);

		return true;
	}

	/** Exposes the variables that will be modified in the corresponding material data uniform buffer used in the scene,
	* allowing to work from variables from C++ without needing to update the values manually (error-prone)
	* @return nothing */
	void exposeResources()
	{
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numElementBase),       move(string("myMaterialData")), move(string("numElementBase")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numElementLevel0),     move(string("myMaterialData")), move(string("numElementLevel0")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numElementLevel1),     move(string("myMaterialData")), move(string("numElementLevel1")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numElementLevel2),     move(string("myMaterialData")), move(string("numElementLevel2")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numElementLevel3),     move(string("myMaterialData")), move(string("numElementLevel3")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_currentStep),          move(string("myMaterialData")), move(string("currentStep")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_currentPhase),         move(string("myMaterialData")), move(string("currentPhase")));
		exposeStructField(ResourceInternalType::RIT_UNSIGNED_INT, (void*)(&m_numberStepsDownSweep), move(string("myMaterialData")), move(string("numberStepsDownSweep")));

		assignShaderStorageBuffer(move(string("prefixSumBuffer")),                  move(string("prefixSumBuffer")),                  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterVisibilityBuffer")),          move(string("clusterVisibilityBuffer")),          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		assignShaderStorageBuffer(move(string("clusterVisibilityCompactedBuffer")), move(string("clusterVisibilityCompactedBuffer")), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	}

	SET(uint, m_prefixSumTotalSize, PrefixSumTotalSize)
	SET(uint, m_numElementBase, NumElementBase)
	SET(uint, m_numElementLevel0, NumElementLevel0)
	SET(uint, m_numElementLevel1, NumElementLevel1)
	SET(uint, m_numElementLevel2, NumElementLevel2)
	SET(uint, m_numElementLevel3, NumElementLevel3)
	SET(uint, m_currentStep, CurrentStep)
	SET(uint, m_numberStepsReduce, NumberStepsReduce)
	SET(uint, m_numberStepsDownSweep, NumberStepsDownSweep)
	SET(uint, m_currentPhase, CurrentPhase)

protected:
	uint m_prefixSumTotalSize;   //!< Total buffer size for the parallel prefix sum algorithm
	uint m_numElementBase;       //!< Number of elements for the base step of the reduce phase of the algorithm (reading from m_voxelFirstIndexBuffer)
	uint m_numElementLevel0;     //!< Number of elements for the step number 0 of the reduce phase of the algorithm
	uint m_numElementLevel1;     //!< Number of elements for the step number 1 of the reduce phase of the algorithm
	uint m_numElementLevel2;     //!< Number of elements for the step number 2 of the reduce phase of the algorithm
	uint m_numElementLevel3;     //!< Number of elements for the step number 3 of the reduce phase of the algorithm
	uint m_currentStep;          //!< Index with the current step being done in the algorithm. At level 0, the number of elements to process is given by prefixSumNumElementBase, and from level one, by prefixSumNumElementLeveli
	uint m_numberStepsReduce;    //!< Number of steps of the algorithm (since the number of elements to apply prefix sum at bufferHistogramBuffer can vary from one execution to another)
	uint m_numberStepsDownSweep; //!< Number of steps of the dow sweep phase of the algorithm (since the number of elements to apply prefix sum at m_voxelFirstIndexBuffer can vary from one execution to another)
	uint m_currentPhase;         //!< 0 means reduction step, 1 sweep down, 2 write final compacted buffer
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MATERIALPREFIXSUM_H_
