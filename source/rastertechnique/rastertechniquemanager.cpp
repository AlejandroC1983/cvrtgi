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
#include "../../include/rastertechnique/rastertechniquemanager.h"
#include "../../include/rastertechnique/rastertechniquefactory.h"
#include "../../include/rastertechnique/rastertechnique.h"
#include "../../include/parameter/attributedefines.h"
#include "../../include/material/material.h"
#include "../../include/material/materialmanager.h"
#include "../../include/core/coremanager.h"

// NAMESPACE

// DEFINES
using namespace attributedefines;

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

RasterTechniqueManager::RasterTechniqueManager()
{
	m_managerName = g_rasterTechniqueManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////

RasterTechniqueManager::~RasterTechniqueManager()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

RasterTechnique *RasterTechniqueManager::buildNewRasterTechnique(string &&className, string &&instanceName, MultiTypeUnorderedMap* attributeData)
{
	if (existsElement(move(instanceName)))
	{
		return getElement(move(instanceName));
	}

	RasterTechnique *result = RasterTechniqueFactory::buildElement(move(className));

	if (result != nullptr)
	{
		result->setName(move(string(instanceName)));
		RasterTechniqueManager::addElement(move(instanceName), result);
		result->setParameterData(attributeData);
		result->init();
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void RasterTechniqueManager::assignSlots()
{
	materialM->refElementSignal().connect<RasterTechniqueManager, &RasterTechniqueManager::slotElement>(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void RasterTechniqueManager::slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType)
{
	if (!gpuPipelineM->getPipelineInitialized())
	{
		return;
	}

	if (managerName == g_materialManager)
	{
		map<string, RasterTechnique*>::iterator it = m_mapElement.begin();
		for (it; it != m_mapElement.end(); ++it)
		{
			if (it->second->materialResourceNotification(move(string(elementName)), notificationType))
			{
				if (gpuPipelineM->getPipelineInitialized())
				{
					emitSignalElement(move(string(elementName)), notificationType);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
