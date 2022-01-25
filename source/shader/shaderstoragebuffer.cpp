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
#include "../../include/shader/shaderstoragebuffer.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderStorageBuffer::ShaderStorageBuffer(): ShaderStruct()
	, m_buffer(nullptr)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderStorageBuffer::ShaderStorageBuffer(string               &&name,
		                                 VkShaderStageFlagBits  shaderStage,
		                                 int                    bindingIndex,
		                                 int                    setIndex,
		                                 string               &&variableName,
		                                 string               &&structName) : 
	ShaderStruct(move(name),
				 move(string("ShaderStorageBuffer")),
		         ResourceInternalType::RIT_STRUCT,
		         shaderStage,
		         bindingIndex,
		         setIndex,
		         move(variableName),
		         move(structName))
	, m_buffer(nullptr)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

ShaderStorageBuffer::~ShaderStorageBuffer()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
