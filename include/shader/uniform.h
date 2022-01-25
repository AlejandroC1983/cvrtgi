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

#ifndef _UNIFORM_H_
#define _UNIFORM_H_

// GLOBAL LINCLUDES

// PROJECT INCLUDES
#include "../../include/shader/uniformBase.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T, int N> class Uniform: public UniformBase
{
public:

	typedef T type;
	const enum attributes
	{
		elementCount = N,
		elementSize = sizeof(T)
	};

	/** Parameter constructor, takes the id of the uniform, the name of the uniform and the internal type of theuniform
	* @param type               [in] enumerated with value the type of the uniform
	* @param shaderStage        [in] enumerated with value the shader stage the uniform being built is used
	* @param name               [in] name of the uniform
	* @param structName         [in] name of the struct whose field contains this variable
	* @param structType         [in] name of the type of the struct whose field contains this variable
	* @param shaderStructOwner  [in] pointer to the shader struct owner of the variable to make (the struct in the shader this variable is defined inside)
	* @return nothing */
	Uniform(ResourceInternalType type, VkShaderStageFlagBits shaderStage, string&& name, string&& structName, string&& structType, ShaderStruct* shaderStructOwner);

	/** Sets the value given by the value parameter in the m_elements member variable
	* @param value [in] value to set
	* @return nothing */
	void setValue(T *value);

	/** Gets a pouithe value given by the value parameter in the m_elements member variable
	* @param value [in] value to set
	* @return nothing */
	const T *getValuePtr() const;

protected:
	T m_elements[elementCount]; //!< value of the uniform
};

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T, int N> Uniform<T, N>::Uniform(ResourceInternalType type, VkShaderStageFlagBits shaderStage, string&& name, string&& structName, string&& structType, ShaderStruct* shaderStructOwner):
	UniformBase(type, shaderStage, move(name), move(string("Uniform")), move(structName), move(structType), shaderStructOwner)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T, int N> inline void Uniform<T, N>::setValue(T *value)
{
	memcpy(m_elements, value, attributes::elementCount * attributes::elementSize);
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T, int N> inline const T *Uniform<T, N>::getValuePtr() const
{
	return &m_elements[0];
}

/////////////////////////////////////////////////////////////////////////////////////////////

typedef Uniform<float,  1>   UniformFloat;
typedef Uniform<float,  2>   UniformFloat2;
typedef Uniform<float,  3>   UniformFloat3;
typedef Uniform<float,  4>   UniformFloat4;
typedef Uniform<double, 1>   UniformDouble;
typedef Uniform<double, 2>   UniformDouble2;
typedef Uniform<double, 3>   UniformDouble3;
typedef Uniform<double, 4>   UniformDouble4;
typedef Uniform<int,    1>   UniformInt;
typedef Uniform<int,    2>   UniformInt2;
typedef Uniform<int,    3>   UniformInt3;
typedef Uniform<int,    4>   UniformInt4;
typedef Uniform<uint,   1>   UniformUInt;
typedef Uniform<uint,   2>   UniformUInt2;
typedef Uniform<uint,   3>   UniformUInt3;
typedef Uniform<uint,   4>   UniformUInt4;
typedef Uniform<bool,   1>   UniformBool;
typedef Uniform<bool,   2>   UniformBool2;
typedef Uniform<bool,   3>   UniformBool3;
typedef Uniform<bool,   4>   UniformBool4;
typedef Uniform<float,  4>   UniformMatF2;
typedef Uniform<float,  9>   UniformMatF3;
typedef Uniform<float,  16>  UniformMatF4;
typedef Uniform<float,  6>   UniformMatF2x3;
typedef Uniform<float,  8>   UniformMatF2x4;
typedef Uniform<float,  6>   UniformMatF3x2;
typedef Uniform<float,  12>  UniformMatF3x4;
typedef Uniform<float,  8>   UniformMatF4x2;
typedef Uniform<float,  12>  UniformMatF4x3;
typedef Uniform<double, 4>   UniformMatD2;
typedef Uniform<double, 9>   UniformMatD3;
typedef Uniform<double, 16>  UniformMatD4;
typedef Uniform<double, 6>   UniformMatD2x3;
typedef Uniform<double, 8>   UniformMatD2x4;
typedef Uniform<double, 6>   UniformMatD3x2;
typedef Uniform<double, 12>  UniformMatD3x4;
typedef Uniform<double, 8>   UniformMatD4x2;
typedef Uniform<double, 12>  UniformMatD4x3;

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _UNIFORM_H_
