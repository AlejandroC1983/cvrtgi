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
#include "../../include/core/eventsignalslot.h"
#include "../../include/util/containerutilities.h"

// NAMESPACE

// DEFINES

// STATIC MEMBER INITIALIZATION

/////////////////////////////////////////////////////////////////////////////////////////////

EventSignalSlot::~EventSignalSlot()
{
	deleteMapped(m_mapKeyDown);
	deleteMapped(m_mapKeyUp);
}

/////////////////////////////////////////////////////////////////////////////////////////////

SignalVoid* EventSignalSlot::refKeyDownSignalByKey(KeyCode key)
{
	return refKeyUpSignalByKey(m_mapKeyDown, key);
}

/////////////////////////////////////////////////////////////////////////////////////////////

SignalVoid* EventSignalSlot::refKeyUpSignalByKey(KeyCode key)
{
	return refKeyUpSignalByKey(m_mapKeyUp, key);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EventSignalSlot::addKeyDownSignal(KeyCode key)
{
	addKeySignal(m_mapKeyDown, key);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EventSignalSlot::addKeyUpSignal(KeyCode key)
{
	addKeySignal(m_mapKeyUp, key);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EventSignalSlot::emitSignalForDownKey(KeyCode key)
{
	emitKeySignal(m_mapKeyDown, key);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EventSignalSlot::emitSignalForUpKey(KeyCode key)
{
	emitKeySignal(m_mapKeyUp, key);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool EventSignalSlot::isKeyDownRegistered(KeyCode key)
{
	return (m_mapKeyDown.find(static_cast<int>(key)) != m_mapKeyDown.end());
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool EventSignalSlot::isKeyUpRegistered(KeyCode key)
{
	return (m_mapKeyUp.find(static_cast<int>(key)) != m_mapKeyUp.end());
}

/////////////////////////////////////////////////////////////////////////////////////////////

SignalVoid* EventSignalSlot::refKeyUpSignalByKey(map<int, SignalVoid*>& mapKey, KeyCode key)
{
	map<int, SignalVoid*>::iterator it = mapKey.find(static_cast<int>(key));

	if (it != mapKey.end())
	{
		return it->second;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EventSignalSlot::addKeySignal(map<int, SignalVoid*>& mapKey, KeyCode key)
{
	map<int, SignalVoid*>::iterator it = mapKey.find(static_cast<int>(key));

	if (it != mapKey.end())
	{
		cout << "ERROR: trying to add again a key value " << static_cast<int>(key) << " in EventSignalSlot::addKeySignal" << endl;
	}

	SignalVoid* signalVoidToInsert = new SignalVoid;
	mapKey.insert(pair<int, SignalVoid*>(static_cast<int>(key), signalVoidToInsert));
}

/////////////////////////////////////////////////////////////////////////////////////////////

void EventSignalSlot::emitKeySignal(map<int, SignalVoid*>& mapKey, KeyCode key)
{
	map<int, SignalVoid*>::iterator it = mapKey.find(static_cast<int>(key));

	if (it == mapKey.end())
	{
		//cout << "ERROR: trying to emit signal for non present slot in EventSignalSlot::emitKeySignal" << endl;
		return;
	}

	it->second->emit();
}

/////////////////////////////////////////////////////////////////////////////////////////////
