#include "sppch.h"
#include "Application.h"

#include "Events/ApplicationEvent.h"

#include "Log.h"

#include "Layer.h"

#include "Saturn/Input.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "ImGui/ImGuiLayer.h"

#include "Saturn/Renderer\Buffer.h"

#include "Renderer/Renderer.h"

#include "Renderer/OrthographicCamera.h"

#include "Saturn/Core/File/SparkyFile.h"

#include "Saturn/ImGui/ImGuiLayer.h"

#include "Saturn/Audio/SparkyAudio.h"

#include "Saturn/Core.h"

#include "Saturn/Core/Serialisation/Serialiser.h"

#include "Saturn/Core/Serialisation/Object.h"

#include "Scene/Components.h"
#include "Scene/Entity.h"

#include "GameBase/GameLayer.h"

#include "Core/World/Level.h"

#include <imgui.h>

#include <json/json.h>

#include <Windows.h>
#include <commdlg.h>
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

		{
			s_Instance = this;
			m_Window = std::unique_ptr<Window>(Window::Create());
			m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
			m_Window->SetVSync(false);

			Renderer::Init();
		}

		{
			m_ImGuiLayer = new ImGuiLayer();

			m_FPSLayer = new ImGuiFPS();
			m_RenderStats = new ImGuiRenderStats();
			m_ImguiTopBar = new ImguiTopBar();

//#ifdef SAT_DEBUG
			m_EditorLayer = new EditorLayer();

//#endif
		}

		{
			PushOverlay(m_ImGuiLayer);
			PushOverlay(m_RenderStats);
			PushOverlay(m_EditorLayer);
			//PushOverlay(m_ImguiTopBar);

			#define SPARKY_GAME_BASE
		}
	}

	Application::~Application()
	{
		SAT_PROFILE_FUNCTION();
		m_Level->~Level();
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

	void Application::Run()
	{		
		SAT_PROFILE_FUNCTION();

		Serialiser::Init();

		Math::Init();

		m_Level = new Level();

		m_Scene = CreateRef<Scene>();

		m_SceneHierarchyPanel.SetContext(m_Scene);

		auto& e = m_Scene->CreateEntity("");
		
		std::vector<std::string> paths;
		paths.push_back("assets/shaders/3d_test.satshaderv");
		paths.push_back("assets/shaders/3d_test.satshaderf");

		gameObject = m_Scene->CreateEntityGameObjectprt("Cone", paths);

		auto* gun = m_Scene->CreateEntityGameObjectprt("Gun", paths, "assets/meshes/m1911/m1911.fbx");

		while (m_Running && !m_Crashed)
		{
			SAT_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime(); //Platform::GetTime();

			Timestep timestep = time - LastFrameTime;

			LastFrameTime = time;


			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
				{
					SAT_CORE_ASSERT(/*!*/layer, "layer in 'm_LayerStack' array null, 0 or not vaild.");
					layer->OnUpdate(timestep);
				}
			}

			m_ImGuiLayer->Begin();
			#if defined(SAT_DEBUG)
						for (Layer* layer : m_LayerStack) {
							layer->OnImGuiRender();
						}
						m_SceneHierarchyPanel.OnImGuiRender();

			#else
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();

				m_SceneHierarchyPanel.OnImGuiRender();
			#endif 			
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
		while (m_Crashed && !m_Running)
		{
			float time = (float)glfwGetTime(); //Platform::GetTime();

			Timestep timestep = time - LastFrameTime;

			LastFrameTime = time;

			m_Window->OnUpdate();
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
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}


	std::pair<std::string, std::string> Application::OpenFile(const char* filter) const
	{

		SAT_PROFILE_FUNCTION();

#ifdef  SAT_PLATFORM_WINDOWS
		SAT_FILEOPENNAMEA ofn;
		CHAR szFile[260] = { 0 };
		SAT_ZeroMemory(&ofn, sizeof(SAT_FILEOPENNAME));

		ofn.lStructSize = sizeof(SAT_FILEOPENNAME);
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

		SAT_ZeroMemory(&ofn, sizeof(OPENFILENAME));
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