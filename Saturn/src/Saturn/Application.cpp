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
#include "Saturn/GameFramework/HotReload.h"

#include <imgui.h>

#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Saturn {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
#pragma warning(disable: BIND_EVENT_FN)

	Application* Application::s_Instance = nullptr;

	Application::Application( const ApplicationProps& props /*= {"Saturn Engine", 1280, 720}*/ )
	{
		SAT_PROFILE_FUNCTION();

		SAT_CORE_ASSERT( !s_Instance, "Application already exists!" );

		s_Instance = this;
		m_Window = std::unique_ptr< Window >( Window::Create( WindowProps( props.Name, props.WindowWidth, props.WindowHeight ) ) );
		m_Window->SetEventCallback( BIND_EVENT_FN( OnEvent ) );
		m_Window->SetVSync( false );

		m_ImGuiLayer = new ImGuiLayer();
		m_EditorLayer = new EditorLayer();

		Renderer::Init();
		Renderer::WaitAndRender();

		//PushLayer(m_EditorLayer);
		PushOverlay( m_ImGuiLayer );
		Init();
		PushOverlay( m_EditorLayer );

		m_Window->Maximize();
	}

	Application::~Application()
	{
		SAT_PROFILE_FUNCTION();
	}

	Layer* Application::PushLayer( Layer* layer )
	{
		SAT_PROFILE_FUNCTION();

		m_LayerStack.PushLayer( layer );
		layer->OnAttach();

		return layer;
	}

	void Application::PushOverlay( Layer* layer )
	{
		SAT_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay( layer );
		layer->OnAttach();
	}

	void Application::RenderImGui()
	{
		m_ImGuiLayer->Begin();

		ImGui::Begin( "Renderer" );
		auto& caps = RendererAPI::GetCapabilities();
		ImGui::Text( "Vendor: %s", caps.Vendor.c_str() );
		ImGui::Text( "Renderer: %s", caps.Renderer.c_str() );
		ImGui::Text( "Version: %s", caps.Version.c_str() );
		ImGui::Text( "Frame Time: %.2fms\n", m_TimeStep.GetMilliseconds() );
		ImGui::End();

		for( Layer* layer : m_LayerStack )
			layer->OnImGuiRender();

		m_ImGuiLayer->End();
	}

	void Application::OnEvent( Event& e )
	{
		SAT_PROFILE_FUNCTION();

		EventDispatcher dispatcher( e );

		dispatcher.Dispatch< WindowCloseEvent >( BIND_EVENT_FN( OnWindowClose ) );

		dispatcher.Dispatch< WindowResizeEvent >( BIND_EVENT_FN( OnWindowResize ) );

		for( auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			( *--it )->OnEvent( e );
			if( e.Handled )
				break;
		}
	}

	void Application::Init()
	{
		Serialiser::Init();

		m_ModuleManager = Ref< ModuleManager >::Create();
		m_SceneManager = Ref< SceneManager >::Create();
		m_HotReload = Ref< HotReload >::Create();
		m_HotReload->m_Scece = m_EditorLayer->GetEditorScene();
	}

	void Application::Run()
	{
		SAT_PROFILE_FUNCTION();
		while( m_Running && !m_Crashed )
		{
			SAT_PROFILE_SCOPE( "RunLoop" );

			if( !m_Minimized )
			{
				for( Layer* layer : m_LayerStack )
					layer->OnUpdate( m_TimeStep );

				Application* app = this;
				Renderer::Submit( [app]() { app->RenderImGui(); } );

				Renderer::WaitAndRender();
			}
			m_Window->OnUpdate();

			float time = ( float )glfwGetTime(); //Platform::GetTime();

			m_TimeStep = time - LastFrameTime;

			LastFrameTime = time;
		}
		/* Call Editor / child functions */
		OnShutdown();
		OnShutdownSave();
	}

	bool Application::OnWindowClose( WindowCloseEvent& e )
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize( WindowResizeEvent& e )
	{
		int width = e.GetWidth(), height = e.GetHeight();
		if( width == 0 || height == 0 )
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::Submit( [=]() { glViewport( 0, 0, width, height ); } );
		auto& fbs = FramebufferPool::GetGlobal()->GetAll();
		for( auto& fb : fbs )
			fb->Resize( width, height );

		return false;
	}

	std::pair< std::string, std::string > Application::OpenFile( const char* filter ) const
	{

		SAT_PROFILE_FUNCTION();

	#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[ 260 ] = { 0 };       // if using TCHAR macros

										// Initialize OPENFILENAME
		ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = glfwGetWin32Window( ( GLFWwindow* )m_Window->GetNativeWindow() );
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if( GetOpenFileNameA( &ofn ) == TRUE )
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


	std::pair< std::string, std::string > Application::SaveFile( const char* f ) const
	{
		SAT_PROFILE_FUNCTION();

	#ifdef  SAT_PLATFORM_LINUX
	#endif
	#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[ 260 ] ={ 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = glfwGetWin32Window( ( GLFWwindow* )m_Window->GetNativeWindow() );
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		ofn.lpstrFilter = f;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if( GetSaveFileNameA( &ofn ) == TRUE )
		{
			return { ofn.lpstrFile, ofn.lpstrFilter };
		}
		return { std::string(), std::string() };
	#endif
		return { std::string(), std::string() };

	}


}