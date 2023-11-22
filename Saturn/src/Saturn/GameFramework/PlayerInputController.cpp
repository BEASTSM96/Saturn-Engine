/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include "sppch.h"
#include "PlayerInputController.h"

#include "Saturn/Core/OptickProfiler.h"

#include "Saturn/Core/App.h"
#include <Ruby/RubyWindow.h>

namespace Saturn {

	PlayerInputController::PlayerInputController()
	{
	}

	PlayerInputController::~PlayerInputController()
	{
		m_ActionMap.clear();
	}

	void PlayerInputController::Update()
	{
		SAT_PF_EVENT();

		const std::unordered_set<RubyKey>& windowKeys = Application::Get().GetWindow()->GetCurrentKeys();

		std::unordered_map<RubyKey, std::unordered_map<bool, ActionFunction>> EventsToFire;

		// First, check if any keys from the window are in our copy.
		for( const RubyKey& key : windowKeys )
		{
			auto [it, inserted] = m_Keys.insert( key );
			if( inserted )
			{
				auto map = m_ActionMap.find( key );

				if( map != m_ActionMap.end() )
					EventsToFire[ key ][ true ] = map->second;
			}
			else 
			{
				// Key was not added for some reason. Maybe it's already in the map, most likely the key is being held.
				// This will crash the engine if the function does is not present but that is rare as the key should always be inserted unless the key is already present which means there the key is being held.

				if( m_ActionMap.find( key ) != m_ActionMap.end() )
					EventsToFire[ key ][ true ] = m_ActionMap.at( key );
			}
		}

		// Now, check if our copy contains keys that are no longer being pressed.
		for( auto it = m_Keys.begin(); it != m_Keys.end(); ) 
		{
			const auto& key = *( it );

			if ( windowKeys.find( key ) == windowKeys.end() )
			{
				auto map = m_ActionMap.find( key );

				if( map != m_ActionMap.end() )
					EventsToFire[ key ][ false ] = map->second;

				it = m_Keys.erase( it );
			}
			else
			{
				++it;
			}
		}
		
		// Trigger events.
		for( const auto& [key, valueMap] : EventsToFire )
		{
			for ( const auto& [ pressed, event ] : valueMap )
			{
				( event ) ( pressed );
			}
		}
	}

	void PlayerInputController::BindAction( RubyKey key, const ActionFunction& rFunction )
	{
		m_ActionMap[ key ] = rFunction;
	}

	void PlayerInputController::RemoveAction( RubyKey key )
	{
		m_ActionMap.erase( key );
	}

}