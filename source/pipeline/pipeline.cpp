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
#include "../../include/pipeline/pipeline.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/parameter/attributedata.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Pipeline::Pipeline() : GenericResource(move(string("")), move(string("Pipeline")), GenericResourceType::GRT_PIPELINE)
	, m_pipeline(VK_NULL_HANDLE)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Pipeline::Pipeline(string &&name) : GenericResource(move(name), move(string("Pipeline")), GenericResourceType::GRT_PIPELINE)
	, m_pipeline(VK_NULL_HANDLE)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Pipeline::~Pipeline()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::setDefaultValues()
{
	m_pipelineData.setDefaultValues();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::destroyResources()
{
	if (m_pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(coreM->getLogicalDevice(), m_pipeline, NULL);

		m_pipeline = VK_NULL_HANDLE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::setPipelineLayout(const VkPipelineLayout& pipelineLayout)
{
	m_pipelineData.m_pipelineInfo.layout = pipelineLayout;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Pipeline::buildPipeline()
{
	if (vkCreateGraphicsPipelines(coreM->getLogicalDevice(), gpuPipelineM->getPipelineCache(), 1, &m_pipelineData.m_pipelineInfo, NULL, &m_pipeline) == VK_SUCCESS)
	{
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::setPipelineShaderStage(const vector<VkPipelineShaderStageCreateInfo>& arrayPipelineShaderStage)
{
	m_pipelineData.m_pipelineInfo.pStages    = arrayPipelineShaderStage.data();
	m_pipelineData.m_pipelineInfo.stageCount = (uint32_t)arrayPipelineShaderStage.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Pipeline::setBasePipelineIndex(int index)
{
	m_pipelineData.m_pipelineInfo.basePipelineIndex = index;
}

/////////////////////////////////////////////////////////////////////////////////////////////
