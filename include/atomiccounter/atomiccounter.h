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

#ifndef _ATOMICCOUNTER_H_
#define _ATOMICCOUNTER_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/util/genericresource.h"

// CLASS FORWARDING

// NAMESPACE

// DEFINES


/////////////////////////////////////////////////////////////////////////////////////////////

/** Atomic counter class wrapper */
class AtomicCounter : public GenericResource
{
protected:
	/** Parameter constructor
	* @param [in] atomic counter's name
	* @return nothing */
	AtomicCounter(string &&name);

	/** Default destructor
	* @return nothing */
	virtual ~AtomicCounter();

public:
	/** Init resource
	* @return nothing */
	void init();

	/** Binds this atomic counter
	* @return nothing */
	void bind() const;

	/** Unbinds the atomic counter
	* @return nothing */
	void unbind() const;

	/** Recovers from GPU memory the value of this atomic counter
	* @return nothing */
	uint getValue();

	/** Sets to GPU memory the value of this atomic counter
	* @param value [in] value to set
	* @return nothing */
	void setValue( uint value );

	GET(int, m_id, Id)

protected:
	SET(int, m_id, Id)
	
	int m_id; //!< Id of the resource
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _ATOMICCOUNTER_H_
