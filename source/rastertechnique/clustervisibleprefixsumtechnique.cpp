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
#include "../../include/rastertechnique/clustervisibleprefixsumtechnique.h"
#include "../../include/buffer/buffer.h"
#include "../../include/buffer/buffermanager.h"
#include "../../include/rastertechnique/clustervisibilitytechnique.h"
#include "../../include/core/gpupipeline.h"
#include "../../include/core/coremanager.h"
#include "../../include/material/materialprefixsum.h"
#include "../../include/util/bufferverificationhelper.h"
#include "../../include/rastertechnique/bufferprefixsumtechnique.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ClusterVisiblePrefixSumTechnique::ClusterVisiblePrefixSumTechnique(string&& name, string&& className) : RasterTechnique(move(name), move(className))
	, m_clusterVisibilityBuffer(nullptr)
	, m_clusterVisibilityCompactedBuffer(nullptr)
	, m_clusterVisibilityFirstIndexBuffer(nullptr)
	, m_prefixSumBuffer(nullptr)
	, m_clusterVisibilityTechnique(nullptr)
	, m_prefixSumPlanarBufferSize(0)
	, m_numberStepsReduce(0)
	, m_firstSetIsSingleElement(false)
	, m_numberStepsDownSweep(0)
	, m_currentStep(0)
	, m_currentPhase(0)
	, m_compactionStepDone(false)
	, m_firstIndexOccupiedElement(0)
	, m_currentStepEnum(PrefixSumStep_::PS_REDUCTION)
	, m_numElementAnalyzedPerThread(0)
{
	m_active                            = false;
	m_needsToRecord                     = false;
	m_rasterTechniqueType               = RasterTechniqueType::RTT_COMPUTE;
	m_computeHostSynchronize            = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterVisiblePrefixSumTechnique::init()
{
	// Shader storage buffer used to store the whole prefix sum steps (reduction and sweepdown)
	// The buffer will be resized once slotClusterVisibility is called
	m_prefixSumBuffer = bufferM->buildBuffer(
		move(string("prefixSumBuffer")),
		nullptr,
		256,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// TODO: Rename to more generic, prefix sum values
	// TODO: Delete prefixSumBuffer once the prefix sum part is done
	m_clusterVisibilityBuffer           = bufferM->getElement(move(string("clusterVisibilityBuffer")));
	m_clusterVisibilityCompactedBuffer  = bufferM->getElement(move(string("clusterVisibilityCompactedBuffer")));
	m_clusterVisibilityFirstIndexBuffer = bufferM->getElement(move(string("clusterVisibilityFirstIndexBuffer")));

	m_vectorMaterialName.resize(1);
	m_vectorMaterial.resize(1);
	m_vectorMaterialName[0] = string("MaterialPrefixSum");
	m_vectorMaterial[0] = materialM->buildMaterial(move(string("MaterialPrefixSum")), move(string("MaterialPrefixSum")), nullptr);

	m_clusterVisibilityTechnique = static_cast<ClusterVisibilityTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("ClusterVisibilityTechnique"))));
	m_clusterVisibilityTechnique->refSignalClusterVisibilityCompletion().connect<ClusterVisiblePrefixSumTechnique, &ClusterVisiblePrefixSumTechnique::slotClusterVisibility>(this);
	m_numElementAnalyzedPerThread = m_clusterVisibilityTechnique->getNumThreadPerLocalWorkgroup();
}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* ClusterVisiblePrefixSumTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	MaterialPrefixSum* material = static_cast<MaterialPrefixSum*>(m_vectorMaterial[0]);

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

	uint32_t offsetData = static_cast<uint32_t>(material->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, material->getPipelineLayout(), 0, 1, &material->refDescriptorSet(), 1, &offsetData);

	uint finalWorkgroupSize;

	// 0 means reduction step, 1 sweep down, 2 write final compacted buffer
	if (m_currentPhase == 0)
	{
		finalWorkgroupSize = m_vectorPrefixSumNumElement[m_currentStep];
	}
	else if (m_currentPhase == 1)
	{
		finalWorkgroupSize = uint(ceilf(float(m_vectorPrefixSumNumElement[m_currentStep]) / float(m_numElementAnalyzedPerThread)));
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

void ClusterVisiblePrefixSumTechnique::postCommandSubmit()
{
	switch (m_currentStepEnum)
	{
		case PrefixSumStep_::PS_REDUCTION:
		{
			MaterialPrefixSum* material = static_cast<MaterialPrefixSum*>(m_vectorMaterial[0]);
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
				// Since the indices to the clusters are encoded using 16 bits, half of the amount of indices is needed
				uint bufferSize = uint(round(float(m_firstIndexOccupiedElement) / 2.0f));
				//bufferM->resize(m_clusterVisibilityCompactedBuffer, nullptr, m_firstIndexOccupiedElement * sizeof(uint));
				bufferM->resize(m_clusterVisibilityCompactedBuffer, nullptr, bufferSize * sizeof(uint));
				m_currentStepEnum = PrefixSumStep_::PS_SWEEPDOWN;
				
				cout << "Total number of visible cluster from all voxel faces is " << m_firstIndexOccupiedElement << endl;
				cout << "Size m_clusterVisibilityCompactedBuffer=" << ((m_clusterVisibilityCompactedBuffer->getDataSize()) / 1024.0f) / 1024.0f << "MB" << endl;
			}

			break;
		}
		case PrefixSumStep_::PS_SWEEPDOWN:
		{
			MaterialPrefixSum* material = static_cast<MaterialPrefixSum*>(m_vectorMaterial[0]);
			if (m_currentStep > 0)
			{
				m_currentStep--;
				material->setCurrentStep(m_currentStep);
			}
			else
			{
				m_currentPhase++;
				m_currentStepEnum = PrefixSumStep_::PS_LAST_STEP;
				material->setCurrentPhase(m_currentPhase);
			}

			break;
		}
		case PrefixSumStep_::PS_LAST_STEP:
		{
			// Copy the data in prefixSumBuffer that contains, for each element in voxelHashedPositionCompactedBuffer
			// the indices where to get in clusterVisibilityCompactedBuffer the clusters visible for that voxel,
			// for each one of the six voxel faces. This is stored at the beginning of prefixSumBuffer
			// after the algorithm ends

			Buffer* prefixSumBuffer = bufferM->getElement(move(string("prefixSumBuffer")));

			BufferPrefixSumTechnique* techniquePrefixSum = static_cast<BufferPrefixSumTechnique*>(gpuPipelineM->getRasterTechniqueByName(move(string("BufferPrefixSumTechnique"))));
			uint numOccupiedVoxel = techniquePrefixSum->getFirstIndexOccupiedElement();
			uint numElementToCopy = numOccupiedVoxel * 6;

			// Resize the cluster visibility first index buffer, which is used in combination with clusterVisibilityNumberBuffer
			// and clusterVisibilityCompactedBuffer to know, for each voxel face of each generated voxel, where in 
			// clusterVisibilityCompactedBuffer start the amount of visible clusters from that voxel
			bufferM->resize(m_clusterVisibilityFirstIndexBuffer, nullptr, numElementToCopy * sizeof(uint));
			cout << "The size of the clusterVisibilityFirstIndexBuffer buffer is" << numElementToCopy * sizeof(uint) << endl;
			// Copy the content to clusterVisibilityFirstIndexBuffer, which is generated at the end of the prefix sum process
			// and is present in the prefixSumBuffer buffer
			bufferM->copyBuffer(prefixSumBuffer, m_clusterVisibilityFirstIndexBuffer, 0, 0, int(m_clusterVisibilityFirstIndexBuffer->getDataSize()));

			//BufferVerificationHelper::verifyClusterVisibilityFirstIndexBuffer();
			//BufferVerificationHelper::verifyClusterVisibilityCompactedData();

			m_compactionStepDone = true;
			m_active             = false;
			m_executeCommand     = false;
			m_needsToRecord      = false;
			m_currentStepEnum    = PrefixSumStep_::PS_FINISHED;
			m_signalClusterVisiblePrefixSumTechnique.emit(); // notify the prefix sum step has been completed

			/**/
			//m_prefixSumBuffer
			vectorUint8 m_prefixSumBufferNumberBuffer;
			Buffer* clusterVisibilityNumberBuffer = bufferM->getElement(move(string("clusterVisibilityCompactedBuffer")));
			clusterVisibilityNumberBuffer->getContentCopy(m_prefixSumBufferNumberBuffer);
			uint numClusterVisibilityNumberBuffer = uint(clusterVisibilityNumberBuffer->getDataSize()) / sizeof(uint);
			uint* pClusterVisibilityNumberBuffer = (uint*)(m_prefixSumBufferNumberBuffer.data());

			// Now do a verification of clusterVisibilityFirstIndexBuffer
			//for (uint i = 0; i < numClusterVisibilityNumberBuffer; ++i)
			for (uint i = 0; i < 100; ++i)
			{
				uint temp = 0;
				uint decoded0 = pClusterVisibilityNumberBuffer[i];
				decoded0 &= (0x0000FFFF << (temp * 16));
				decoded0 >>= (temp * 16);
				temp = 1;
				uint decoded1 = pClusterVisibilityNumberBuffer[i];
				decoded1 &= (0x0000FFFF << (temp * 16));
				decoded1 >>= (temp * 16);
				cout << "i = " << i << ", decoded0=" << decoded0 << ", decoded1=" << decoded1 << endl;
			}
			/**/
			break;
		}
		default:
		{
			cout << "ERROR: no enum value in ClusterVisiblePrefixSumTechnique::postCommandSubmit" << endl;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ClusterVisiblePrefixSumTechnique::slotClusterVisibility()
{
	m_needsToRecord = true;
	m_active        = true;

	// TODO: Put the code below in a utility method and refactor with BufferPrefixSumTechnique

	m_vectorPrefixSumNumElement.resize(5); // Up to four levels needed to the prefix parallel sum
	memset(m_vectorPrefixSumNumElement.data(), 0, m_vectorPrefixSumNumElement.size() * size_t(sizeof(uint)));

	// The amount of elements to process is the value of ClusterVisibilityTechnique::m_bufferNumElement
	// which is the amount of occupied voxels multiplied by the number of faces of each voxel (6)
	m_vectorPrefixSumNumElement[0]  = m_clusterVisibilityTechnique->getBufferNumElement() / m_numElementAnalyzedPerThread;
	MaterialPrefixSum* material     = static_cast<MaterialPrefixSum*>(m_vectorMaterial[0]);

	material->setNumElementBase(m_vectorPrefixSumNumElement[0]);
	material->setCurrentStep(m_currentStep);

	// The maximum number of elements in bufferHistogramBuffer is 536862720,
	// given that each thread processes 128 elements of bufferHistogramBuffer in the initial pass, up to
	// four levels are needed to complete the algorithm.
	float numElementPerThread           = float(m_numElementAnalyzedPerThread);
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
	bufferM->resize(m_prefixSumBuffer, nullptr, m_prefixSumPlanarBufferSize * sizeof(uint));
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint ClusterVisiblePrefixSumTechnique::retrieveAccumulatedNumValues()
{
	// TODO: Set as utility function and reuse it in BufferPrefixSumTechnique, right now duplicated

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
	VkResult result = vkMapMemory(coreM->getLogicalDevice(), m_prefixSumBuffer->getMemory(), offset * sizeof(uint), numElementLastStep * sizeof(uint), 0, &mappedMemory);
	assert(result == VK_SUCCESS);

	memcpy((void*)vectorReductionLastStep.data(), mappedMemory, numElementLastStep * sizeof(uint));
	vkUnmapMemory(coreM->getLogicalDevice(), m_prefixSumBuffer->getMemory());

	uint accumulated = 0;
	uint* pData = (uint*)(vectorReductionLastStep.data());
	forI(numElementLastStep)
	{
		accumulated += pData[i];
	}

	return accumulated;
}

/////////////////////////////////////////////////////////////////////////////////////////////
