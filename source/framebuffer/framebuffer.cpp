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
#include "../../include/framebuffer/framebuffer.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/texture/texture.h"
#include "../../include/texture/texturemanager.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Framebuffer::Framebuffer(string &&name) : GenericResource(move(name), move(string("Framebuffer")), GenericResourceType::GRT_FRAMEBUFFER)
	, m_width(0)
	, m_height(0)
	, m_renderPass(nullptr)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

Framebuffer::~Framebuffer()
{
	destroyFramebufferResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool Framebuffer::attachmentResourceIsUsed(const string& resourceName)
{
	return (find(m_arrayAttachmentName.begin(), m_arrayAttachmentName.end(), resourceName) != m_arrayAttachmentName.end());
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Framebuffer::destroyFramebufferResources()
{
	vkDestroyFramebuffer(coreM->getLogicalDevice(), m_framebuffer, NULL);
	m_framebuffer = VK_NULL_HANDLE;
	m_renderPass  = nullptr;
	m_arrayAttachment.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////
