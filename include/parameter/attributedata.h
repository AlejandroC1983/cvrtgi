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
#ifndef _ATTRIBUTEDATA_H_
#define _ATTRIBUTEDATA_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** Basic struct to interface with MultiTypeUnorderedMap, adding and retrieving / removing elements from
* a MultiTypeUnorderedMap variable that constitutes the multy type container to store all needed parameters */

template <class T> struct AttributeData
{
	AttributeData(string&& name, T&& data):
		m_name(move(name)),
		m_hashedName(uint(hash<string>()(m_name))),
		m_data(move(data))
	{

	}

	string m_name;       //!< Name of this struct
	uint   m_hashedName; //!< Hashed version of the name of this struct
	T      m_data;       //!< Data represented by this struct
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _ATTRIBUTEDATA_H_
