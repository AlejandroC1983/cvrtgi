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

#ifndef _COREENUM_H_
#define _COREENUM_H_

// GLOBAL INCLUDES

// PROJECT INCLUDES
#include "../../include/headers.h"
#include "../../include/commonnamespace.h"
#include "../../include/rastertechnique/rastertechniqueenum.h"

// CLASS FORWARDING

// NAMESPACE
using namespace commonnamespace;
using namespace rastertechniqueenum;

// DEFINES

namespace coreenum
{
	/////////////////////////////////////////////////////////////////////////////////////////////

	/** keyboard / mouse / touchscreen actions */
	enum class ActionCode
	{
		ACTION_CODE_UP = 0,
		ACTION_CODE_DOWN,
		ACTION_CODE_MOVE,
		ACTION_CODE_MOUSE_WHEEL
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Key mapped codes */
	enum class KeyCode
	{
		KEY_CODE_NONE           = 0, // Undefined. (No event)
		KEY_CODE_A              = 4,
		KEY_CODE_B              = 5,
		KEY_CODE_C              = 6,
		KEY_CODE_D              = 7,
		KEY_CODE_E              = 8,
		KEY_CODE_F              = 9,
		KEY_CODE_G              = 10,
		KEY_CODE_H              = 11,
		KEY_CODE_I              = 12,
		KEY_CODE_J              = 13,
		KEY_CODE_K              = 14,
		KEY_CODE_L              = 15,
		KEY_CODE_M              = 16,
		KEY_CODE_N              = 17,
		KEY_CODE_O              = 18,
		KEY_CODE_P              = 19,
		KEY_CODE_Q              = 20,
		KEY_CODE_R              = 21,
		KEY_CODE_S              = 22,
		KEY_CODE_T              = 23,
		KEY_CODE_U              = 24,
		KEY_CODE_V              = 25,
		KEY_CODE_W              = 26,
		KEY_CODE_X              = 27,
		KEY_CODE_Y              = 28,
		KEY_CODE_Z              = 29,
		KEY_CODE_1              = 30, // 1 and !
		KEY_CODE_2              = 31, // 2 and @
		KEY_CODE_3              = 32, // 3 and #
		KEY_CODE_4              = 33, // 4 and $
		KEY_CODE_5              = 34, // 5 and %
		KEY_CODE_6              = 35, // 6 and ^
		KEY_CODE_7              = 36, // 7 and &
		KEY_CODE_8              = 37, // 8 and *
		KEY_CODE_9              = 38, // 9 and (
		KEY_CODE_0              = 39, // 0 and )
		KEY_CODE_ENTER          = 40, // (Return)
		KEY_CODE_ESCAPE         = 41,
		KEY_CODE_DELETE         = 42,
		KEY_CODE_TAB            = 43,
		KEY_CODE_SPACE          = 44,
		KEY_CODE_MINUS          = 45, // - and (underscore)
		KEY_CODE_EQUALS         = 46, // = and +
		KEY_CODE_LEFTBRACKET    = 47, // [ and {
		KEY_CODE_RIGHTBRACKET   = 48, // ] and }
		KEY_CODE_BACKSLASH      = 49, // \ and |
		KEY_CODE_KEY_NONUSHASH  = 50, // # and ~
		KEY_CODE_SEMICOLON      = 51, // ; and :
		KEY_CODE_QUOTE          = 52, // ' and "
		KEY_CODE_GRAVE          = 53,
		KEY_CODE_COMMA          = 54, // , and <
		KEY_CODE_PERIOD         = 55, // . and >
		KEY_CODE_SLASH          = 56, // / and ?
		KEY_CODE_CAPSLOCK       = 57,
		KEY_CODE_F1             = 58,
		KEY_CODE_F2             = 59,
		KEY_CODE_F3             = 60,
		KEY_CODE_F4             = 61,
		KEY_CODE_F5             = 62,
		KEY_CODE_F6             = 63,
		KEY_CODE_F7             = 64,
		KEY_CODE_F8             = 65,
		KEY_CODE_F9             = 66,
		KEY_CODE_F10            = 67,
		KEY_CODE_F11            = 68,
		KEY_CODE_F12            = 69,
		KEY_CODE_PRINTSCREEN    = 70,
		KEY_CODE_SCROLLLOCK     = 71,
		KEY_CODE_PAUSE          = 72,
		KEY_CODE_INSERT         = 73,
		KEY_CODE_HOME           = 74,
		KEY_CODE_PAGEUP         = 75,
		KEY_CODE_DELETEFORWARD  = 76,
		KEY_CODE_END            = 77,
		KEY_CODE_PAGEDOWN       = 78,
		KEY_CODE_RIGHT          = 79, // Right arrow
		KEY_CODE_LEFT           = 80, // Left arrow
		KEY_CODE_DOWN           = 81, // Down arrow
		KEY_CODE_UP             = 82, // Up arrow
		KEY_CODE_KP_NUMLOCK     = 83,
		KEY_CODE_KP_DIVIDE      = 84,
		KEY_CODE_KP_MULTIPLY    = 85,
		KEY_CODE_KP_SUBTRACT    = 86,
		KEY_CODE_KP_ADD         = 87,
		KEY_CODE_KP_ENTER       = 88,
		KEY_CODE_KP_1           = 89,
		KEY_CODE_KP_2           = 90,
		KEY_CODE_KP_3           = 91,
		KEY_CODE_KP_4           = 92,
		KEY_CODE_KP_5           = 93,
		KEY_CODE_KP_6           = 94,
		KEY_CODE_KP_7           = 95,
		KEY_CODE_KP_8           = 96,
		KEY_CODE_KP_9           = 97,
		KEY_CODE_KP_0           = 98,
		//KEY_CODE_KP_POINT     = 99, // . and Del
		KEY_CODE_CODE_KP_EQUALS = 103,
		KEY_CODE_F13            = 104,
		KEY_CODE_F14            = 105,
		KEY_CODE_F15            = 106,
		KEY_CODE_F16            = 107,
		KEY_CODE_F17            = 108,
		KEY_CODE_F18            = 109,
		KEY_CODE_F19            = 110,
		KEY_CODE_F20            = 111,
		KEY_CODE_F21            = 112,
		KEY_CODE_F22            = 113,
		KEY_CODE_F23            = 114,
		KEY_CODE_F24            = 115,
		//KEY_CODE_KEY_HELP     = 117,
		KEY_CODE_MENU           = 118,
		KEY_CODE_MUTE           = 127,
		KEY_CODE_VOLUMEUP       = 128,
		KEY_CODE_VOLUMEDOWN     = 129,
		KEY_CODE_LEFTCONTROL    = 224, // WARNING : Android has no Ctrl keys.
		KEY_CODE_LEFTSHIFT      = 225,
		KEY_CODE_LEFTALT        = 226,
		KEY_CODE_LEFTGUI        = 227,
		KEY_CODE_RIGHTCONTROL   = 228,
		KEY_CODE_RIGHTSHIFT     = 229, // WARNING : Win32 fails to send a WM_KEYUP message if both shift keys are pressed, and one released.
		KEY_CODE_RIGHTALT       = 230,
		KEY_CODE_RIGHTGUI       = 231
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	/** Convert native Win32 keyboard scancode to cross-platform USB HID code. */
	const unsigned char WIN32_TO_HID[256] =
	{
		  0,   0,   0,   0,   0,   0,   0,   0,  42,  43,   0,   0,   0,  40,   0,   0, //  16
		225, 224, 226,  72,  57,   0,   0,   0,   0,   0,   0,  41,   0,   0,   0,   0, //  32
		 44,  75,  78,  77,  74,  80,  82,  79,  81,   0,   0,   0,  70,  73,  76,   0, //  48
		 39,  30,  31,  32,  33,  34,  35,  36,  37,  38,   0,   0,   0,   0,   0,   0, //  64
		  0,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18, //  80
		 19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,   0,   0,   0,   0,   0, //  96
		 98,  89,  90,  91,  92,  93,  94,  95,  96,  97,  85,  87,   0,  86,  99,  84, // 112
		 58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69, 104, 105, 106, 107, // 128
		108, 109, 110, 111, 112, 113, 114, 115,   0,   0,   0,   0,   0,   0,   0,   0, // 144
		 83,  71,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 160
		225, 229, 224, 228, 226, 230,   0,   0,   0,   0,   0,   0,   0, 127, 128, 129, // 176    L/R shift/ctrl/alt  mute/vol+/vol-
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  51,  46,  54,  45,  55,  56, // 192
		 53,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 208
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  47,  49,  48,  52,   0, // 224
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 240
		  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0  // 256
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	// NOTE: This values can differ from the surface size built, use CoreManager::getWidth and
	//       CoreManager::getHeight to obtain the real size of the surface built
	const int  windowWidth  = 1920; //! Width of the window to build.
	const int  windowHeight = 1080; //! Height of the window to build
	const bool fullscreen   = true; //!< Whether to display in fullscreen

	/////////////////////////////////////////////////////////////////////////////////////////////
}

#endif _COREENUM_H_
