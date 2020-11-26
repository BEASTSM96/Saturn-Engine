#include "sppch.h"
#include "Application.h"
#include "Saturn/Core/Base.h"

#include "Events/ApplicationEvent.h"

#include "Log.h"
#include "Layer.h"
#include "Saturn/Input.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ImGui/ImGuiLayer.h"
#include "Renderer/Renderer.h"
#include "Saturn/ImGui/ImGuiLayer.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Saturn/Renderer/Framebuffer.h"
#include "Core/Modules/ModuleManager.h"
#include "Core/Modules/Module.h"
#include "Scene/SceneManager.h"

#include <imgui.h>

#include <json/json.h>

#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Saturn {
	#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
	#pragma warning(disable: BIND_EVENT_FN)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		SAT_PROFILE_FUNCTION();

		SAT_CORE_ASSERT(!s_Instance, "Application already exists!");

		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		m_Window->SetVSync(false);

		m_ImGuiLayer = new ImGuiLayer();
		m_EditorLayer = new EditorLayer();

		PushOverlay(m_ImGuiLayer);
		PushOverlay(m_EditorLayer);

		Renderer::Init();
		Renderer::WaitAndRender();
	}

	Application::~Application()
	{
		SAT_PROFILE_FUNCTION();
	}

	void Application::PushLayer(Layer* layer)
	{
		SAT_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		SAT_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::RenderImGui()
	{
		m_ImGuiLayer->Begin();

		ImGui::Begin("Renderer");
		auto& caps = RendererAPI::GetCapabilities();
		ImGui::Text("Vendor: %s", caps.Vendor.c_str());
		ImGui::Text("Renderer: %s", caps.Renderer.c_str());
		ImGui::Text("Version: %s", caps.Version.c_str());
		ImGui::Text("Frame Time: %.2fms\n", m_TimeStep.GetMilliseconds());
		ImGui::End();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();

		m_ImGuiLayer->End();
	}

	void Application::OnEvent(Event& e)
	{
		SAT_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Init()
	{
		Serialiser::Init();
		Math::Init();

		m_ModuleManager = Ref<ModuleManager>::Create();
		m_SceneManager = Ref<SceneManager>::Create();
	}

	void Application::Run()
	{		
		SAT_PROFILE_FUNCTION();

		Init();

		while (m_Running && !m_Crashed)
		{
			SAT_PROFILE_SCOPE("RunLoop");

			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_TimeStep);
				
				Application* app = this;
				Renderer::Submit([app]() { app->RenderImGui(); });

				Renderer::WaitAndRender();
			}
			m_Window->OnUpdate();

			float time = (float)glfwGetTime(); //Platform::GetTime();

			m_TimeStep = time - LastFrameTime;

			LastFrameTime = time;
		}
	}

	void Application::SetCrashState(bool state)
	{
		m_Crashed = state;
		m_Running = !state;
	}

	bool Application::SetRunningState(bool state)
	{
		m_Running = state;

		return state;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		int width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::Submit([=]() { glViewport(0, 0, width, height); });
		auto& fbs = FramebufferPool::GetGlobal()->GetAll();
		for (auto& fb : fbs)
			fb->Resize(width, height);

		return false;
	}

	std::pair<std::string, std::string> Application::OpenFile(const char* filter) const
	{

		SAT_PROFILE_FUNCTION();

#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));

		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.lpstrFilter = filter;
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrTitle = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return { ofn.lpstrFile,  ofn.lpstrFilter };
		}
		return { std::string(), std::string() };
#endif

#ifdef  SAT_PLATFORM_LINUX

		return { std::string(), std::string() };
#endif

		return  { std::string(), std::string() };
	}


	std::pair<std::string, std::string> Application::SaveFile(const char* f) const
	{
		SAT_PROFILE_FUNCTION();

#ifdef  SAT_PLATFORM_LINUX
#endif
#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = f;
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrTitle = "Save file";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return { ofn.lpstrFile, ofn.lpstrFilter };
		}
		return { std::string(), std::string() };
#endif
		return { std::string(), std::string() };

	}


}