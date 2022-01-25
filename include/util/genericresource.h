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

#ifndef _GENERICRESOURCE_H_
#define _GENERICRESOURCE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/parameter/multitypeunorderedmap.h"
#include "../../include/shader/resourceenum.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** This class is the top parent class of any resource class used in the framework, GPU wrappers, pipeline elements, etc.
* Simply adds a name and hashed name for the resource and the possibility of setting and getting parameters associated with
* the resource through an MultiTypeUnorderedMap member variable, this parameters will be the ones used to initialize the
* resource in the init() method. The m_parameterData variable must be supplied by the factory / class htat instantiates
* the resource class (in the constructor or setting it later through the setter method) */
class GenericResource
{
protected:
	/** Default constructor
	* @return nothing */
	GenericResource();

	/** Parameter constructor
	* @param name         [in] resource name
	* @param className    [in] class name
	* @param resourceType [in] resource type
	* @return nothing */
	GenericResource(string&& name, string&& className, GenericResourceType resourceType);

	/** Default destructor
	* @return nothing */
	virtual ~GenericResource();

	/** Initialize the resource
	* @return nothing */
	virtual void init();

	// TODO: use crtp to avoid this virtual method call
	/** Slot for managing added, elements signal
	* @param managerName      [in] name of the manager performing the notification
	* @param elementName      [in] name of the element added
	* @param notificationType [in] enum describing the type of notification
	* @return nothing */
	virtual void slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType);

public:
	GET(string, m_name, Name)
	GET(string, m_className, ClassName)
	GET(uint, m_hashedName, HashedName)
	GET_PTR(MultiTypeUnorderedMap, m_parameterData, ParameterData)
	SET_PTR(MultiTypeUnorderedMap, m_parameterData, ParameterData)
	GETCOPY_SET(bool, m_dirty, Dirty)
	GETCOPY_SET(bool, m_ready, Ready)
	GETCOPY(GenericResourceType, m_resourceType, ResourceType)

protected:
	/** Setter of m_sName, assigns the porper value also to m_hashedName
	* @param name [in] vlaue to set to m_sName
	* @return nothing */
	void setName(string&& name);

	SET(string, m_name, Name)
	
	string                 m_name;          //!< Name of the resource
	string                 m_className;     //!< Name of the C++ class this resource is an instance of (for derived classes)
	uint                   m_hashedName;    //!< Hashed name of the resource
	MultiTypeUnorderedMap* m_parameterData; //!< Parameter data for this resource, will be used during the initialization of the resource
	bool                   m_dirty;         //!< True if the resource needs to be rebuilt / has been modified
	bool                   m_ready;         //!< True if the resource is ready for use
	GenericResourceType    m_resourceType;  //!< Resource type
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _GENERICRESOURCE_H_
