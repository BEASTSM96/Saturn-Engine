/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "Core/Base.h"

#include "Window.h"
#include "Saturn/LayerStack.h"
#include "Saturn/Events/Event.h"
#include "Saturn/Events/ApplicationEvent.h"
#include "Saturn/ImGui/ImGuiLayer.h"
#include "Saturn/Core/Timestep.h"

#include <string>
#include <vector>

namespace Saturn {
	class ModuleManager;
	class Module;
	class SceneManager;
	class HotReload;
}

namespace Saturn {

	class ModuleManager;
	class Module;

	struct ApplicationProps
	{
		std::string Name;
		uint32_t WindowWidth, WindowHeight;
	};

	class SATURN_API Application
	{
	public:
		Application( const ApplicationProps& props ={ "Saturn Engine", 1280, 720 } );
		virtual ~Application( void );

		void Run( void );

		/*----------For Editor----------*/
		virtual void OnInit( void ) { }
		virtual void OnShutdown( void ) { }
		virtual void OnShutdownSave( void ) { }
		/*------------------------------*/

		void OnEvent( Event& e );

		Layer* PushLayer( Layer* layer );
		void PushOverlay( Layer* layer );
		void RenderImGui( void );

		void Init( void );

		Window& GetWindow() { return *m_Window; }

		static Application& Get() { return *s_Instance; }
		static bool IsRunning( void ) { return Get().m_Running; }
		static bool GetMinimized( void ) { return Get().m_Minimized; }

		std::pair< std::string, std::string > OpenFile( const char* filter ) const;
		std::pair< std::string, std::string > SaveFile( const char* f ) const;

	public:
		Scene& GetCurrentScene( void ) { return *m_Scene; }
		ModuleManager& GetModuleManager( void ) { return *m_ModuleManager; }
		Ref<SceneManager>& GetSceneMananger( void ) { return m_SceneManager; }
		Ref< HotReload >& GetHotReload() { return m_HotReload; }

	private:
		bool OnWindowClose( WindowCloseEvent& e );
		bool OnWindowResize( WindowResizeEvent& e );

	private:

		std::unique_ptr< Window > m_Window;

		ImGuiLayer* m_ImGuiLayer;

		EditorLayer* m_EditorLayer;

		Ref< Scene > m_Scene;

		Ref< SceneManager > m_SceneManager;
		Ref< ModuleManager > m_ModuleManager;
		Ref< HotReload > m_HotReload;

		bool m_Running = true;

		LayerStack m_LayerStack;

		bool m_Crashed = false;

		float LastFrameTime = 0.0f;

		bool m_Minimized = false;

		Timestep m_TimeStep;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}