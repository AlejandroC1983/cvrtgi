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

#ifndef _SINGLETON_H_
#define _SINGLETON_H_

// GLOBAL INCLUDES
#include <string>

// PROJECT INCLUDES

// CLASS FORWARDING

// NAMESPACE

// DEFINES

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class Singleton
{
public:
	/** Return singleton instance's pointer
	* @return singleton instance's pointer */
	static T* instance();

	/** Build singleton instance
	* @return singleton instance's pointer */
	static T* init();

protected:
	/** Default constructor
	* @return nothing */
	Singleton() {}

	/** Default destructor
	* @return nothing */
	virtual ~Singleton() {}

private:
	/** Avoid implementation of reference parameter constructor
	* @return nothing */
	Singleton(Singleton const&);

	/** Avoid implementation of copy constructor
	* @return nothing */
	Singleton &operator=(Singleton const&);

	static T* m_instance; //!< Static variable, singleton instance
};

template <class T> T* Singleton<T>::m_instance = nullptr; // single initialization, in main.cpp now

/////////////////////////////////////////////////////////////////////////////////////////////

// INLINE METHODS

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline T *Singleton<T>::instance()
{
	return m_instance;
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline T *Singleton<T>::init()
{
	if (!m_instance)
	{
		m_instance = new T;
	}
	return m_instance;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _SINGLETON_H_
