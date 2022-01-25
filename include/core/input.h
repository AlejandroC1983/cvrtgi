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

#ifndef _INPUT_H_
#define _INPUT_H_

// GLOBAL INCLUDES
#include "../../external/nano-signal-slot/nano_signal_slot.hpp"

// PROJECT INCLUDES
#include "../headers.h"
#include "../../include/util/singleton.h"
#include "../../include/util/getsetmacros.h"
#include "../../include/core/eventsignalslot.h"
#include "../../include/core/coreenum.h"
#include "../../include/util/containerutilities.h"

// CLASS FORWARDING

// NAMESPACE
using namespace coreenum;

// DEFINES
#define inputM s_pInputSingleton->instance()

/////////////////////////////////////////////////////////////////////////////////////////////

class Input : public Singleton<Input>
{
public:
	/** Default constructor
	* @return nothing */
	Input();

	/** Default destructor
	* @return nothing */
	~Input();

	/** Update input according to the Windows message given by message
	* @param message [in] message to process
	* @return nothing */
	void updateInput(MSG& message);

	/** Update keyboard input from user
	* @return nothing */
	void updateKeyboard();

	/** Returns the cursor position
	* @param x [out] x posiiton of the cursor
	* @param y [out] y posiiton of the cursor
	* @return nothing */
	void getCursorPos(int32_t &x, int32_t &y);

	/** Sets the cursor position
	* @param x [out] x position of the cursor
	* @param y [out] y position of the cursor
	* @return nothing */
	void setCursorPos(int32_t x, int32_t y);

	/** Method called in updateInput when there're events of mouse movement of mouse button
	* down / up
	* @param action     [in] Type of event regarding the mouse
	* @param x          [in] Mouse x screen coordinate
	* @param y          [in] Mouse y screen coordinate
	* @param btn        [in] Mouse button pressed / released
	* @param wheelDelta [in] Mouse wheel delta sign (if any)
	* @return nothing */
	void MouseEvent(ActionCode action, int32_t x, int32_t y, uint8_t btn, int8_t wheelDelta);

	/** Method called in updateInput when there're events of key down / up
	* @param action [in] Type of event regarding the key given by the key parameter
	* @param key    [in] Key involved in the event
	* @return nothing */
	void KeyEvent(ActionCode action, uint8_t key);

	/** Returns the state of the buttons of the mouse
	* @param button [in] Button to test
	* @return true of pressed, false if released */
	bool ButtonState(uint8_t  button);

	/** Returns the state of the buttons of the mouse for the single press version
	* @param button [in] Button to test
	* @return true of pressed, false if released */
	bool ButtonSinglePressState(uint8_t  button);

	GETCOPY(int32_t, m_mouseX, MouseX)
	GETCOPY(int32_t, m_mouseY, MouseY)
	REF(EventSignalSlot, m_eventSignalSlot, EventSignalSlot)
	REF(EventSignalSlot, m_eventSinglePressSignalSlot, EventSinglePressSignalSlot)

protected:
	int32_t         m_mouseX;                        //!< Mouse position x
	int32_t         m_mouseY;                        //!< Mouse position y
	bool            m_buttonState[5];                //!< mouse button state
	bool            m_buttonSinglePressState[5];     //!< mouse button state for single press events
	bool            m_keyState[256];                 //!< Mapping of each key's state
	bool            m_keySinglePressState[256];      //!< Mapping of each key's state for single press events
	bool            m_settingCursorPosition;         //!< To know when the application is setting the cursor position, to avoid processing events regarding this particular event
	int8_t          m_mouseWheelDelta;               //!< Mouse wheel delta event value (if any)
	bool            m_leftMouseButtonDown;           //!< Flag to register left mouse button down
	bool            m_centerMouseButtonDown;         //!< Flag to register center mouse button down
	bool            m_rightMouseButtonDown;          //!< Flag to register right mouse button down
	bool            m_leftMouseButtonUp;             //!< Flag to register left mouse button up
	bool            m_centerMouseButtonUp;           //!< Flag to register center mouse button up
	bool            m_rightMouseButtonUp;            //!< Flag to register right mouse button up
	EventSignalSlot m_eventSignalSlot;               //!< To notify for mouse movement and mouse and keyboard down and up events
	EventSignalSlot m_eventSinglePressSignalSlot;    //!< To notify for mouse movement and mouse and keyboard down and up events for single press events
	vector<KeyCode> m_vectorPressedKey;              //!< Vector with the currently pressed keys
	vector<KeyCode> m_vectorSingleStatePressedKey;   //!< Vector with the currently pressed keys for single press events
	vector<KeyCode> m_vectorSingleStateReleaseddKey; //!< Vector with the currently released keys for single press events
};

static Input* s_pInputSingleton;

/////////////////////////////////////////////////////////////////////////////////////////////

// INLINE FUNCTIONS

/////////////////////////////////////////////////////////////////////////////////////////////

inline void Input::KeyEvent(ActionCode action, uint8_t key)
{
	if ((action == ActionCode::ACTION_CODE_DOWN) && !m_keySinglePressState[key] &&
		m_eventSinglePressSignalSlot.isKeyDownRegistered(static_cast<KeyCode>(key)))
	{
		addIfNoPresent(static_cast<KeyCode>(key), m_vectorSingleStatePressedKey);
		m_keySinglePressState[key] = true;
	}

	if ((action == ActionCode::ACTION_CODE_UP) && !m_keySinglePressState[key] &&
		m_eventSinglePressSignalSlot.isKeyUpRegistered(static_cast<KeyCode>(key)))
	{
		addIfNoPresent(static_cast<KeyCode>(key), m_vectorSingleStateReleaseddKey);
		m_keySinglePressState[key] = false;
	}

	m_keyState[key] = (action == ActionCode::ACTION_CODE_DOWN);

	if (action == ActionCode::ACTION_CODE_DOWN)
	{
		addIfNoPresent(static_cast<KeyCode>(key), m_vectorPressedKey);
	}
	else
	{
		removeIfPresent(static_cast<KeyCode>(key), m_vectorPressedKey);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline bool Input::ButtonState(uint8_t button)
{
	if (button < 3)
	{
		return m_buttonState[button];
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

inline bool Input::ButtonSinglePressState(uint8_t button)
{
	if (button < 3)
	{
		return m_buttonSinglePressState[button];
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

#endif _INPUT_H_
