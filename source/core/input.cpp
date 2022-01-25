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
#include <windowsx.h>

// PROJECT INCLUDES
#include "../../include/core/input.h"
#include "../../include/scene/scene.h"
#include "../../include/core/coremanager.h"
#include "../../include/camera/camera.h"
#include "../../include/camera/cameramanager.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

Input::Input():
	  m_mouseX(-1)
	, m_mouseY(-1)
	, m_buttonState()
	, m_buttonSinglePressState()
	, m_keyState()
	, m_keySinglePressState()
	, m_settingCursorPosition(false)
	, m_mouseWheelDelta(0)
	, m_leftMouseButtonDown(false)
	, m_centerMouseButtonDown(false)
	, m_rightMouseButtonDown(false)
	, m_leftMouseButtonUp(false)
	, m_centerMouseButtonUp(false)
	, m_rightMouseButtonUp(false)
{
	// Camera keyboard controls
	m_eventSignalSlot.addKeyDownSignal(KeyCode::KEY_CODE_UP);
	m_eventSignalSlot.addKeyDownSignal(KeyCode::KEY_CODE_DOWN);
	m_eventSignalSlot.addKeyDownSignal(KeyCode::KEY_CODE_LEFT);
	m_eventSignalSlot.addKeyDownSignal(KeyCode::KEY_CODE_RIGHT);

	m_eventSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_UP);
	m_eventSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_DOWN);
	m_eventSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_LEFT);
	m_eventSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_RIGHT);

	m_eventSinglePressSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_UP);
	m_eventSinglePressSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_DOWN);
	m_eventSinglePressSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_LEFT);
	m_eventSinglePressSignalSlot.addKeyUpSignal(KeyCode::KEY_CODE_RIGHT);
}

/////////////////////////////////////////////////////////////////////////////////////////////

