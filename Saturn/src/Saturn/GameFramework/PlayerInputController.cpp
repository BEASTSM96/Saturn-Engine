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

#include "Saturn/Project/Project.h"

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

		if( !m_ActionMap.size() )
			return;

		const std::unordered_set<RubyKey>& windowKeys = Application::Get().GetWindow()->GetCurrentKeys();

		std::unordered_map<std::string, ActionBinding> EventsToFire;

		// What current keys are down in the window and try to map that to an action binding.
		for( RubyKey key : windowKeys ) 
		{
			// Try to add the key to our local key map.
			auto [it, inserted] = m_Keys.insert( key );

			if( inserted )
			{
				// Try to map the key to a binding.
				auto bindingItr = std::find_if( m_ActionMap.begin(), m_ActionMap.end(), 
					[key]( const auto& a ) 
					{ 
						auto&& [k, v] = a;

						return v.Key == key; 
					} );

				// Did we find a binding?
				if( bindingItr != m_ActionMap.end() )
				{
					auto&& [name, binding] = ( *bindingItr );

					binding.State = true;
					EventsToFire[ name ] = binding;
				}
			}
		}

		// Now, check if our local copy contains keys that are no longer being pressed.
		for( auto it = m_Keys.begin(); it != m_Keys.end(); ) 
		{
			auto key = *( it );

			if( windowKeys.find( key ) == windowKeys.end() )
			{
				// Try find a binding that matches that key.
				auto bindingItr = std::find_if( m_ActionMap.begin(), m_ActionMap.end(),
					[key]( const auto& a )
					{
						auto&& [k, v] = a;

						return v.Key == key;
					} );

				if( bindingItr != m_ActionMap.end() )
				{
					auto&& [name, binding] = ( *bindingItr );

					EventsToFire[ name ].State = false;
				}	

				it = m_Keys.erase( it );
			}
			else
			{
				++it;
			}
		}
		
		// Trigger events.
		for( const auto& [name, binding] : EventsToFire )
		{
			if( binding.Function )
				binding.Function();
		}
	}

	void PlayerInputController::BindAction( const std::string& rBindingName, const ActionFunction& rFunction )
	{
		Ref<Project> project = Project::GetActiveProject();
		
		// Check if the binding exists in the project
		auto it = std::find_if( project->ActionBindings.begin(), project->ActionBindings.end(),
			[rBindingName]( auto& binding )
			{
				return binding.Name == rBindingName;
			} );

		if( it != project->ActionBindings.end() )
		{
			auto& rBinding = *( it );
			
			m_ActionMap[ rBindingName ] = rBinding;
			m_ActionMap[ rBindingName ].Function = rFunction;
		}
	}

	void PlayerInputController::RemoveAction( const std::string& rBindingName )
	{
		m_ActionMap.erase( rBindingName );
	}

}