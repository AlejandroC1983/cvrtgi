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

#ifndef _FACTORYTEMPLATE_H_
#define _FACTORYTEMPLATE_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/objectfactory.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class FactoryTemplate
{
public:
	/** Adds the pair <string, ObjectFactory>(name, buildMethod) if not present to m_mapFactory, allowing new elements of that Shader subclass
	* to be instantiated
	* @param name	     [in] name of the Shader subclass
	* @param buildMethod [in] class with a build method to generate new instances of this Shader subclass
	* @return true if added successfully and false otherwise */
	static bool registerSubClass(string &&name, ObjectFactory<T> *buildMethod);

	/** Uses the m_mapFactory to build an already registered class
	* @param name         [in] name of the class to build a new instance
	* @param instanceName [in] name of the instance to build
	* @return a pointer to the newly built class if success and nullptr otherwise */
	static T *buildElement(string &&name);

protected:
	static map<string, ObjectFactory<T>*> m_mapFactory; //!< Map used to generate the new instances of the classes from this factory
};

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline bool FactoryTemplate<T>::registerSubClass(string &&name, ObjectFactory<T> *buildMethod)
{
	bool result = addIfNoPresent(move(name), buildMethod, m_mapFactory);

	if (!result)
	{
		cout << "ERROR: failed trying to register a class " << name << endl;
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline T *FactoryTemplate<T>::buildElement(string &&name)
{
	auto it = m_mapFactory.find(name);

	if (it != m_mapFactory.end())
	{
		return it->second->create(move(name));
	}

	cout << "ERROR: trying to instantiate a non registered shader class " << name << endl;
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _FACTORYTEMPLATE_H_
