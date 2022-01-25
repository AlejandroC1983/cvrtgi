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
#include "../../include/rastertechnique/bufferprocesstechnique.h"
#include "../../include/material/materialmanager.h"
#include "../../include/material/material.h"
#include "../../include/core/coremanager.h"
#include "../../include/uniformbuffer/uniformbuffer.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

BufferProcessTechnique::BufferProcessTechnique(string &&name, string&& className) : RasterTechnique(move(name), move(className))
	, m_buffer(nullptr)
	, m_bufferNumElement(0)
	, m_numElementPerLocalWorkgroupThread(0)
	, m_numThreadPerLocalWorkgroup(0)
	, m_localWorkGroupsXDimension(0)
	, m_localWorkGroupsYDimension(0)
	, m_numThreadExecuted(0)
	, m_material(nullptr)
	, m_newPassRequested(false)
	, m_localSizeX(1)
	, m_localSizeY(1)
{
	m_recordPolicy  = CommandRecordPolicy::CRP_SINGLE_TIME;
	m_active        = false;
	m_needsToRecord = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

BufferProcessTechnique::~BufferProcessTechnique()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferProcessTechnique::init()
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferProcessTechnique::recordBarriers(VkCommandBuffer* commandBuffer)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

VkCommandBuffer* BufferProcessTechnique::record(int currentImage, uint& commandBufferID, CommandBufferType& commandBufferType)
{
	commandBufferType = CommandBufferType::CBT_COMPUTE_QUEUE;

	VkCommandBuffer* commandBuffer;
	addCommandBufferQueueType(commandBufferID, commandBufferType);
	commandBuffer = addRecordedCommandBuffer(commandBufferID);
	coreM->allocCommandBuffer(&coreM->getLogicalDevice(), coreM->getComputeCommandPool(), commandBuffer);

	coreM->beginCommandBuffer(*commandBuffer);

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex0);
#endif

	recordBarriers(commandBuffer);

	uint dynamicAllignment = materialM->getMaterialUBDynamicAllignment();

	uint32_t offsetData;
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_material->getPipeline()->getPipeline());
	offsetData = static_cast<uint32_t>(m_material->getMaterialUniformBufferIndex() * dynamicAllignment);
	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_material->getPipelineLayout(), 0, 1, &m_material->refDescriptorSet(), 1, &offsetData);
	vkCmdDispatch(*commandBuffer, m_localWorkGroupsXDimension, m_localWorkGroupsYDimension, 1); // Compute shader global workgroup https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html

#ifdef USE_TIMESTAMP
	vkCmdWriteTimestamp(*commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, coreM->getComputeQueueQueryPool(), m_queryIndex1);
#endif

	coreM->endCommandBuffer(*commandBuffer);
	
	m_vectorCommand.push_back(commandBuffer);

	return commandBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferProcessTechnique::buildShaderThreadMapping()
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

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferProcessTechnique::obtainDispatchWorkGroupCount()
{
	uvec3 maxComputeWorkGroupCount     = coreM->getMaxComputeWorkGroupCount();
	uint maxLocalWorkGroupXDimension   = maxComputeWorkGroupCount.x;
	uint maxLocalWorkGroupYDimension   = maxComputeWorkGroupCount.y;
	uint numElementPerLocalWorkgroup   = m_numThreadPerLocalWorkgroup * m_numElementPerLocalWorkgroupThread;
	m_numThreadExecuted                = uint(ceil(float(m_bufferNumElement) / float(m_numElementPerLocalWorkgroupThread)));

	float numLocalWorkgroupsToDispatch = ceil(float(m_bufferNumElement) / float(numElementPerLocalWorkgroup));
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

/////////////////////////////////////////////////////////////////////////////////////////////

void BufferProcessTechnique::obtainDispatchWorkGroupCount(uint  numThreadPerLocalWorkgroup,
														  uint  numElementPerLocalWorkgroupThread,
														  uint  bufferNumElement,
														  uint& localWorkGroupsXDimension,
														  uint& localWorkGroupsYDimension)
{
	uvec3 maxComputeWorkGroupCount   = coreM->getMaxComputeWorkGroupCount();
	uint maxLocalWorkGroupXDimension = maxComputeWorkGroupCount.x;
	uint maxLocalWorkGroupYDimension = maxComputeWorkGroupCount.y;
	uint numElementPerLocalWorkgroup = numThreadPerLocalWorkgroup * numElementPerLocalWorkgroupThread;
	uint numThreadExecuted           = uint(ceil(float(bufferNumElement) / float(numElementPerLocalWorkgroupThread)));

	float numLocalWorkgroupsToDispatch = ceil(float(bufferNumElement) / float(numElementPerLocalWorkgroup));
	if (numLocalWorkgroupsToDispatch <= maxLocalWorkGroupYDimension)
	{
		localWorkGroupsXDimension = 1;
		localWorkGroupsYDimension = uint(numLocalWorkgroupsToDispatch);
	}
	else
	{
		float integerPart;
		float fractional            = glm::modf(float(numLocalWorkgroupsToDispatch) / float(maxLocalWorkGroupYDimension), integerPart);
		localWorkGroupsXDimension = uint(ceil(integerPart + 1.0f));
		localWorkGroupsYDimension = uint(ceil(float(numLocalWorkgroupsToDispatch) / float(localWorkGroupsXDimension)));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
