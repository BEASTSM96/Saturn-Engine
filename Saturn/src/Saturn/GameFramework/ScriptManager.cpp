/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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
#include "ScriptManager.h"

#include "GameScript.h"
#include "GameDLL.h"
#include "Saturn/Scene/Entity.h"

namespace Saturn {

	ScriptManager* ScriptManager::s_Instance;

	ScriptManager::ScriptManager()
	{
		SAT_CORE_ASSERT( !s_Instance, "Script manager already created!" )

		s_Instance = this;
	}

	ScriptManager::~ScriptManager()
	{
		for( auto&& [name, func] : m_ScriptFunctions )
			delete m_Scripts[ name ];
	}

	void ScriptManager::RegisterScript( const std::string& rName )
	{
		if( m_ScriptFunctions.find( rName ) != m_ScriptFunctions.end() )
			return;

		auto module = GameDLL::Get().m_DLLInstance;

		typedef SClass* ( __stdcall* funcptr )();

		std::string funcName = "CreateScriptClass" + rName + "_";

		funcptr regfn = ( funcptr ) GetProcAddress( module, funcName.c_str() );
		
		m_ScriptFunctions[ rName ] = regfn;
	}

	void ScriptManager::BeginPlay()
	{
		for( auto&& [name, script] : m_Scripts )
			script->BeginPlay();
	}

	void ScriptManager::UpdateAllScripts( Saturn::Timestep ts )
	{
		for( auto&& [name, script] : m_Scripts )
			script->OnUpdate( ts );
	}

	void ScriptManager::CreateAllScripts()
	{
		for( auto&& [ name, func ] : m_ScriptFunctions )
			m_Scripts[ name ] = func();
	}

	Saturn::SClass* ScriptManager::CreateScript( const std::string& rName )
	{
		SAT_CORE_INFO( "Created a new script: {0}", rName );

		return m_Scripts[ rName ] = m_ScriptFunctions[ rName ]();
	}

	void ScriptManager::SetScriptOwner( const std::string& rName, Entity* rOwner )
	{
		Entity* e = ( Entity* ) m_Scripts[ rName ];
		e->__create_entity( rOwner );
	}
}