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
#include "../../include/renderpass/renderpassmanager.h"
#include "../../include/renderpass/renderpass.h"
#include "../../include/parameter/attributedefines.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

RenderPassManager::RenderPassManager()
{
	m_managerName = g_renderPassManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////

RenderPassManager::~RenderPassManager()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void RenderPassManager::destroyResources()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

RenderPass* RenderPassManager::buildRenderPass(string&& instanceName, MultiTypeUnorderedMap* attributeData)
{
	if (existsElement(move(instanceName)))
	{
		return getElement(move(instanceName));
	}

	RenderPass* renderPass = new RenderPass(move(string(instanceName)));
	renderPass->m_parameterData = attributeData;
	renderPass->updateRenderPassParameters();
	renderPass->buildRenderPass();

	addElement(move(string(instanceName)), renderPass);
	renderPass->m_name = move(instanceName);
	renderPass->m_ready = true;

	return renderPass;
}

/////////////////////////////////////////////////////////////////////////////////////////////
