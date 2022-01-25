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

#ifndef _PUSH_CONSTANT_H_
#define _PUSH_CONSTANT_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/genericresource.h"
#include "../../include/uniformbuffer/cpubuffer.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class PushConstant
{
public:
	/** Default constructor
	* @return nothing */
	PushConstant();

	/** Destructor
	* @return nothing */
	virtual ~PushConstant();

	REF(vectorUniformBasePtr, m_vecUniformBase, VecUniformBase)
	REF(CPUBuffer, m_CPUBuffer, CPUBuffer)
	GETCOPY_SET(VkShaderStageFlags, m_shaderStages, ShaderStages)

public:
	vectorUniformBasePtr m_vecUniformBase; //!< Vector with the uniform variables present in the push constant struct
	CPUBuffer            m_CPUBuffer;      //!< CPU bufer mapping the information of the GPU buffer
	VkShaderStageFlags   m_shaderStages;   //!< All shader stages in which the push constant is used (assuming a single push constant per whole shader, with no differencies between sgader stages)
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _PUSH_CONSTANT_H_
