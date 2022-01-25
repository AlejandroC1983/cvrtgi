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
#include "../../include/shader/shaderstruct.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderStruct::ShaderStruct():
	  m_uniformBufferType(ResourceInternalType::RIT_SIZE)
	, m_bindingIndex(-1)
	, m_setIndex(-1)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderStruct::ShaderStruct(string               &&name,
						   string               &&className,
		                   ResourceInternalType   uniformBufferType,
              	           VkShaderStageFlagBits  shaderStage,
		                   int                    bindingIndex,
		                   int                    setIndex,
		                   string               &&variableName,
		                   string               &&structName) : GenericResource(move(name), move(className), GenericResourceType::GRT_SHADERSTRUCT)
	, m_uniformBufferType(uniformBufferType)
	, m_shaderStage(shaderStage)
	, m_bindingIndex(bindingIndex)
	, m_setIndex(setIndex)
	, m_variableName(move(variableName))
	, m_structName(move(structName))
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderStruct::~ShaderStruct()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
