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

#pragma once

#include "Base.h"

#include "Layer.h"
#include "Events.h"
#include "Input.h"
#include "UserSettings.h"

#include "SingletonStorage.h"

#include <optick/optick.h>

#include <vector>
#include <functional>

namespace Saturn {

	struct ApplicationSpecification
	{
		bool CreateSceneRenderer = true;
		bool Titlebar = false;
		bool UIOnly = false;
		bool GameDist = false;
		bool EnableGameThread = false;
		
		uint32_t WindowWidth = 0;
		uint32_t WindowHeight = 0;
	};

	class SceneRenderer;
	class Application
	{
	public:
		Application( const ApplicationSpecification& spec );

		virtual ~Application() {}

		void Run();
		void Close();

		bool Running() { return m_Running; }

		Timestep& Time() { return m_Timestep; }

		std::string OpenFile( const char* pFilter ) const;
		std::string SaveFile( const char* pFilter ) const;
		std::string OpenFolder() const;

		const char* GetConfigName();

		static inline Application& Get() { return *SingletonStorage::Get().GetSingleton<Application>(); }
		ApplicationSpecification& GetSpecification() { return m_Specification; }

		void PushLayer( Layer* pLayer );
		void PopLayer( Layer* pLayer );

		virtual void OnInit() {}
		virtual void OnShutdown() {}
		
		SceneRenderer& PrimarySceneRenderer() { return *m_SceneRenderer; }

		void SubmitOnMainThread( std::function<void()>&& rrFunction ) 
		{
			if( m_Specification.EnableGameThread )
				m_MainThreadQueue.push_back( std::move( rrFunction ) );
			else
				rrFunction();
		}

		std::filesystem::path& GetRootContentDir() { return m_RootContentPath; }
		const std::filesystem::path& GetRootContentDir() const { return m_RootContentPath; }

	protected:

		void OnEvent( Event& e );
		bool OnWindowResize( WindowResizeEvent& e );

		void RenderImGui();

		std::string OpenFileInternal( const char* pFilter ) const;
		std::string SaveFileInternal( const char* pFilter ) const;
		std::string OpenFolderInternal() const;

		// The path where the default content is. Path is absolute.
		std::filesystem::path m_RootContentPath;

	private:
		bool m_Running = true;
		
		ImGuiLayer* m_ImGuiLayer = nullptr;

		Timestep m_Timestep;
		float m_LastFrameTime = 0.0f;
		
		ApplicationSpecification m_Specification;

		std::vector<Layer*> m_Layers;

		// TODO: Change to ref
		SceneRenderer* m_SceneRenderer = nullptr;

		std::vector<std::function<void()>> m_MainThreadQueue;
	};

	Application* CreateApplication( int argc, char** argv );
}