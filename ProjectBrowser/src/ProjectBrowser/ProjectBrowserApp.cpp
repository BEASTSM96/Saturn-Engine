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

#include "ProjectBrowserApp.h"

#include <Saturn/Renderer/Renderer.h>

#include "ProjectBrowserLayer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace ProjectBrowser {
#define BIND_EVENT_FN(x) std::bind(&ProjectBrowserApp::x, this, std::placeholders::_1)

	ProjectBrowserApp* ProjectBrowserApp::s_Instance = nullptr;

	ProjectBrowserApp::ProjectBrowserApp( Saturn::ApplicationCommandLineArgs args, const Saturn::ApplicationProps& props )
	{
		SAT_PROFILE_FUNCTION();

		SAT_CORE_ASSERT( !s_Instance, "Application already exists!" );

		s_Instance = this;
		m_Window = std::unique_ptr< Saturn::Window >( Saturn::Window::Create( Saturn::WindowProps( props.Name, props.WindowWidth, props.WindowHeight ) ) );
		m_Window->SetEventCallback( BIND_EVENT_FN( OnEvent ) );
		m_Window->SetVSync( false );

		m_ImGuiLayer = new ProjectBrowserLayer();

		Saturn::Renderer::Init();
		Saturn::Renderer::WaitAndRender();

		m_ImGuiLayer->InitImGui();

		Saturn::Renderer::Submit( [=]() { glViewport( 0, 0, props.WindowWidth, props.WindowHeight ); } );
		auto& fbs = Saturn::FramebufferPool::GetGlobal()->GetAll();
		for( auto& fb : fbs )
			fb->Resize( props.WindowWidth, props.WindowHeight );

		Init();
	}

	ProjectBrowserApp::~ProjectBrowserApp()
	{
		SAT_PROFILE_FUNCTION();

		m_Window = nullptr;
		s_Instance = nullptr;
	}

	void ProjectBrowserApp::Init( void )
	{
		SAT_PROFILE_FUNCTION();
	}

	void ProjectBrowserApp::RenderImGui( void )
	{
		SAT_PROFILE_FUNCTION();

		m_ImGuiLayer->Begin();

		ImGui::Begin( "Renderer" );
		auto& caps = Saturn::RendererAPI::GetCapabilities();
		ImGui::Text( "Vendor: %s", caps.Vendor.c_str() );
		ImGui::Text( "Renderer: %s", caps.Renderer.c_str() );
		ImGui::Text( "Version: %s", caps.Version.c_str() );
		ImGui::Text( "Frame Time: %.2fms\n", m_TimeStep.GetMilliseconds() );
		ImGui::End();

		m_ImGuiLayer->OnImGuiRender();

		//for( Saturn::Layer* layer : m_LayerStack )
		//	layer->OnImGuiRender();

		m_ImGuiLayer->End();
	}

	void ProjectBrowserApp::Run( void )
	{
		SAT_PROFILE_FUNCTION();
		while( m_Running && !m_Crashed && !m_PendingClose )
		{
			SAT_PROFILE_SCOPE( "ProjectBroswer-RunLoop" );

			if( !m_Minimized )
			{
				ProjectBrowserApp* app = this;
				Saturn::Renderer::Submit( [app]() { app->RenderImGui(); } );

				Saturn::Renderer::WaitAndRender();
			}
			m_Window->OnUpdate();

			float time = ( float )glfwGetTime(); //Platform::GetTime();

			m_TimeStep = time - LastFrameTime;

			LastFrameTime = time;
		}
		m_Window = nullptr;
		m_ImGuiLayer->Shutdown();
		m_ImGuiLayer = nullptr;
		Saturn::Renderer::Shutdown();
		glfwTerminate();
	}

	void ProjectBrowserApp::OnEvent( Saturn::Event& e )
	{
		SAT_PROFILE_FUNCTION();
		Saturn::EventDispatcher dispatcher( e );

		dispatcher.Dispatch< Saturn::WindowCloseEvent >( BIND_EVENT_FN( OnWindowClose ) );

		dispatcher.Dispatch< Saturn::WindowResizeEvent >( BIND_EVENT_FN( OnWindowResize ) );

		for( auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			( *--it )->OnEvent( e );
			if( e.Handled )
				break;
		}
	}

	Saturn::Layer* ProjectBrowserApp::PushLayer( Saturn::Layer* layer )
	{
		SAT_PROFILE_FUNCTION();

		m_LayerStack.PushLayer( layer );
		layer->OnAttach();

		return layer;
	}

	void ProjectBrowserApp::PushOverlay( Saturn::Layer* layer )
	{
		SAT_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay( layer );
		layer->OnAttach();
	}

	bool ProjectBrowserApp::OnWindowClose( Saturn::WindowCloseEvent& e )
	{
		SAT_PROFILE_FUNCTION();
		m_Running = false;
		return true;
	}

	bool ProjectBrowserApp::OnWindowResize( Saturn::WindowResizeEvent& e )
	{
		SAT_PROFILE_FUNCTION();
		int width = e.GetWidth(), height = e.GetHeight();
		if( width == 0 || height == 0 )
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Saturn::Renderer::Submit( [=]() { glViewport( 0, 0, width, height ); } );
		auto& fbs = Saturn::FramebufferPool::GetGlobal()->GetAll();
		for( auto& fb : fbs )
			fb->Resize( width, height );

		return false;
	}

}