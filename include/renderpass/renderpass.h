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

#ifndef _RENDERPASS_H_
#define _RENDERPASS_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class RenderPass : public GenericResource
{
	friend class RenderPassManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	RenderPass(string &&name);

	/** Destructor
	* @return nothing */
	virtual ~RenderPass();

	/** Updates the member variables with the parameters present in parent member variable m_parameterData
	* @return nothing */
	void updateRenderPassParameters();

	/** Builds the render pass m_renderPass member variable with the data present in the class member variables 
	* @return nothing */
	void buildRenderPass();

public:
	GET(VkRenderPass, m_renderPass, RenderPass)
	REF_RETURN_PTR(VkRenderPass, m_renderPass, RenderPass)

protected:
	VkAttachmentReference         m_depthReference;                  //!< Attachment reference data for the depth attachment in the render pass
	VkPipelineBindPoint           m_pipelineBindPoint;               //!< Pipeline bind point enum
	VkRenderPass                  m_renderPass;                      //!< Vulkan render pass built element handler
	vector<VkFormat>              m_vectorAttachmentFormat;          //!< Vector with the format of each one of the render pass attachments
	vector<VkSampleCountFlagBits> m_vectorAttachmentSamplesPerPixel; //!< Vector with the number of samples per pixels of each attachment in the render pass
	vector<VkImageLayout>         m_vectorAttachmentFinalLayout;     //!< Vector with the final layout to apply to each one of the render pass attachments
	vector<VkAttachmentReference> m_vectorColorReference;            //!< Vector with the attachment reference data for each one of the color attachments in the render pass
	bool                          m_hasDepthAttachment;              //!< True if the render pass has depth attachment, false otherwise
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _RENDERPASS_H_
