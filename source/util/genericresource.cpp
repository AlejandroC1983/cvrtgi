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
#include "../../include/util/genericresource.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

GenericResource::GenericResource() :
	  m_name("")
	, m_className("")
	, m_hashedName(0)
	, m_parameterData(nullptr)
	, m_dirty(false)
	, m_ready(false)
	, m_resourceType(GenericResourceType::GRT_UNDEFINED)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

GenericResource::GenericResource(string&& name, string&& className, GenericResourceType resourceType) :
	  m_name(move(name))
	, m_className(move(className))
	, m_dirty(false)
	, m_ready(false)
	, m_resourceType(resourceType)
	, m_parameterData(nullptr)
{
	m_hashedName = uint(hash<string>()(m_name));
}

/////////////////////////////////////////////////////////////////////////////////////////////

GenericResource::~GenericResource()
{
	if (m_parameterData != nullptr)
	{
		delete m_parameterData;
		m_parameterData = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GenericResource::init()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void GenericResource::setName(string&& name)
{
	m_name       = move(name);
	m_hashedName = uint(hash<string>()(m_name));
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GenericResource::slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
