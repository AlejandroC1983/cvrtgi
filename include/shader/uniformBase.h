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

#ifndef _UNIFORMBASE_H_
#define _UNIFORMBASE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING
class ShaderStruct;

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class UniformBase : public GenericResource
{
public:
	/** Parameter constructor
	* @param type              [in] enumerated with value the type of the uniform
	* @param shaderStage       [in] enumerated with value the shader stage the uniform being built is used
	* @param name              [in] name of the uniform
	* @param className         [in] class name
	* @param structName        [in] name of the struct whose field contains this variable
	* @param structType        [in] name of the type of the struct whose field contains this variable
	* @param shaderStructOwner [in] pointer to the shader struct owner of the variable to make (the struct in the shader this variable is defined inside)
	/* @return nothing */
	UniformBase(const ResourceInternalType type, VkShaderStageFlagBits shaderStage, string&& name, string&& className, string&& structName, string&& structType, ShaderStruct* shaderStructOwner);

	GET(ResourceInternalType, m_type, ResourceInternalType)
	GET(VkShaderStageFlagBits, m_shaderStage, ShaderStage)
	GET_PTR(ShaderStruct, m_shaderStructOwner, ShaderStructOwner)
	GET(string, m_name, Name)
	GET(string, m_structName, StructName)
	GET(string, m_structType, StructType)

protected:
	ResourceInternalType  m_type;              //!< type of the uniform
	VkShaderStageFlagBits m_shaderStage;       //!< Shader stage this uniform is being used
	ShaderStruct*         m_shaderStructOwner; //!< Pointer to the wrapper of the uniform buffer this uniform is defined in the shader
	string                m_name;              //!< name of the uniform
	string                m_structName;        //!< name of the struct whose field contains this variable
	string                m_structType;        //!< name of the type of the struct whose field contains this variable
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _UNIFORMBASE_H_
