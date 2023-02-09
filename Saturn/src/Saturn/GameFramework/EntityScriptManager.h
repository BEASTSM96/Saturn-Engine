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

#pragma once

#include "GameScript.h"
#include <unordered_map>

namespace Saturn {
	
	class Entity;

	class EntityScriptManager
	{
	public:
		static EntityScriptManager& Get() { return *SingletonStorage::Get().GetOrCreateSingleton<EntityScriptManager>(); }
	public:
		EntityScriptManager();
		~EntityScriptManager();

		void SetCurrentScene( const Ref<Scene>& rScene ) { m_CurrentScene = rScene; }
		void TransferEntities( const Ref<Scene>& rOldScene );

		void RegisterScript( const std::string& rName );

		void BeginPlay();
		void UpdateAllScripts( Saturn::Timestep ts );
		void CreateAllScripts();

		void DestroyEntityInScene( const Ref<Scene>& rScene );

		Saturn::SClass* CreateScript( const std::string& rName, SClass* Base );

		void RT_AddToEditor( const std::string& rName );

		std::vector<std::string>& GetVisibleScripts() { return m_VisibleScripts; }
		const std::vector<std::string>& GetVisibleScripts() const { return m_VisibleScripts; }

	private:

		// The register function defined in the game dll. i.e. _Z_Create_MyClass
		std::unordered_map< std::string, SClass* ( __stdcall* )( SClass* ) > m_ScriptFunctions;

		std::unordered_map< UUID, std::unordered_map< std::string, SClass* >> m_Scripts;
		std::vector< Ref<Scene> > m_Scenes;

		std::vector< std::string > m_VisibleScripts;

		Ref<Scene> m_CurrentScene;

	private:
		static EntityScriptManager* s_Instance;
	};
}