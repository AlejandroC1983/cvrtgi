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

#ifndef _EVENTSIGNALSLOT_H_
#define _EVENTSIGNALSLOT_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../../include/util/getsetmacros.h"
#include "../../include/core/coreenum.h"
#include "../../include/commonnamespace.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;
using namespace coreenum;

// DEFINES
typedef Nano::Signal<void()> SignalVoid;
typedef Nano::Signal<void()> SignalVoidIntInt;

/////////////////////////////////////////////////////////////////////////////////////////////

class EventSignalSlot
{
	friend class Input;

public:
	/** Default destructor
	* @return nothing */
	~EventSignalSlot();

	/** Returns the mapped value of m_mapKeyDown[key]
	* @param key [in] key to look for at m_mapKeyDown
	* @return pointer to the signal if any, nullptr otherwise */
	SignalVoid* refKeyDownSignalByKey(KeyCode key);

	/** Returns the mapped value of m_mapKeyUp[key]
	* @param key [in] key to look for at m_mapKeyUp
	* @return pointer to the signal if any, nullptr otherwise */
	SignalVoid* refKeyUpSignalByKey(KeyCode key);

	/** Adds a new signal to m_mapKeyDown to be able to attach slots and call them
	* @param key [in] key to add
	* @return nothing */
	void addKeyDownSignal(KeyCode key);

	/** Adds a new signal to m_mapKeyUp to be able to attach slots and call them
	* @param key [in] key to add
	* @return nothing */
	void addKeyUpSignal(KeyCode key);

	/** Emits signal for all the slots present in the mapped value m_mapKeyDown[key]
	* @param key [in] key to emit signal for in m_mapKeyDown
	* @return nothing */
	void emitSignalForDownKey(KeyCode key);

	/** Emits signal for all the slots present in the mapped value m_mapKeyUp[key]
	* @param key [in] key to emit signal for in m_mapKeyUp
	* @return nothing */
	void emitSignalForUpKey(KeyCode key);

	/** Returns true if the key given as parameter is registered fo ra key down event and false otherwise
	* @return true if the key given as parameter is registered fo ra key down event and false otherwise */
	bool isKeyDownRegistered(KeyCode key);

	/** Returns true if the key given as parameter is registered for a key up event and false otherwise
	* @return true if the key given as parameter is registered fo ra key up event and false otherwise */
	bool isKeyUpRegistered(KeyCode key);

	REF(SignalVoidIntInt, m_mouseMove, MouseMove)
	REF(SignalVoid, m_leftMouseButtonDown, LeftMouseButtonDown)
	REF(SignalVoid, m_centerMouseButtonDown, CenterMouseButtonDown)
	REF(SignalVoid, m_rightMouseButtonDown, RightMouseButtonDown)
	REF(SignalVoid, m_leftMouseButtonUp, LeftMouseButtonUp)
	REF(SignalVoid, m_rightMouseButtonUp, RightMouseButtonUp)
	REF(SignalVoid, m_centerMouseButtonUp, CenterMouseButtonUp)
	REF(SignalVoid, m_mouseWheelPositiveDelta, MouseWheelPositiveDelta)
	REF(SignalVoid, m_mouseWheelNegativeDelta, MouseWheelNegativeDelta)

protected:
	/** Returns the mapped value of mapKey[key]
	* @param mapKey [in] map to look for the key parameter key value
	* @param key    [in] key to look for at mapKey
	* @return pointer to the signal if any, nullptr otherwise */
	SignalVoid* refKeyUpSignalByKey(map<int, SignalVoid*>& mapKey, KeyCode key);

	/** Adds a new signal to mapKey to be able to attach slots and call them
	* @param key [in] key to add to mapKey
	* @return nothing */
	void addKeySignal(map<int, SignalVoid*>& mapKey, KeyCode key);

	/** Emits a signal for mapKey[key]
	* @param key [in] key to add to emit the signal
	* @return nothing */
	void emitKeySignal(map<int, SignalVoid*>& mapKey, KeyCode key);

	SignalVoidIntInt      m_mouseMove;               //!< Signal for mouse movement
	SignalVoid            m_leftMouseButtonDown;     //!< Signal for left mouse button down
	SignalVoid            m_centerMouseButtonDown;   //!< Signal for center mouse button down
	SignalVoid            m_rightMouseButtonDown;    //!< Signal for right mouse button down
	SignalVoid            m_leftMouseButtonUp;       //!< Signal for left mouse button up
	SignalVoid            m_rightMouseButtonUp;      //!< Signal for right mouse button up
	SignalVoid            m_centerMouseButtonUp;     //!< Signal for center mouse button up
	SignalVoid            m_mouseWheelPositiveDelta; //!< Signal for mouse wheel positive delta
	SignalVoid            m_mouseWheelNegativeDelta; //!< Signal for mouse wheel negative delta
	map<int, SignalVoid*> m_mapKeyDown;              //!< Map for signaling keys down
	map<int, SignalVoid*> m_mapKeyUp;                //!< Map for signaling keys up
};

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _EVENTSIGNALSLOT_H_
