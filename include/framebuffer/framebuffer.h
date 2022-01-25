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

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/genericresource.h"
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class Framebuffer : public GenericResource
{
	friend class FramebufferManager;

protected:
	/** Parameter constructor
	* @param [in] shader's name
	* @return nothing */
	Framebuffer(string &&name);

	/** Destructor
	* @return nothing */
	virtual ~Framebuffer();

public:
	GETCOPY(uint32_t, m_width, Width)
	GETCOPY(uint32_t, m_height, Height)
	GET_PTR(VkRenderPass, m_renderPass, RenderPass)
	GETCOPY(VkFramebuffer, m_framebuffer, Framebuffer)
	GET(string, m_renderPassName, RenderPassName)
	GET(vector<string>, m_arrayAttachmentName, ArrayAttachmentName)

protected:
	/** Returns true if the resource is used in the attachment vector
	* @param resourceName [in] resource name
	* @return true if the resource is used in the attachment vector, and flase otherwise */
	bool attachmentResourceIsUsed(const string& resourceName);

	/** Destorys m_framebuffer and cleans m_arrayAttachment and m_renderPass
	* @return nothing */
	void destroyFramebufferResources();

	uint32_t            m_width;               //!< Framebuffer width
	uint32_t            m_height;              //!< Framebuffer height
	VkRenderPass*       m_renderPass;          //!< Pointer to the render pass used to build this framebuffer
	vector<VkImageView> m_arrayAttachment;     //!< Vector with the attachments used in this framebuffer
	VkFramebuffer       m_framebuffer;         //!< Framebuffer object
	vector<string>      m_arrayAttachmentName; //!< Vector with the names of the attachments used in this framebuffer
	string              m_renderPassName;      //!< Name of the render pass to use for this framebuffer
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _FRAMEBUFFER_H_
