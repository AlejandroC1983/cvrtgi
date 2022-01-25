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
#include "../../include/atomiccounter/atomiccounter.h"
#include "../../include/util/loopmacrodefines.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

AtomicCounter::AtomicCounter(string &&name) : GenericResource(move(name), move(string("AtomicCounter")), GenericResourceType::GRT_UNDEFINED),
	m_id(-1)
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////

AtomicCounter::~AtomicCounter()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void AtomicCounter::init()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void AtomicCounter::bind() const
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void AtomicCounter::unbind() const
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

uint AtomicCounter::getValue()
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void AtomicCounter::setValue(uint value)
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////
