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
#include "GameModule.h"

#include "Saturn/Core/App.h"
#include "Saturn/Scene/Entity.h"

#include "Saturn/Project/Project.h"

#include "GameScript.h"

#include "SourceManager.h"

namespace Saturn {

	GameModule::GameModule()
	{
		SingletonStorage::Get().AddSingleton( this );

		Load( false );
	}

	GameModule::~GameModule()
	{
		Unload();
	}

	Entity* GameModule::CreateEntity( const std::string& rClassName )
	{
		std::string funcName = "_Z_Create_" + rClassName;

		CreateSClassFn createFunc = ( CreateSClassFn ) m_GameModule->GetOrFindFunction<CreateSClassFn>( funcName );

		if( createFunc )
			return ( createFunc ) ( );
		else
			return nullptr;
	}

	void GameModule::Load( bool reload /*=false*/ )
	{
		if( !Application::Get().HasFlag( ApplicationFlags::GameDist ) )
		{
			// We are the editor, load DLL.
			auto binDir = Project::GetActiveProject()->GetBinDir();
			auto& DllPath = binDir /= Project::GetActiveConfig().Name + ".dll";

			m_GameModule = Ref<Module>::Create( DllPath, Project::GetActiveConfig().Name );
			m_GameModule->Load();

			// Call the init function.
			InitModuleFn initModFn = ( InitModuleFn )m_GameModule->m_Library.GetSymbol( "InitializeModule" );
			( initModFn ) ( Project::GetActiveProject().Get() );
		}
		else 
		{
			// We are the game so there is no need to load the dll all we need to do is set the handle to ourself.
			m_GameModule = Ref<Module>::Create( "", Project::GetActiveConfig().Name );
			m_GameModule->m_Library.SetExisting( GetModuleHandle( nullptr ) );
		}

		SourceManager::Get();
	}

	void GameModule::Unload()
	{
		if( !Application::Get().HasFlag( ApplicationFlags::GameDist ) )
		{
			m_GameModule = nullptr;
		}
	}

	void GameModule::Reload() 
	{
		Unload();
		Load(true);
	}
}