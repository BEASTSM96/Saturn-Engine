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
#include "EntityScriptManager.h"

#include "GameScript.h"
#include "GameDLL.h"
#include "GameThread.h"

#include "Saturn/Scene/Entity.h"
#include "Saturn/Core/OptickProfiler.h"

namespace Saturn {

	EntityScriptManager::EntityScriptManager()
	{
		SingletonStorage::Get().AddSingleton( this );
	}

	EntityScriptManager::~EntityScriptManager()
	{
		for( auto&& [name, func] : m_ScriptFunctions )
			delete m_Scripts[ m_CurrentScene->GetId() ][ name ];
	}

	void EntityScriptManager::TransferEntities( const Ref<Scene>& rOldScene )
	{
		for( auto&& [name, script] : m_Scripts[ rOldScene->GetId() ] )
		{
			auto ScriptComponents = m_CurrentScene->GetAllEntitiesWith<ScriptComponent>();

			for( auto entity : ScriptComponents )
			{
				auto& scriptName = m_CurrentScene->GetRegistry().get<ScriptComponent>( entity ).ScriptName;
				
				if( scriptName == name ) 
				{
					Entity e{ entity, m_CurrentScene.Pointer() };

					// We can always assume the class has a base class ctor.
					m_Scripts[ m_CurrentScene->GetId() ].insert( { scriptName, CreateScript( scriptName, ( SClass* ) &e ) } );
				}
			}
		}
	}

	void EntityScriptManager::RegisterScript( const std::string& rName )
	{
		if( m_ScriptFunctions.find( rName ) != m_ScriptFunctions.end() )
			return;

		auto module = GameDLL::Get().m_DLLInstance;

		typedef SClass* ( __stdcall* func )( SClass* TBase );

		std::string funcNameTBase = "_Z_Create_" + rName + "_FromBase";

		func regfn = ( func ) GetProcAddress( module, funcNameTBase.c_str() );
		
		m_ScriptFunctions[ rName ] = regfn;
	}

	void EntityScriptManager::BeginPlay()
	{
		SAT_PF_EVENT();

		GameThread::Get().Submit( [&]() 
			{
				SAT_PF_EVENT("Saturn::EntityScriptManager::BeginPlay - Submit");

				for( auto&& [name, script] : m_Scripts[ m_CurrentScene->GetId() ] )
					script->BeginPlay();
			} );
	}

	void EntityScriptManager::UpdateAllScripts( Saturn::Timestep ts )
	{
		SAT_PF_EVENT();

		GameThread::Get().Submit( [&]()
			{
				SAT_PF_EVENT("Saturn::EntityScriptManager::UpdateAllScripts - Submit");

				for( auto&& [name, script] : m_Scripts[ m_CurrentScene->GetId() ] )
					script->OnUpdate( ts );
			} );
	}

	void EntityScriptManager::CreateAllScripts()
	{
	}

	void EntityScriptManager::DestroyEntityInScene( const Ref<Scene>& rScene )
	{
		for( auto&& [name, script] : m_Scripts[ rScene->GetId() ] )
			delete script;

		m_Scripts[ rScene->GetId() ].clear();

		m_Scripts.erase( rScene->GetId() );
	}

	Saturn::SClass* EntityScriptManager::CreateScript( const std::string& rName, SClass* Base )
	{
		return m_Scripts[ m_CurrentScene->GetId() ][ rName ] = m_ScriptFunctions[ rName ]( Base );
	}

	void EntityScriptManager::RT_AddToEditor( const std::string& rName )
	{
		m_VisibleScripts.push_back( rName );
	}

}