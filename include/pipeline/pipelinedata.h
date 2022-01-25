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

#ifndef _PIPELINEDATA_H_
#define _PIPELINEDATA_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/getsetmacros.h"
#include "../headers.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class PipelineData
{
	friend class Pipeline;

public:
	/** Default constructor
	* @return nothing */
	PipelineData();

	/** Sets default values to the member variables
	* @return nothing */
	void setDefaultValues();

	/** Sets as nullptr the field m_pipelineInfo::pDepthStencilState
	* @return nothing */
	void setNullDepthStencilState();

	/** Setter for m_pipelineInfo::renderPass
	* @param renderPass [in] value to set to m_pipelineInfo::renderPass
	* @return nothing */
	void setRenderPass(VkRenderPass renderPass);

	/** Update the m_arrayColorBlendAttachmentStateInfo array with the new data, and the corresponding fields in the color
	* blend state and pipeline create info structures
	* @param arrayData [in] new pipeline color blend attachment information
	* @return nothing */
	void updateArrayColorBlendAttachmentStateInfo(vector<VkPipelineColorBlendAttachmentState>& arrayData);

	/** Update m_vertexInputStateInfo and the corresponding fields in m_pipelineInfo
	* @param vertexInputStateInfo [in] new pipeline vertex input state information
	* @return nothing */
	void updateVertexInputStateInfo(VkPipelineVertexInputStateCreateInfo& vertexInputStateInfo);

	/** Update m_rasterStateInfo and the corresponding fields in m_pipelineInfo
	* @param rasterStateInfo [in] new pipeline raster state information
	* @return nothing */
	void updateRasterStateInfo(VkPipelineRasterizationStateCreateInfo& rasterStateInfo);

	/** Update m_colorBlendStateInfo and the corresponding fields in m_pipelineInfo
	* @param colorBlendStateInfo [in] new color blend state information
	* @param arrayData           [in] color blend attachment information
	* @return nothing */
	void updateColorBlendStateInfo(VkPipelineColorBlendStateCreateInfo&         colorBlendStateInfo,
		                           vector<VkPipelineColorBlendAttachmentState>& arrayData);

	/** Update m_depthStencilStateInfo and the corresponding fields in m_pipelineInfo
	* @param depthStencilStateInfo [in] new depth stencil state information
	* @return nothing */
	void updateDepthStencilStateInfo(VkPipelineDepthStencilStateCreateInfo& depthStencilStateInfo);

	/** Update m_vertexInputStateInfo and the corresponding fields in m_pipelineInfo
	* @param inputAssemblyStateCreateInfo [in] new input assembly state create information
	* @return nothing */
	void updateinputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& inputAssemblyStateCreateInfo);

	/** Update m_dynamicState and the corresponding fields in m_pipelineInfo
	* @param vectorDynamicState [in] vector with the new input dynamic state create info flags
	* @return nothing */
	void updateDynamicStateCreateInfo(vector<VkDynamicState> vectorDynamicState);

	GET_SET(VkPipelineDynamicStateCreateInfo, m_dynamicState, DynamicState)
	GET_SET(VkPipelineVertexInputStateCreateInfo, m_vertexInputStateInfo, VertexInputStateInfo)
	GET_SET(VkPipelineInputAssemblyStateCreateInfo, m_inputAssemblyInfo, InputAssemblyInfo)
	GET_SET(VkPipelineRasterizationStateCreateInfo, m_rasterStateInfo, RasterStateInfo)
	GET(vector<VkPipelineColorBlendAttachmentState>, m_arrayColorBlendAttachmentStateInfo, ArrayColorBlendAttachmentStateInfo)
	GET_SET(VkPipelineColorBlendStateCreateInfo, m_colorBlendStateInfo, ColorBlendStateInfo)
	GET_SET(VkPipelineViewportStateCreateInfo, m_viewportStateInfo, ViewportStateInfo)
	GET_SET(VkPipelineDepthStencilStateCreateInfo, m_depthStencilStateInfo, DepthStencilStateInfo)
	GET_SET(VkPipelineMultisampleStateCreateInfo, m_multiSampleStateInfo, MultiSampleStateInfo)
	GET_SET(VkGraphicsPipelineCreateInfo, m_pipelineInfo, PipelineInfo)

protected:
	VkDynamicState                              m_dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE]; //!< Dynamic state information for the pipeline, viewport and scissor
	VkPipelineDynamicStateCreateInfo            m_dynamicState;                                     //!< Information strcut for the dynamic state information for the pipeline
	VkPipelineVertexInputStateCreateInfo        m_vertexInputStateInfo;                             //!< Information about the format and input of vertex for the vertex shaders
	VkPipelineInputAssemblyStateCreateInfo      m_inputAssemblyInfo;                                //!< Input topology used (triangles)
	VkPipelineRasterizationStateCreateInfo      m_rasterStateInfo;                                  //!< Information about raster state (polygon fill, clockwiseness, depth bias,...)
	vector<VkPipelineColorBlendAttachmentState> m_arrayColorBlendAttachmentStateInfo;               //!< Color blend information
	VkPipelineColorBlendStateCreateInfo         m_colorBlendStateInfo;                              //!< More color blend information for the pipeline
	VkPipelineViewportStateCreateInfo           m_viewportStateInfo;                                //!< Number of viewports used
	VkPipelineDepthStencilStateCreateInfo       m_depthStencilStateInfo;                            //!< Depth and stencil settings
	VkPipelineMultisampleStateCreateInfo        m_multiSampleStateInfo;                             //!< Multisample settings
	VkGraphicsPipelineCreateInfo                m_pipelineInfo;                                     //!< Pipeline building struct that holds most of the previous structs
};

/////////////////////////////////////////////////////////////////////////////////////////////

// INLINE METHODS

inline void PipelineData::setRenderPass(VkRenderPass renderPass)
{
	m_pipelineInfo.renderPass = renderPass;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _PIPELINEDATA_H_
