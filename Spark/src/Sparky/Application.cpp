#include "sppch.h"

#include "Application.h"

#include "Events/ApplicationEvent.h"

#include "Log.h"

#include "Layer.h"

#include "Sparky/Input.h"

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <GLFW/glfw3.h>

#include "ImGui/ImGuiLayer.h"

#include "Sparky\Renderer\Buffer.h"

#include "Renderer/Renderer.h"

#include "Renderer/OrthographicCamera.h"

#include "Sparky/Core/File/SparkyFile.h"

#include "Sparky/ImGui/ImGuiLayer.h"

//TEMP

#include "imgui.h"

namespace Sparky {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		SP_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(false);

		m_ImGuiLayer = new ImGuiLayer();
		m_FPSLayer = new ImGuiFPS();
		m_RenderStats = new ImGuiRenderStats();


		PushOverlay(m_ImGuiLayer);
		//PushOverlay(m_FPSLayer);
		PushOverlay(m_RenderStats);
	}

	Application::~Application()
	{
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			/*
			//1, 1, 0, 0 // yellow
			glClearColor(1, 1, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			m_BlueShader->Bind();
			m_SquareVA->Bind();
			glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			m_Shader->Bind();
			m_VertexArray->Bind();
			glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
			*/

			float time = (float)glfwGetTime(); //Platform::GetTime();

			Timestep timestep = time - LastFrameTime;

			LastFrameTime = time;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(timestep);

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();


			bool bIsFileOpened = false;
			bool bWasFileOpened = false;
			if (!bIsFileOpened && !bWasFileOpened)
			{
				bWasFileOpened = true;


				FileCreation::NewProjectFile();

				bIsFileOpened = true;


				//FileReader::ReadFile("good3dgame.sproject", "testReader");

			}
		}
		while (m_Crashed)
		{
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->OnUpdate();
		}
	}

	void Application::SetCrashState(bool state)
	{
		//m_Window = nullptr;

		//m_Window = std::unique_ptr<Window>(Window::Create());
		//m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_Crashed = state;
		m_Running = !state;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}