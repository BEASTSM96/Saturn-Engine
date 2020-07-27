#pragma once

#include "Core.h"

#include "Window.h"
#include "Sparky/LayerStack.h"
#include "Sparky/Events/Event.h"
#include "Sparky/Events/ApplicationEvent.h"

#include "Sparky/ImGui/ImGuiLayer.h"

#include "Sparky/Renderer/Shader.h"
#include "Sparky/Renderer/Buffer.h"

#include "Sparky\Renderer\VertexArray.h"

namespace Sparky {

	class SPARKY_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;

		std::shared_ptr<Shader> m_BlueShader;
		std::shared_ptr<VertexArray> m_SquareVA;
	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}