Input::~Input()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Input::updateInput(MSG& message)
{
	int32_t x = GET_X_LPARAM(message.lParam);
	int32_t y = GET_Y_LPARAM(message.lParam);

	uint8_t bestBtn = ButtonState(1) ? 1 : ButtonState(2) ? 2 : ButtonState(3) ? 3 : 0;

	switch (message.message)
	{
		case WM_KEYDOWN:
		{
			return KeyEvent(ActionCode::ACTION_CODE_DOWN, WIN32_TO_HID[message.wParam]);
		}
		case WM_KEYUP:
		{
			return KeyEvent(ActionCode::ACTION_CODE_UP, WIN32_TO_HID[message.wParam]);
		}
		case WM_MOUSEMOVE:
		{
			if (m_settingCursorPosition)
			{
				m_settingCursorPosition = false;
				return;
			}

			if (cameraM->getOperatingCamera())
			{
				cameraM->refCameraOperated()->update(x, y);
			}
			else
			{
				cameraM->refMainCamera()->update(x, y);
			}
			return MouseEvent(ActionCode::ACTION_CODE_MOVE, x, y, bestBtn, 0);
		}
		case WM_LBUTTONDOWN:
		{
			return MouseEvent(ActionCode::ACTION_CODE_DOWN, x, y, 1, 0);
		}
		case WM_MBUTTONDOWN:
		{
			return MouseEvent(ActionCode::ACTION_CODE_DOWN, x, y, 2, 0);
		}
		case WM_RBUTTONDOWN:
		{
			return MouseEvent(ActionCode::ACTION_CODE_DOWN, x, y, 3, 0);
		}
		case WM_LBUTTONUP:
		{
			return MouseEvent(ActionCode::ACTION_CODE_UP, x, y, 1, 0);
		}
		case WM_MBUTTONUP:
		{
			return MouseEvent(ActionCode::ACTION_CODE_UP, x, y, 2, 0);
		}
		case WM_RBUTTONUP:
		{
			return MouseEvent(ActionCode::ACTION_CODE_UP, x, y, 3, 0);
		}
		case WM_MOUSEWHEEL:
		{
			return MouseEvent(ActionCode::ACTION_CODE_MOUSE_WHEEL, x, y, 0, int8_t(GET_WHEEL_DELTA_WPARAM(message.wParam)));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Input::updateKeyboard()
{
	vector<KeyCode> vectorPressedKeyCopy = m_vectorPressedKey;

	forIT(m_vectorPressedKey)
	{
		m_eventSignalSlot.emitSignalForDownKey((*it));
	}

	vector<KeyCode> vectorSingleStatePressedKeyCopy = m_vectorSingleStatePressedKey;

	forIT(vectorSingleStatePressedKeyCopy)
	{
		m_eventSinglePressSignalSlot.emitSignalForDownKey((*it));
		m_keySinglePressState[static_cast<int>((*it))] = false;
	}

	vector<KeyCode> m_vectorSingleStateReleasedKeyCopy = m_vectorSingleStateReleaseddKey;

	forIT(m_vectorSingleStateReleasedKeyCopy)
	{
		m_eventSinglePressSignalSlot.emitSignalForUpKey((*it));
	}

	m_vectorPressedKey.clear();
	m_vectorSingleStatePressedKey.clear();
	m_vectorSingleStateReleaseddKey.clear();

	if (m_mouseWheelDelta)
	{
		if (m_mouseWheelDelta > 0)
		{
			m_eventSinglePressSignalSlot.refMouseWheelPositiveDelta().emit();
		}
		else
		{
			m_eventSinglePressSignalSlot.refMouseWheelNegativeDelta().emit();
		}

		m_mouseWheelDelta = 0;
	}

	if (m_leftMouseButtonDown)
	{
		m_eventSignalSlot.refLeftMouseButtonDown().emit();
		m_leftMouseButtonDown = false;
	}

	if (m_centerMouseButtonDown)
	{
		m_eventSignalSlot.refCenterMouseButtonDown().emit();
		m_centerMouseButtonDown = false;
	}

	if (m_rightMouseButtonDown)
	{
		m_eventSignalSlot.refRightMouseButtonDown().emit();
		m_rightMouseButtonDown = false;
	}

	if (m_leftMouseButtonUp)
	{
		m_eventSignalSlot.refLeftMouseButtonUp().emit();
		m_leftMouseButtonUp = false;
	}

	if (m_centerMouseButtonUp)
	{
		m_eventSignalSlot.refCenterMouseButtonUp().emit();
		m_centerMouseButtonUp = false;
	}

	if (m_rightMouseButtonUp)
	{
		m_eventSignalSlot.refRightMouseButtonUp().emit();
		m_rightMouseButtonUp = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Input::MouseEvent(ActionCode action, int32_t x, int32_t y, uint8_t btn, int8_t wheelDelta)
{
	m_mouseX = x;
	m_mouseY = y;

	if ((action == ActionCode::ACTION_CODE_DOWN) && !m_buttonState[btn])
	{
		m_buttonSinglePressState[btn] = true;
	}

	if ((action == ActionCode::ACTION_CODE_UP) || (action == ActionCode::ACTION_CODE_DOWN))
	{
		m_buttonState[btn] = (action == ActionCode::ACTION_CODE_DOWN);  // Keep track of button state

		if (action == ActionCode::ACTION_CODE_DOWN)
		{
			if (btn == 1)
			{
				m_leftMouseButtonDown = true;
			}
			else if (btn == 2)
			{
				m_centerMouseButtonDown = true;
			}
			else if (btn == 3)
			{
				m_rightMouseButtonDown = true;
			}
		}
		else if (action == ActionCode::ACTION_CODE_UP)
		{
			if (btn == 1)
			{
				m_leftMouseButtonUp = true;
			}
			else if (btn == 2)
			{
				m_centerMouseButtonUp = true;
			}
			else if (btn == 3)
			{
				m_rightMouseButtonUp = true;
			}
		}
	}

	if (action == ActionCode::ACTION_CODE_MOUSE_WHEEL)
	{
		m_mouseWheelDelta = wheelDelta;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Input::getCursorPos(int32_t &x, int32_t &y)
{
	x = m_mouseX;
	y = m_mouseY;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Input::setCursorPos(int32_t x, int32_t y)
{
	tagPOINT point;
	point.x = x;
	point.y = y;
	ClientToScreen(coreM->getWindowPlatformHandle(), &point);
	SetCursorPos(point.x, point.y);

	m_mouseX = x;
	m_mouseY = y;
}

/////////////////////////////////////////////////////////////////////////////////////////////
