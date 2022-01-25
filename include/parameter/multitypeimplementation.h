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

#ifndef _MULTITYPEIMPLEMENTATION_H_
#define _MULTITYPEIMPLEMENTATION_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/parameter/multitypebase.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

/** Templatized class to store elements in the MultiUnorderedMap */

template <typename T> class MultiTypeImplementation: public MultiTypeBase
{
public:
	virtual ~MultiTypeImplementation() = default;

	/** Assigns element to m_data
	* @return nothing */
	template <typename T> void assignData(T element)
	{
		m_data = element;
	}

	/** Getter of m_data
	* @return copy of m_data */
	T getData()
	{
		return m_data;
	}

	/** Getter of m_data
	* @return copy of m_data */
	const T getData() const
	{
		return m_data;
	}

protected:
	T m_data; //!< Templatized data element to be added to the MultiTypeUnorderedMap
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MULTITYPEIMPLEMENTATION_H_
