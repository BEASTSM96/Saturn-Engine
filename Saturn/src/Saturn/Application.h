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
		Application( const ApplicationProps& props = {"Saturn Engine", 1280, 720} );
		virtual ~Application( void );

		void Run( void );

		/*----------For Editor----------*/
		virtual void OnInit( void ) {}
		virtual void OnShutdown( void ) {}
		virtual void OnShutdownSave( void ) {}
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
		//Ref<Saturn::ModuleManager> GetModuleManagerRef() { return m_ModuleManager; }

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