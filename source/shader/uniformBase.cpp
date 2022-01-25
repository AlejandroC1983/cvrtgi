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
#include "../../include/shader/uniformBase.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

UniformBase::UniformBase(const ResourceInternalType type, VkShaderStageFlagBits shaderStage, string&& name, string&& className, string&& structName, string&& structType, ShaderStruct* shaderStructOwner) : GenericResource(move(string(name)), move(className), GenericResourceType::GRT_UNIFORMBASE)
	, m_type(type)
	, m_shaderStage(shaderStage)
	, m_shaderStructOwner(shaderStructOwner)
	, m_name(move(name))
	, m_structName(move(structName))
	, m_structType(move(structType))
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
