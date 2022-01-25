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
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/materialbufferprefixsum.h"
#include "../../include/core/coremanager.h"
#include "../../include/rastertechnique/scenevoxelizationtechnique.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/util/bufferverificationhelper.h"

// NAMESPACE

// DEFINES
#define NUM_ELEMENT_ANALYZED_PER_THREAD 128

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

BufferPrefixSumTechnique::BufferPrefixSumTechnique(string &&name, string&& className) : RasterTechnique(move(name), move(className))
	, m_prefixSumPlanarBuffer(nullptr)
	, m_voxelFirstIndexBuffer(nullptr)
	, m_voxelFirstIndexCompactedBuffer(nullptr)
	, m_voxelHashedPositionCompactedBuffer(nullptr)
	, m_voxelFirstIndexEmitterCompactedBuffer(nullptr)
	, m_indirectionIndexBuffer(nullptr)
	, m_indirectionRankBuffer(nullptr)
	, m_lightBounceVoxelIrradianceBuffer(nullptr)
	, m_lightBounceVoxelFilteredIrradianceBuffer(nullptr)
	, m_lightBounceProcessedVoxelBuffer(nullptr)
	, m_firstIndexOccupiedElement(0)
	, m_fragmentOccupiedCounter(0)
	, m_voxelizationSize(0)
	, m_bufferVoxelFirstIndexComplete(false)
	, m_prefixSumPlanarBufferSize(0)
	, m_numElementAnalyzedPreThread(NUM_ELEMENT_ANALYZED_PER_THREAD)
	, m_currentStep(0)
	, m_numberStepsReduce(0)
	, m_numberStepsDownSweep(0)
	, m_currentPhase(0)
	, m_firstSetIsSingleElement(false)
	, m_compactionStepDone(false)
	, m_indirectionBufferRange(0)
	, m_voxelizationWidth(0)
	, m_voxelizationHeight(0)
	, m_voxelizationDepth(0)
	, m_currentStepEnum(PrefixSumStep::PS_REDUCTION)
{
	m_recordPolicy            = CommandRecordPolicy::CRP_SINGLE_TIME;
	m_active                  = false;
	m_needsToRecord           = false;
	m_rasterTechniqueType     = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize  = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferPrefixSumTechnique::init()
{
	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	technique->refVoxelizationComplete().connect<BufferPrefixSumTechnique, &BufferPrefixSumTechnique::slotVoxelizationComplete>(this);

	// Shader storage buffer to store all the levels of the parallel prefix sum algorithm to apply
	// to the m_voxelFirstIndexBuffer buffer
	// The buffer will be re-done with the proper size once m_fragmentOccupiedCounter is known
	m_prefixSumPlanarBuffer = bufferM->buildBuffer(
		move(string("prefixSumPlanarBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Shader storage buffer with the parallel prefix sum algorithm result
	// The buffer will be resized once slotVoxelizationCompleteis called
	m_voxelFirstIndexCompactedBuffer = bufferM->buildBuffer(
		move(string("voxelFirstIndexCompactedBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Shader storage buffer with the hashed position of the 3D volume voxelization
	// coordinates the fragment data in the same index at the m_voxelFirstIndexCompactedBuffer
	// buffer occupied initially in the non-compacted buufer
	// The buffer will be resized once slotVoxelizationCompleteis called
	m_voxelHashedPositionCompactedBuffer = bufferM->buildBuffer(
		move(string("voxelHashedPositionCompactedBuffer")),
		nullptr,
		256,
		//VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Shader storage buffer with the index to the first element of a linked list with all the emitters
	// that have intersected with this voxel (for now, only the index of the emitter in the emitter buffer)
	m_voxelFirstIndexEmitterCompactedBuffer = bufferM->buildBuffer(
		move(string("voxelFirstIndexEmitterCompactedBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_lightBounceVoxelIrradianceBuffer = bufferM->buildBuffer(
		move(string("lightBounceVoxelIrradianceBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_lightBounceVoxelFilteredIrradianceBuffer = bufferM->buildBuffer(
		move(string("lightBounceVoxelFilteredIrradianceBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_lightBounceProcessedVoxelBuffer = bufferM->buildBuffer(
		move(string("lightBounceProcessedVoxelBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT  | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Warning: missing here the possibility to have non-regular in the three dimensions voxelization volumes
	m_voxelizationWidth              = technique->getVoxelizedSceneWidth();
	m_voxelizationHeight             = technique->getVoxelizedSceneHeight();
	m_voxelizationDepth              = technique->getVoxelizedSceneDepth();
	m_indirectionBufferRange         = m_voxelizationWidth;
	uint indirectionBufferNumElement = m_voxelizationWidth * m_voxelizationWidth;

	vector<uint> vectorIndirectionRankBuffer;
	vectorIndirectionRankBuffer.resize(indirectionBufferNumElement);
	memset(vectorIndirectionRankBuffer.data(), maxValue, vectorIndirectionRankBuffer.size() * size_t(sizeof(uint)));

	m_indirectionIndexBuffer         = bufferM->buildBuffer(
		move(string("IndirectionIndexBuffer")),
		vectorIndirectionRankBuffer.data(),
		indirectionBufferNumElement * sizeof(uint),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	memset(vectorIndirectionRankBuffer.data(), 0, vectorIndirectionRankBuffer.size() * size_t(sizeof(uint)));

	m_indirectionRankBuffer = bufferM->buildBuffer(
		move(string("IndirectionRankBuffer")),
		vectorIndirectionRankBuffer.data(),
		indirectionBufferNumElement * sizeof(uint),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_voxelFirstIndexBuffer = bufferM->getElement(move(string("voxelFirstIndexBuffer")));

	m_vectorMaterialName.resize(1);
	m_vectorMaterial.resize(1);
	m_vectorMaterialName[0] = string("MaterialBufferPrefixSum");
	m_vectorMaterial[0]     = materialM->buildMaterial(move(string("MaterialBufferPrefixSum")), move(string("MaterialBufferPrefixSum")), nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* BufferPrefixSumTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	MaterialBufferPrefixSum* material = static_cast<MaterialBufferPrefixSum*>(m_vectorMaterial[0]);

	commandBufferType = CommandBufferType::CBT_COMPUTE_QUEUE;

	VkCommandBuffer* commandBuffer;
	commandBuffer = addRecordedCommandBuffer(commandBufferID);
	addCommandBufferQueueType(commandBufferID, commandBufferType);
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getComputeCommandPool(), commandBuffer);

	coreM->beginCommandBuffer(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex0);
#endif

	// Dispatch the compute shader to build the prefix sum, the buffer m_prefixSumPlanarBuffer
	// has all the required prefix sum level number of elements
	// Several dispatchs can be needed to complete the algorithm (depending on the number of elements
	// in the buffer), the method postQueueSubmit is used to coordinate the dispatchs

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, material->getPipeline()->getPipeline());

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	uint32_t offsetData[3];
	offsetData[0] = 0;
	offsetData[1] = 0;
	offsetData[2] = static_cast<uint32_t>(material->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, material->getPipelineLayout(), 0, 1, &material->refDescriptorSet(), 3, &offsetData[0]);

	uint finalWorkgroupSize;
		
	// 0 means reduction step, 1 sweep down, 2 write final compacted buffer
	if (m_currentPhase == 0)
	{
		finalWorkgroupSize = m_vectorPrefixSumNumElement[m_currentStep];
	}
	else if (m_currentPhase == 1)
	{
		finalWorkgroupSize = uint(ceilf(float(m_vectorPrefixSumNumElement[m_currentStep]) / float(NUM_ELEMENT_ANALYZED_PER_THREAD)));
	}
	else if (m_currentPhase == 2)
	{
		finalWorkgroupSize = m_vectorPrefixSumNumElement[0];
	}

	vkCmdDispatch(*commandBuffer, finalWorkgroupSize, 1, 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);

	// NOTE: Clear command buffer if re-recorded
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferPrefixSumTechnique::postCommandSubmit()
{
	switch (m_currentStepEnum)
	{
		case PrefixSumStep::PS_REDUCTION:
		{
			MaterialBufferPrefixSum* material = static_cast<MaterialBufferPrefixSum*>(m_vectorMaterial[0]);
			if ((m_currentStep + 1) < m_numberStepsReduce)
			{	
				m_currentStep++;
				material->setCurrentStep(m_currentStep);
			}
			else
			{
				//BufferVerificationHelper::verifyPrefixSumData();
				m_currentPhase++;
				material->setCurrentPhase(m_currentPhase);
				if (m_firstSetIsSingleElement)
				{
					m_currentStep--;
					material->setCurrentStep(m_currentStep);
				}

				m_firstIndexOccupiedElement = retrieveAccumulatedNumValues();
				bufferM->resize(m_voxelFirstIndexCompactedBuffer, nullptr, m_firstIndexOccupiedElement * sizeof(uint));
				bufferM->resize(m_voxelHashedPositionCompactedBuffer, nullptr, m_firstIndexOccupiedElement * sizeof(uint));
				vector<uint> vectorData;
				vectorData.resize(m_firstIndexOccupiedElement);
				memset(vectorData.data(), maxValue, vectorData.size() * size_t(sizeof(uint)));
				bufferM->resize(m_voxelFirstIndexEmitterCompactedBuffer, vectorData.data(), m_firstIndexOccupiedElement * sizeof(uint));
				m_currentStepEnum = PrefixSumStep::PS_SWEEPDOWN;
				
				// To store just used fields, 19 elements will be used for each voxel: 3 elements for the xyz irradiance
				// per voxel face (-x,+x,-y,+y,-z,+z), indices (0,1,2,3,4,5) and the 19th element, used to tag main camera visible voxels to
				// compute light bounce for them
				// To map to a particular voxel face use 19 * (voxel index) + 3 * (face index) + 0, +1 and +2
				bufferM->resize(m_lightBounceVoxelIrradianceBuffer,         nullptr, m_firstIndexOccupiedElement * 12 * sizeof(float));
				bufferM->resize(m_lightBounceVoxelFilteredIrradianceBuffer, nullptr, m_firstIndexOccupiedElement * 12 * sizeof(float));
				bufferM->resize(m_lightBounceProcessedVoxelBuffer,          nullptr, m_firstIndexOccupiedElement * sizeof(int));

				cout << "Number of occupied voxel is " << m_firstIndexOccupiedElement << endl;
				cout << "Size m_lightBounceVoxelIrradianceBuffer=" << ((m_lightBounceVoxelIrradianceBuffer->getDataSize()) / 1024.0f) / 1024.0f << "MB" << endl;
			}

			break;
		}
		case PrefixSumStep::PS_SWEEPDOWN:
		{
			MaterialBufferPrefixSum* material = static_cast<MaterialBufferPrefixSum*>(m_vectorMaterial[0]);
			if (m_currentStep > 0)
			{
				m_currentStep--;
				material->setCurrentStep(m_currentStep);
			}
			else
			{
				m_currentPhase++;
				m_currentStepEnum = PrefixSumStep::PS_LAST_STEP;
				material->setCurrentPhase(m_currentPhase);
			}

			break;
		}
		case PrefixSumStep::PS_LAST_STEP:
		{
			//BufferVerificationHelper::verifyVoxelizationProcessData();
			m_compactionStepDone = true;
			m_active             = false;
			m_executeCommand     = false;
			m_needsToRecord      = false;
			m_currentStepEnum    = PrefixSumStep::PS_FINISHED;
			m_prefixSumComplete.emit(); // notify the prefix sum step has been completed

			vectorUint8 vectorHashedPositionCompactedBuffer;
			m_voxelHashedPositionCompactedBuffer->getContentCopy(vectorHashedPositionCompactedBuffer);
			uint numHashedPositionCompactedBuffer = uint(m_voxelHashedPositionCompactedBuffer->getDataSize()) / sizeof(uint);
			uint* pHashedPositionCompactedBuffer  = (uint*)(vectorHashedPositionCompactedBuffer.data());

			// Reset the contents of the voxelFirstIndexBuffer buffer, which will be reused to tag those voxels with
			// an irradiance field to be built at that position.
			vector<uint> vectorData;
			vectorData.resize(m_voxelizationWidth * m_voxelizationHeight * m_voxelizationDepth);
			memset(vectorData.data(), maxValue, vectorData.size() * size_t(sizeof(uint)));
			m_voxelFirstIndexBuffer->setContent(vectorData.data());
			break;
		}
		default:
		{
			cout << "ERROR: no enum value in BufferPrefixSumTechnique::prepare" << endl;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferPrefixSumTechnique::slotVoxelizationComplete()
{
	m_bufferVoxelFirstIndexComplete = true;
	m_needsToRecord                 = true;
	m_active                        = true;

	m_vectorPrefixSumNumElement.resize(5); // Up to four levels needed to the prefix parallel sum
	memset(m_vectorPrefixSumNumElement.data(), 0, m_vectorPrefixSumNumElement.size() * size_t(sizeof(uint)));

	SceneVoxelizationTechnique* technique = static_cast<SceneVoxelizationTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("SceneVoxelizationTechnique"))));
	m_voxelizationSize                    = technique->getVoxelizedSceneWidth() * technique->getVoxelizedSceneHeight() * technique->getVoxelizedSceneDepth();
	m_vectorPrefixSumNumElement[0]        = m_voxelizationSize / m_numElementAnalyzedPreThread;
	MaterialBufferPrefixSum* material     = static_cast<MaterialBufferPrefixSum*>(m_vectorMaterial[0]);

	material->setNumElementBase(m_vectorPrefixSumNumElement[0]);
	material->setCurrentStep(m_currentStep);
	material->setIndirectionBufferRange(m_indirectionBufferRange);
	material->setVoxelizationWidth(m_voxelizationWidth);

	// The maximum number of elements in bufferHistogramBuffer is 536862720,
	// given that each thread processes 128 elements of bufferHistogramBuffer in the initial pass, up to
	// four levels are needed to complete the algorithm.
	float numElementPerThread           = float(m_numElementAnalyzedPreThread);
	m_prefixSumPlanarBufferSize        += uint(m_vectorPrefixSumNumElement[0]);
	float prefixSumNumElemenCurrentStep = float(m_vectorPrefixSumNumElement[0]);
	bool stop                           = ((prefixSumNumElemenCurrentStep / numElementPerThread) <= 1.0f);

	m_numberStepsReduce++;

	while (!stop)
	{
		prefixSumNumElemenCurrentStep                    = ceilf(prefixSumNumElemenCurrentStep / numElementPerThread);
		m_prefixSumPlanarBufferSize                     += uint(prefixSumNumElemenCurrentStep);
		m_vectorPrefixSumNumElement[m_numberStepsReduce] = uint(prefixSumNumElemenCurrentStep);
		stop                                             = (prefixSumNumElemenCurrentStep <= 1.0f);
		m_numberStepsReduce++;
	}

	m_currentPhase = 0;
	m_compactionStepDone = false;
	m_firstIndexOccupiedElement = 0;
	material->setNumElementLevel0(m_vectorPrefixSumNumElement[1]);
	material->setNumElementLevel1(m_vectorPrefixSumNumElement[2]);
	material->setNumElementLevel2(m_vectorPrefixSumNumElement[3]);
	material->setNumElementLevel3(m_vectorPrefixSumNumElement[4]);
	material->setNumberStepsReduce(m_numberStepsReduce);
	material->setCurrentPhase(m_currentPhase);

	m_numberStepsDownSweep = m_numberStepsReduce;

	uint numElement = uint(m_vectorPrefixSumNumElement.size());
	forI(numElement)
	{
		if (m_vectorPrefixSumNumElement[numElement - 1 - i] > 1)
		{
			if ((i > 0) && (m_vectorPrefixSumNumElement[numElement - i] == 1))
			{
				m_firstSetIsSingleElement = true;
				m_numberStepsDownSweep--;
			}
			break;
		}
	}

	m_usedCommandBufferNumber = m_numberStepsReduce + m_numberStepsDownSweep + 2;

	material->setNumberStepsDownSweep(m_numberStepsDownSweep);
	bufferM->resize(m_prefixSumPlanarBuffer, nullptr, m_prefixSumPlanarBufferSize * sizeof(uint));
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint BufferPrefixSumTechnique::retrieveAccumulatedNumValues()
{
	uint offsetIndex;
	uint numElementLastStep;
	uint maxIndex = uint(m_vectorPrefixSumNumElement.size());

	forI(maxIndex)
	{
		if (m_vectorPrefixSumNumElement[maxIndex - i - 1] > 0)
		{
			numElementLastStep = m_vectorPrefixSumNumElement[maxIndex - i - 1];
			offsetIndex = maxIndex - i - 1;
			break;
		}
	}

	uint offset = 0;
	forI(offsetIndex)
	{
		offset += m_vectorPrefixSumNumElement[i];
	}

	vector<uint8_t> vectorReductionLastStep;
	vectorReductionLastStep.resize(numElementLastStep * sizeof(uint));

	void* mappedMemory;
	VkResult result = vkMapMemory(coreM->getLogicalDevice(), m_prefixSumPlanarBuffer->getMemory(), offset * sizeof(uint), numElementLastStep * sizeof(uint), 0, &mappedMemory);
	assert(result == VK_SUCCESS);

	memcpy((void*)vectorReductionLastStep.data(), mappedMemory, numElementLastStep * sizeof(uint));
	vkUnmapMemory(coreM->getLogicalDevice(), m_prefixSumPlanarBuffer->getMemory());

	uint accumulated = 0;
	uint* pData = (uint*)(vectorReductionLastStep.data());
	forI(numElementLastStep)
	{
		accumulated += pData[i];
	}

	return accumulated;
}

/////////////////////////////////////////////////////////////////////////////////////////////
