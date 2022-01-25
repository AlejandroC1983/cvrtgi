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
#include "../../include/shader/atomiccounterunit.h"
#include "../../include/atomiccounter/atomiccounter.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

AtomicCounterUnit::AtomicCounterUnit(const int id, string &&name, const ResourceInternalType atomicCounterType, VkShaderStageFlagBits shaderStage):
	m_id(id),
	m_atomicCounterType(atomicCounterType),
	m_shaderStage(shaderStage),
	m_hasAssignedAtomicCounter(false),
	m_atomicCounter(nullptr),
	m_name(move(name)),
	m_atomicCounterToBindName(string(""))
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

bool AtomicCounterUnit::setAtomicCounter(AtomicCounter *atomicCounter)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool AtomicCounterUnit::setAtomicCounter(string &&name)
{
	return m_hasAssignedAtomicCounter;
}

/////////////////////////////////////////////////////////////////////////////////////////////

AtomicCounterUnit::~AtomicCounterUnit()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
