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

#ifndef _SHADER_STRUCT_H_
#define _SHADER_STRUCT_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

class ShaderStruct : public GenericResource
{
public:
	/** Default constructor
	* @return nothing */
	ShaderStruct();

	/** Parameter constructor
	* @param name              [in] uniform buffer variable name (hasn't to do with declared variables in the shader, is this C++'s class instance name)
	* @param className         [in] class name (for derived classes)
	* @param uniformBufferType [in] enumerated with value the type of the struct
	* @param shaderStage       [in] enumerated with value the shader stage the uniform being built is used
	* @param bindingIndex      [in] binding index of this uniform buffer for the shader it's owned by
	* @param setIndex          [in] set index inside the binding index given by bindingIndex of this uniform buffer for the shader it's owned by
	* @param uniformBufferType [in] uniform buffer type
	* @param variableName      [in] name of the variable in the shader
	* @param structName        [in] name of the variable in the struct
	* @return nothing */
	ShaderStruct(string               &&name,
				 string               &&className,
		         ResourceInternalType   uniformBufferType,
		         VkShaderStageFlagBits  shaderStage,
		         int                    bindingIndex,
		         int                    setIndex,
		         string               &&variableName,
		         string               &&structName);

	/** Default destructor
	* @return nothing */
	~ShaderStruct();
	
	GETCOPY(ResourceInternalType, m_uniformBufferType, UniformBufferType)
	GET(VkShaderStageFlagBits, m_shaderStage, ShaderStage)
	GETCOPY(int, m_bindingIndex, BindingIndex)
	GETCOPY(int, m_setIndex, SetIndex)
	GETCOPY(string, m_variableName, VariableName)
	GETCOPY(string, m_structName, StructName)

protected:
	ResourceInternalType  m_uniformBufferType; //!< sampler type
	VkShaderStageFlagBits m_shaderStage;       //!< enumerated with value the shader stage the uniform being built is used
	int                   m_bindingIndex;      //!< Binding index for this uniform buffer in the shader its owned by
	int                   m_setIndex;          //!< Set index inside m_bindingIndex for this uniform buffer in the shader its owned by
	string                m_variableName;      //!< Name of the variable of type equal to m_structName declared in the shader for this uniform buffer (i.e. "myStruct" in struct S { float f; }; S myStruct)
	string                m_structName;        //!< Name of the defined struct in the shader this uniform buffer has a variable declared (i.e. "S" in struct S { float f; }; S myStruct)
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SHADER_STRUCT_H_
