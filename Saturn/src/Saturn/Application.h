#pragma once

#include "Core.h"

#include "Window.h"
#include "Saturn/LayerStack.h"
#include "Saturn/Events/Event.h"
#include "Saturn/Events/ApplicationEvent.h"
#include "Saturn/ImGui/ImGuiLayer.h"
#include "Saturn/Core/Timestep.h"
#include "GameBase/GameLayer.h"
#include "Core/World/Level.h"

#include <string>
#include <vector>

namespace Saturn {
	class ModuleManager;
	class Module;
	class GameLayer;
	class Level;
	class SceneManager;
}

namespace Saturn {

	class ModuleManager;
	class Module;

	class SATURN_API Application
	{
	public:
		Application();
		virtual ~Application();

		float frames;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void RenderImGui();

		Window& GetWindow() { return *m_Window; }

		Scene& GetCurrentScene() { return *m_Scene; }
		ModuleManager& GetModuleManager() { return *m_ModuleManager; }
		SceneMananger& GetSceneManangerr() { return *m_SceneManager; }
		//Ref<Saturn::ModuleManager> GetModuleManagerRef() { return m_ModuleManager; }

		static Application& Get() { return *s_Instance; }
		static bool IsRunning() { return Get().m_Running; }
		static bool GetMinimized() { return Get().m_Minimized; }

		void SetCrashState(bool state);
		bool SetRunningState(bool state);

		std::pair< std::string, std::string> OpenFile(const char* filter) const;

		std::string OpenProjectFile() const;
		std::pair< std::string, std::string> SaveFile(const char* f) const;
		std::string SaveJSONFile() const;

		GameLayer* m_gameLayer;

		GameObject* gameObject;

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void Init();

	private:

		std::unique_ptr<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;

		EditorLayer* m_EditorLayer;

		Ref<Scene> m_Scene;

		Ref<SceneMananger> m_SceneManager;
		Ref<ModuleManager> m_ModuleManager;

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