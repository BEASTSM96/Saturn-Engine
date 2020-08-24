#pragma once

#include "Core.h"

#include "Window.h"
#include "Sparky/LayerStack.h"
#include "Sparky/Events/Event.h"
#include "Sparky/Events/ApplicationEvent.h"

#include "Sparky/ImGui/ImGuiLayer.h"
#include "Sparky/Core/Timestep.h"


#include <variant>
#include <vector>


#include <filesystem>
#include <string>
#include <vector>

#include "GameBase/Player.h"

namespace Sparky {

	class SPARKY_API Application
	{
	public:
		Application();
		virtual ~Application();

		float frames;

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application& Get() { return *s_Instance; }

		void SetCrashState(bool state);
		bool SetRunningState(bool state);

		std::string OpenFile(const char* filter) const;

		std::string OpenProjectFile() const;
		std::string SaveFile() const;
		std::string SaveJSONFile() const;
	private:
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		std::unique_ptr<Window> m_Window;

		ImGuiLayer* m_ImGuiLayer;
		
		ImGuiFPS* m_FPSLayer;

		ImGuiRenderStats* m_RenderStats;

		ImguiTopBar* m_ImguiTopBar;

		EditorLayer* m_EditorLayer;

		bool m_Running = true;

		LayerStack m_LayerStack;

		bool m_Crashed = false;

		float LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}