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
#include "../../include/uniformbuffer/uniformbuffermanager.h"
#include "../../include/uniformbuffer/uniformbuffer.h"
#include "../../include/core/coremanager.h"
#include "../../include/parameter/attributedefines.h"

// NAMESPACE
using namespace attributedefines;

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

UniformBufferManager::UniformBufferManager()
{
	m_managerName = g_uniformBufferManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////

UniformBufferManager::~UniformBufferManager()
{
	destroyResources();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void UniformBufferManager::destroyResources()
{
	forIT(m_mapElement)
	{
		delete it->second;
		it->second = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool UniformBufferManager::resize(string&& instanceName, int cellSize, int numCells)
{
	UniformBuffer* uniformBuffer = getElement(move(string(instanceName)));

	if (uniformBuffer == nullptr)
	{
		return false;
	}

	uniformBuffer->m_ready       = false;
	uniformBuffer->destroyResources();
	uniformBuffer->setMinCellSize(cellSize);
	uniformBuffer->m_CPUBuffer.setNumCells(numCells);
	uniformBuffer->m_CPUBuffer.setMinCellSize(cellSize);
	buildUniformBufferResources(uniformBuffer);
	uniformBuffer->m_ready = true;

	if (gpuPipelineM->getPipelineInitialized())
	{
		emitSignalElement(move(instanceName), ManagerNotificationType::MNT_CHANGED);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

UniformBuffer* UniformBufferManager::buildUniformBuffer(string&& instanceName, int cellSize, int numCells)
{
	if (existsElement(move(instanceName)))
	{
		return getElement(move(instanceName));
	}

	UniformBuffer* uniformBuffer = new UniformBuffer(move(string(instanceName)));
	uniformBuffer->setMinCellSize(cellSize);
	uniformBuffer->m_CPUBuffer.setNumCells(numCells);
	uniformBuffer->m_CPUBuffer.setMinCellSize(cellSize);

	buildUniformBufferResources(uniformBuffer);

	addElement(move(string(instanceName)), uniformBuffer);
	uniformBuffer->m_name  = move(instanceName);
	uniformBuffer->m_ready = true;

	return uniformBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void UniformBufferManager::buildUniformBufferResources(UniformBuffer* uniformBuffer)
{
	uniformBuffer->m_CPUBuffer.buildCPUBuffer();
	uniformBuffer->buildGPUBufer();
}

/////////////////////////////////////////////////////////////////////////////////////////////
