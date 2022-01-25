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
#include "../../include/material/exposedstructfield.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

ExposedStructField::ExposedStructField() :
	m_internalType(ResourceInternalType::RIT_SIZE)
	, m_data(nullptr)
	, m_structFieldResource(nullptr)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

ExposedStructField::ExposedStructField(ResourceInternalType internalType, string&& structName, string&& structFieldName, void* data) :
	m_internalType(internalType)
	, m_data(data)
	, m_structFieldResource(nullptr)
	, m_structName(move(structName))
	, m_structFieldName(move(structFieldName))
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
