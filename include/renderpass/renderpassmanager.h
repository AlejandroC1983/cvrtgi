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

#ifndef _RENDERPASSMANAGER_H_
#define _RENDERPASSMANAGER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/singleton.h"
#include "../../include/util/managertemplate.h"

// CLASS FORWARDING
class MultiTypeUnorderedMap;
class RenderPass;

// NAMESPACE

// DEFINES
#define renderPassM s_pRenderPassManager->instance()

/////////////////////////////////////////////////////////////////////////////////////////////

class RenderPassManager: public ManagerTemplate<RenderPass>, public Singleton<RenderPassManager>
{
	friend class CoreManager;

public:
	/** Constructor
	* @return nothing */
	RenderPassManager();

	/** Destructor
	* @return nothing */
	virtual ~RenderPassManager();

	/** Builds a new render pass, a pointer to the render pass is returned, nullptr is returned if any errors while building it
	* @param instanceName  [in] name of the new instance (the m_sName member variable)
	* @param attributeData [in] attributes to configure the newly built render pass
	* @return a pointer to the built render pass, nullptr otherwise */
	RenderPass* buildRenderPass(string&& instanceName, MultiTypeUnorderedMap* attributeData);

	/** Destroys all elements in the manager
	* @return nothing */
	void destroyResources();
};

static RenderPassManager *s_pRenderPassManager;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _RENDERPASSMANAGER_H_
