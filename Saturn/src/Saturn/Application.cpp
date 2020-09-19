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

#include "Debug/memoryexttest.h"

#include "Core/Math/Math.h"

#include <imgui.h>

#include <json/json.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>

namespace Saturn {
	#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
	#pragma warning(disable: BIND_EVENT_FN)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{


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
			//PushOverlay(m_EditorLayer);
			//PushOverlay(m_ImguiTopBar);

			#define SPARKY_GAME_BASE


			#ifndef APP_CORE_UI
			#define APP_CORE_UI
					#ifdef SAT_DEBUG
								//PushOverlay(m_ImguiTopBar);
								PushOverlay(m_EditorLayer);
					#endif // SAT_DEBUG
			#endif // !APP_CORE_UI
		}
	}

	Application::~Application()
	{
		m_Level->~Level();
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
		Audio::Init();
		
		Serialiser::Init();

		Math::Init();

		m_Level = new Level();

		m_Scene = CreateRef<Scene>();

		m_SceneHierarchyPanel.SetContext(m_Scene);

		auto& e = m_Scene->CreateEntity("");
		
		gameObject = m_Scene->CreateEntityGameObjectprt("Cone");
		//SAT_TEST_MEMORY;

		//Saturn::Debuging::TestMemory2(gameObject, 100, 10);

		while (m_Running && !m_Crashed)
		{
			frames++;

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

			glEnable(GL_MULTISAMPLE);

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


	std::pair< std::string, std::string> Application::OpenFile(const char* filter) const
	{

#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		ofn.lStructSize = sizeof(OPENFILENAME);
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
		m_Running = false;
		m_Crashed = false;

		return { std::string(), std::string() };
#endif

		return  { std::string(), std::string() };
	}


	std::pair< std::string, std::string> Application::SaveFile(const char* f) const
	{
#ifdef  SAT_PLATFORM_LINUX
		m_Running = false;
		m_Crashed = false;
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

	std::string Application::SaveJSONFile() const
	{
#ifdef  SAT_PLATFORM_LINUX
		m_Running = false;
		m_Crashed = false;f
#endif
#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = "SaveJSONFile (.json / .JSON) \0 * .json; * .JSON; \0\0";
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrTitle = "Save file";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
#endif
		return std::string();

	}

	std::string Application::OpenProjectFile() const
	{
		#ifdef  SAT_PLATFORM_LINUX
				m_Running = false;
				m_Crashed = false;

				return nullptr;
		#endif
#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));

		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.lpstrFilter = "ProjectFile\0*.sproject;\0\0";
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrTitle = "Open Project file";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();

#endif
		return std::string();
	}

}