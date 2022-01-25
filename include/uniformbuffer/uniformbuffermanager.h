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

#ifndef _UNIFORMBUFFERMANAGER_H_
#define _UNIFORMBUFFERMANAGER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/singleton.h"
#include "../../include/util/managertemplate.h"
#include "../headers.h"

// CLASS FORWARDING
class MultiTypeUnorderedMap;
class UniformBuffer;

// NAMESPACE

// DEFINES
#define uniformBufferM s_pUniformBufferManager->instance()

/////////////////////////////////////////////////////////////////////////////////////////////

class UniformBufferManager: public ManagerTemplate<UniformBuffer>, public Singleton<UniformBufferManager>
{
	friend class CoreManager;

public:
	/** Constructor
	* @return nothing */
	UniformBufferManager();

	/** Destructor
	* @return nothing */
	virtual ~UniformBufferManager();

	/** Builds a new framebuffer, a pointer to the framebuffer is returned, nullptr is returned if any errors while building it
	* @param instanceName [in] name of the new instance (the m_sName member variable)
	* @param cellSize     [in] size of each cell in the buffer in bytes
	* @param numCells     [in] number of cells in the buffer
	* @return a pointer to the built uniform buffer, nullptr otherwise */
	UniformBuffer* buildUniformBuffer(string&& instanceName, int cellSize, int numCells);

	/** Destroys all elements in the manager
	* @return nothing */
	void destroyResources();

	/** Resizes the uniform buffer with name given by instanceName if present in the manager's resources
	* @param instanceName [in] name of the instance to resize
	* @param cellSize     [in] size of each cell in the buffer in bytes
	* @param numCells     [in] number of cells in the buffer
	* @return true if thre resize operation was performed successfully, false otherwise */
	bool resize(string&& instanceName, int cellSize, int numCells);

protected:
	/** Builds the resources for the UniformBuffer given as parameter
	* @param uniformBuffer [in] uniform buffer to build resources for
	* @return nothing */
	void buildUniformBufferResources(UniformBuffer* uniformBuffer);
};

static UniformBufferManager* s_pUniformBufferManager;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _UNIFORMBUFFERMANAGER_H_
