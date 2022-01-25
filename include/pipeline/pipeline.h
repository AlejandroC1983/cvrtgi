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

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/genericresource.h"
#include "../../include/pipeline/pipelinedata.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Pipeline : public GenericResource
{
	friend class PipelineManager;

public:
	/** Default constructor
	* @return nothing */
	Pipeline();

	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	Pipeline(string &&name);

	/** Destructor
	* @return nothing */
	virtual ~Pipeline();

	/** Sets default values to the member variables
	* @return nothing */
	void setDefaultValues();

	/** Destroys the resources allocated by this pipeline, if any
	* @return nothing */
	void destroyResources();

public:
	/** Sets the pipeline layout used in this pipeline before its built
	* @return nothing */
	void setPipelineLayout(const VkPipelineLayout& pipelineLayout);

	/** Builds the pipeline with the values present in the member variables, result is stored in m_pipeline
	* @return true if the pipeline was built successfully and false otherwise */
	bool buildPipeline();

	/** Sets the value of VkGraphicsPipelineCreateInfo::pStages
	* @param arrayPipelineShaderStage [in] value data to set
	* @return nothing */
	void setPipelineShaderStage(const vector<VkPipelineShaderStageCreateInfo>& arrayPipelineShaderStage);

	/** Sets the value of m_pipelineInfo.basePipelineIndex
	* @param index [in] value to assign to m_pipelineInfo.basePipelineIndex
	* @return nothing */
	void setBasePipelineIndex(int index);

	GET(VkPipeline, m_pipeline, Pipeline)
	REF(VkPipeline, m_pipeline, Pipeline)
	GET_SET(PipelineData, m_pipelineData, PipelineData)
	REF(PipelineData, m_pipelineData, PipelineData)

protected:
	PipelineData m_pipelineData; //!< Contains the information that will be used to build this pipeline
	VkPipeline   m_pipeline;     //!< Finally built pipeline
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _PIPELINE_H_
