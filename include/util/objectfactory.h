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

#ifndef _OBJECTFACTORY_H_
#define _OBJECTFACTORY_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;

// DEFINES
// To automatically register each class of the shader factory
//https://www.codeproject.com/Articles/363338/Factory-Pattern-in-Cplusplus
//http://stackoverflow.com/questions/9975672/c-automatic-factory-registration-of-derived-types
#define REGISTER_TYPE(baseClass, registerSubClassName) \
class registerSubClassName##IntoFactory : public ObjectFactory<baseClass> \
{ \
public: \
    registerSubClassName##IntoFactory() \
    { \
        baseClass##Factory::registerSubClass(string(#registerSubClassName), this); \
    } \
    virtual baseClass *create(string &&name) \
    { \
        return new registerSubClassName(move(name), move(string(#registerSubClassName))); \
    } \
}; \
static registerSubClassName##IntoFactory global_##registerSubClassName##IntoFactory;

// To allow each class built to automatically register new classes to be able to build new instances of the class
// (since the shader and subclass constructors are protected
#define DECLARE_FRIEND_REGISTERER(registerSubClassName) friend class registerSubClassName##IntoFactory;

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class ObjectFactory
{
public:
	/** Builds a new element, method used to build new elements from the factories (Shader, Material, etc)
	* @return pointer to the new built instance */
	virtual T* create(string &&name) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _OBJECTFACTORY_H_
