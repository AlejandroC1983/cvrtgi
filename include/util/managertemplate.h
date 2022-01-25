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

#ifndef _MANAGERTEMPLATE_H_
#define _MANAGERTEMPLATE_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/util/containerutilities.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/commonnamespace.h"
#include "../../include/shader/resourceenum.h" // TODO: put ManagerNotificationType in proper namespace file

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;
using namespace resourceenum;

// DEFINES
typedef Nano::Signal<void(const char*, string&&, ManagerNotificationType)> SignalElementNotification;

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> class ManagerTemplate
{
public:
	/** Destructor
	* @return nothing */
	virtual ~ManagerTemplate();

	/** Add a new elment to m_mapElement if it does not exist, returning true if the element was successfully added and false otherwise
	* @param name    [in] name of the element to add
	* @param element [in] element to add
	* @return true if the pair (name, element) was successfully added and false otherwise */
	bool addElement(string &&name, T *element);

	/** Erases the elment with name given as parameter from m_mapElement if it does exist, returning true if the element was successfully removed
	*   and false otherwise
	* @param name [in] name of the element to remove
	* @return true if the element was successfully removed and false otherwise */
	bool removeElement(string &&name);

	/** Returns a pointer to the elment with name given as parameter from m_mapElement if it does exist, or nullptr if it does not exist
	* @param name [in] name of the element to return
	* @return pointer to an element of m_mapElement if it does exist and nullptr otherwise */
	T *getElement(string &&name);

	/** Returns true if the elment with name given as parameter exists in m_mapElement, and false if it doesn't
	* @param name [in] name of the element to test if it's present in m_mapElement
	* @return true if the elment with name given as parameter exists in m_mapElement, and false if it doesn't */
	bool existsElement(string &&name);

	/** Returns a const vector with pointers to the elements in m_mapElement
	* @return const vector with pointers to the elements in m_mapElement */
	const vector<T*> getVectorElement() const;

	/** Returns a const vector with pointers to the elements in m_mapElement with class name given by name
	* @return const vector with pointers to the elements in m_mapElement with class name given by name */
	const vector<T*> getElementByClassName(string&& name) const;

	REF(SignalElementNotification, m_elementSignal, ElementSignal)

protected:
	/** Emits signal for all slots in m_newElementSignal regarding a new
	* element added named elementName
	* @param elementName      [in] name of the element added
	* @param NotificationType [in] enum with the type of notification being emitted
	* @return nothing */
	void emitSignalElement(string&& elementName, ManagerNotificationType NotificationType);

	// TODO: use crtp to avoid this virtual method call
	/** Assigns the corresponding slots to listen to signals affecting the resources
	* managed by this manager */
	virtual void assignSlots();

	// TODO: use crtp to avoid this virtual method call
	/** Slot for managing added, elements signal
	* @param managerName      [in] name of the manager performing the notification
	* @param elementName      [in] name of the element added
	* @param notificationType [in] enum describing the type of notification
	* @return nothing */
	virtual void slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType);

	map<string, T*>           m_mapElement;    //!< A map with the elements to the managed by the implementation of this manager, string should be really a shashed version of the string
	const char*               m_managerName;   //!< Manager name
	SignalElementNotification m_elementSignal; //!< Signal for elements in the manager: added, removed, changed
};

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> ManagerTemplate<T>::~ManagerTemplate()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline bool ManagerTemplate<T>::addElement(string &&name, T *element)
{
	bool result = addIfNoPresent(move(string(name)), element, m_mapElement);
	if (result)
	{
		emitSignalElement(move(name), ManagerNotificationType::MNT_ADDED);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline bool ManagerTemplate<T>::removeElement(string &&name)
{
	bool result = removeByKey(move(name), m_mapElement);

	if (result)
	{
		emitSignalElement(move(name), ManagerNotificationType::MNT_REMOVED);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline T *ManagerTemplate<T>::getElement(string &&name)
{
	return getByKey(move(name), m_mapElement);
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline bool ManagerTemplate<T>::existsElement(string &&name)
{
	return existsByKey(move(name), m_mapElement);
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline const vector<T*> ManagerTemplate<T>::getVectorElement() const
{
	vector<T*> vectorResult;

	forIT(m_mapElement)
	{
		vectorResult.push_back(it->second);
	}

	return vectorResult;
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline const vector<T*> ManagerTemplate<T>::getElementByClassName(string&& name) const
{
	vector<T*> vectorResult;

	forIT(m_mapElement)
	{
		if (it->second->getClassName() == name)
		{
			vectorResult.push_back(it->second);
		}
	}

	return vectorResult;
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline void ManagerTemplate<T>::emitSignalElement(string&& elementName, ManagerNotificationType notificationType)
{
	m_elementSignal.emit(m_managerName, move(string(elementName)), notificationType);
}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline void ManagerTemplate<T>::assignSlots()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

template <class T> inline void ManagerTemplate<T>::slotElement(const char* managerName, string&& elementName, ManagerNotificationType notificationType)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _MANAGERTEMPLATE_H_
