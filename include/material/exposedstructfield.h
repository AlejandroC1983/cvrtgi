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

#ifndef _EXPOSEDSTRUCTFIELD_H_
#define _EXPOSEDSTRUCTFIELD_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/commonnamespace.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING
class UniformBase;

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

// TODO: put in separate file and add proper methods
class ExposedStructField
{
public:
	/** Default constructor
	* @return nothing */
	ExposedStructField();

	/** Parameter constructor
	* @param internalType    [in] enum with the internal type of this variable being exposed
	* @param structName      [in] name of the struct in the shader that contains the struct member this variable will be linked with
	* @param structFieldName [in] pointer to the CPU copy of the variable in the shader that represents the struct member in the shader, for lazy updating
	* @param data            [in] pointer to the member variable to expose
	* @return nothing */
	ExposedStructField(ResourceInternalType internalType, string&& structName, string&& structFieldName, void* data);

	GETCOPY(ResourceInternalType, m_internalType, InternalType)
	GET_PTR(void, m_data, Data)
	GET_PTR(UniformBase, m_structFieldResource, StructFieldResource)
	REF_PTR(UniformBase, m_structFieldResource, StructFieldResource)
	SET_PTR(UniformBase, m_structFieldResource, StructFieldResource)
	GET(string, m_structName, StructName)
	GET(string, m_structFieldName, StructFieldName)

protected:
	ResourceInternalType m_internalType;        //!< Exposed variable internal type (should be one of the glm library types or a C++ fundamental type present in ResourceInternalType)
	void*                m_data;                //!< Void pointer to the exposed variable
	UniformBase*         m_structFieldResource; //!< Pointer to the resource in the shader that represents a struct field copy in C++ to link the member variable m_data with
	string               m_structName;          //!< Name of the struct in the shader the exposed variable will be linked with
	string               m_structFieldName;     //!< Name of the field in the struct in the shader the exposed variable will be linked with
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _EXPOSEDSTRUCTFIELD_H_
