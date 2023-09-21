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
#include "RuntimeLayer.h"

#include "Saturn/Project/Project.h"

#include "Saturn/Serialisation/SceneSerialiser.h"
#include "Saturn/Serialisation/ProjectSerialiser.h"
#include "Saturn/Serialisation/UserSettingsSerialiser.h"
#include "Saturn/Serialisation/AssetRegistrySerialiser.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

#include "Saturn/GameFramework/GameDLL.h"
#include "Saturn/GameFramework/GameManager.h"
#include "Saturn/GameFramework/EntityScriptManager.h"

#include "Saturn/Vulkan/SceneRenderer.h"

#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Asset/Prefab.h"

#include "Saturn/Physics/PhysicsFoundation.h"

namespace Saturn {

	RuntimeLayer::RuntimeLayer()
		: m_RuntimeScene( Ref<Scene>::Create() )
	{
		Scene::SetActiveScene( m_RuntimeScene.Pointer() );

		// Init Physics
		PhysicsFoundation* pPhysicsFoundation = new PhysicsFoundation();
		pPhysicsFoundation->Init();

		auto& rUserSettings = GetUserSettings();

		ProjectSerialiser ps;
		ps.Deserialise( rUserSettings.FullStartupProjPath.string() );

		SAT_CORE_ASSERT( Project::GetActiveProject(), "No project was given." );

		AssetManager* pAssetManager = new AssetManager();

		Project::GetActiveProject()->CheckMissingAssetRefs();

		EntityScriptManager::Get();

		GameDLL* pGameDLL = new GameDLL();
		pGameDLL->Load();

		GameManager* pGameManager = new GameManager();

		OpenFile( Project::GetActiveProject()->GetConfig().StartupScenePath );

		m_RuntimeScene->OnRuntimeStart();
		m_RuntimeScene->m_RuntimeRunning = true;
	}

	RuntimeLayer::~RuntimeLayer()
	{
		m_RuntimeScene->OnRuntimeEnd();

		EntityScriptManager::Get().DestroyEntityInScene( m_RuntimeScene );
		EntityScriptManager::Get().SetCurrentScene( nullptr );

		m_RuntimeScene = nullptr;
	}

	void RuntimeLayer::OpenFile( const std::filesystem::path& rFilepath )
	{
		Ref<Scene> newScene = Ref<Scene>::Create();
		EntityScriptManager::Get().SetCurrentScene( newScene );

		auto fullPath = Project::GetActiveProject()->FilepathAbs( rFilepath );
		SceneSerialiser serialiser( newScene );
		serialiser.Deserialise( fullPath.string() );

		m_RuntimeScene = nullptr;
		m_RuntimeScene = newScene;

		EntityScriptManager::Get().SetCurrentScene( m_RuntimeScene );

		newScene = nullptr;

		Application::Get().PrimarySceneRenderer().SetCurrentScene( m_RuntimeScene.Pointer() );
	}

	void RuntimeLayer::OnUpdate( Timestep time )
	{
		m_RuntimeScene->OnUpdate( time );
		m_RuntimeScene->OnRenderRuntime( time, Application::Get().PrimarySceneRenderer() );
	}

	void RuntimeLayer::OnImGuiRender()
	{
	}

	void RuntimeLayer::OnEvent( Event& rEvent )
	{
		EventDispatcher dispatcher( rEvent );
		dispatcher.Dispatch< WindowResizeEvent >( SAT_BIND_EVENT_FN( OnWindowResize ) );
	}

	bool RuntimeLayer::OnWindowResize( WindowResizeEvent& e )
	{
		int width = e.Width(), height = e.Height();

		if( width == 0 && height == 0 )
			return false;

		Application::Get().PrimarySceneRenderer().SetViewportSize( ( uint32_t ) width, ( uint32_t ) height );

		return true;
	}
}