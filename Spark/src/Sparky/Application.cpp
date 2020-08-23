#include "sppch.h"
#include "Application.h"

#include "Events/ApplicationEvent.h"

#include "Log.h"

#include "Layer.h"

#include "Sparky/Input.h"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "ImGui/ImGuiLayer.h"

#include "Sparky\Renderer\Buffer.h"

#include "Renderer/Renderer.h"

#include "Renderer/OrthographicCamera.h"

#include "Sparky/Core/File/SparkyFile.h"

#include "Sparky/ImGui/ImGuiLayer.h"

#include "Sparky/Audio/SparkyAudio.h"

#include "Sparky/Core.h"

#include <imgui.h>


#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>

#ifndef CEREAL_EXPOSE
#define CEREAL_EXPOSE
	#include <cereal/types/unordered_map.hpp>
	#include <cereal/types/memory.hpp>
	#include <cereal/archives/binary.hpp>
	#include <fstream>
	#include <cereal/archives/portable_binary.hpp>
	#include <cereal/archives/xml.hpp>
	#include <cereal/archives/json.hpp>
#endif // !CEREAL_EXPOSE

#ifndef SPARKY_EXPOSE_APPDATA
#define SPARKY_EXPOSE_APPDATA
#include "Core/AppData/SparkyAppData.h"
#endif // !SPARKY_EXPOSE_APPDATA

#ifndef SPARKY_EXPOSE_SERIALISER
#define SPARKY_EXPOSE_SERIALISER
#include "Sparky/Core/Serialisation/Serialiser.h"
#include "Sparky/Core/Serialisation/Object.h"
#endif // !SPARKY_EXPOSE_SERIALISER


namespace Sparky {
	struct MyRecord
	{
		uint8_t x, y;
		float z;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(x, y, z);
		}
	};

	struct SomeData
	{
		int32_t id;
		std::shared_ptr<std::unordered_map<uint32_t, MyRecord>> data;

		template <class Archive>
		void save(Archive& ar) const
		{
			ar(data);
		}

		template <class Archive>
		void load(Archive& ar)
		{
			static int32_t idGen = 0;
			id = idGen++;
			ar(data);
		}
	};


#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		SP_CORE_ASSERT(!s_Instance, "Application already exists!");

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
#ifdef SP_DEBUG
			m_EditorLayer = new EditorLayer();
#endif
		}

		{
			PushOverlay(m_ImGuiLayer);
			PushOverlay(m_RenderStats);

			#ifndef APP_CORE_UI
			#define APP_CORE_UI
					#ifdef SP_DEBUG
								PushOverlay(m_ImguiTopBar);
								PushOverlay(m_EditorLayer);
					#endif // SP_DEBUG
			#endif // !APP_CORE_UI
		}



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
		Audio::Init();

		while (m_Running && !m_Crashed)
		{
			float time = (float)glfwGetTime(); //Platform::GetTime();

			Timestep timestep = time - LastFrameTime;

			LastFrameTime = time;

			for (Layer* layer : m_LayerStack)
			{
				SP_CORE_ASSERT(layer, "layer in 'm_LayerStack' array null, 0 or not vaild.");
				layer->OnUpdate(timestep);
			}

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();

#ifndef SP_DEBUG
			
			std::string appd = SparkyAppData() / "enginedebug.ses";

			std::ofstream os("enginedebug.ses", std::ios::binary);
			cereal::BinaryOutputArchive archive(os);

			

			if (m_ImGuiLayer)
			{
				for (Layer* layer : m_LayerStack)
				{
					os << "All layers in the engine : " << layer << std::endl;
					os << "All layers names in the engine : " << layer->GetName() << std::endl;
				}
				os << "Window " << m_Window << std::endl;
				os << "IsVSync " << Application::GetWindow().IsVSync() << std::endl;
				os << "Window Title " << Application::GetWindow().title << std::endl;
				os << "Window Width " << Application::GetWindow().GetWidth() << std::endl;
				os << "Window Height " << Application::GetWindow().GetHeight() << std::endl;
				os << "Window NativeWindow " << Application::GetWindow().GetNativeWindow() << std::endl;
				os << "--------------------------------------------------------------------------------";
			}

#endif // SP_DEBUG
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


	std::string Application::OpenFile(const char* filter) const
	{

#ifdef  SP_PLATFORM_WINDOWS
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
			return ofn.lpstrFile;
		}
		return std::string();
#endif

#ifdef  SP_PLATFORM_LINUX
		m_Running = false;
		m_Crashed = false;

		return std::string();
#endif

		return std::string();
	}

	std::string Application::SaveFile() const
	{
		#ifdef  SP_PLATFORM_LINUX
				m_Running = false;
				m_Crashed = false;
		#endif
#ifdef  SP_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = "SparkySaveFile\0 * .sasset; \0\0";
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

	std::string Application::SaveJSONFile() const
	{
#ifdef  SP_PLATFORM_LINUX
		m_Running = false;
		m_Crashed = false;
#endif
#ifdef  SP_PLATFORM_WINDOWS
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
		#ifdef  SP_PLATFORM_LINUX
				m_Running = false;
				m_Crashed = false;

				return nullptr;
		#endif
#ifdef  SP_PLATFORM_WINDOWS
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