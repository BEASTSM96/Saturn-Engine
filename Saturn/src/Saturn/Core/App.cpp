/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "App.h"

#include "Window.h"

#include "Saturn/Vulkan/SceneRenderer.h"
#include "Saturn/Vulkan/VulkanContext.h"

#include "OptickProfiler.h"

#include "Saturn/GameFramework/GameThread.h"
#include "Renderer/RenderThread.h"

#include "Saturn/Audio/AudioSystem.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if defined( SAT_WINDOWS )
#include <ShObjIdl.h>
#endif

#define APP_BIND_EVENT_FN(_) std::bind(&Application::_, this, std::placeholders::_1)

namespace Saturn {
	
	bool OnOptickStateChanged( Optick::State::Type state );

	Application::Application( const ApplicationSpecification& spec )
		: m_Specification( spec )
	{
		OPTICK_SET_STATE_CHANGED_CALLBACK( OnOptickStateChanged );

		SingletonStorage::Get().AddSingleton( this );

		// This may not be the best way... but its better than lazy loading.
		Window* pWindow = new Window();
		VulkanContext* pVkContext = new VulkanContext();

		pVkContext->Init();

		m_SceneRenderer = new SceneRenderer();

		pWindow->SetEventCallback( APP_BIND_EVENT_FN( OnEvent ) );

		pWindow->RemoveBorder();
		pWindow->Show();

		if( m_Specification.WindowWidth != 0 && m_Specification.WindowHeight != 0 )
			pWindow->Resize( m_Specification.WindowWidth, m_Specification.WindowHeight );

		// Lazy load.
		AudioSystem::Get();
		RenderThread::Get().Enable( m_Specification.EnableGameThread );

		m_ImGuiLayer = new ImGuiLayer();
		m_ImGuiLayer->OnAttach();
	}

	void Application::Run()
	{
		// Tell children to create what ever they need.
		OnInit();

		while( m_Running )
		{
			SAT_PF_FRAME("Master Thread");

			Window::Get().Update();
		
			for( auto&& fn : m_MainThreadQueue )
				fn();

			m_MainThreadQueue.clear();

			// Execute render thread (last frame).
			RenderThread::Get().WaitAll();

			Window::Get().OnUpdate();

			if( !Window::Get().Minimized() )
			{
				Renderer::Get().BeginFrame();
				{
					// Render Scene on render thread.
					RenderThread::Get().Queue( [=] { m_SceneRenderer->RenderScene(); } );
					
					// Render UI
					{
						RenderImGui();
					}
				}
				// End this frame on render thread.
				RenderThread::Get().Queue( [=] { Renderer::Get().EndFrame(); } );
			}

			float time = ( float ) glfwGetTime();

			float frametime = time - m_LastFrameTime;

			m_Timestep = std::min<float>( frametime, 0.0333f );
			
			m_LastFrameTime = time;
		}

		OnShutdown();
		
		GameThread::Get().Terminate();
		RenderThread::Get().Terminate();

		VulkanContext::Get().SubmitTerminateResource( [&]() 
		{
			for ( auto& layer : m_Layers )
			{
				delete layer;
			}

			m_ImGuiLayer->OnDetach();
			
			delete m_ImGuiLayer;

			m_ImGuiLayer = nullptr;
		} );

		AudioSystem::Get().Terminate();

		OPTICK_SHUTDOWN();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::RenderImGui()
	{
		SAT_PF_EVENT();

		// Begin on main thread.
		m_ImGuiLayer->Begin();

		// Update on the main thread.
		for( auto& layer : m_Layers )
		{
			layer->OnUpdate( m_Timestep );
		}

		// I'm not really sure if I want the render thread to render imgui.
		// TEMP: There is some bugs when we try to render imgui on renderer thread, and if it needs a new window it will freeze
		RenderThread::Get().Queue( [=]
			{
				for( auto& layer : m_Layers )
				{
					layer->OnImGuiRender();
				}
			} );

		RenderThread::Get().Queue( [=]
			{
				m_ImGuiLayer->End( Renderer::Get().ActiveCommandBuffer() );
			} );
	}

	std::string Application::OpenFile( const char* pFilter ) const
	{
		return OpenFileInternal( pFilter );
	}

	std::string Application::SaveFile( const char* f ) const
	{
		return SaveFileInternal( f );
	}

	std::string Application::OpenFolder() const
	{
		return OpenFolderInternal();
	}

	const char* Application::GetConfigName()
	{
#if defined(SAT_DEBUG)
		return "Debug";
#elif defined(SAT_RELEASE)
		return "Release";
#elif defined(SAT_DIST)
		return "Dist";
#endif
		return "Unknown";
	}

	void Application::PushLayer( Layer* pLayer )
	{
		m_Layers.push_back( pLayer );
		pLayer->OnAttach();
	}

	void Application::PopLayer( Layer* pLayer )
	{
		// Find the layer in the layer stack.
		auto it = std::find( m_Layers.begin(), m_Layers.end(), pLayer );
		if( it != m_Layers.end() )
		{
			m_Layers.erase( it );
			pLayer->OnDetach();
		}
	}

	void Application::OnEvent( Event& e )
	{
		EventDispatcher dispatcher( e );

		dispatcher.Dispatch< WindowResizeEvent >( APP_BIND_EVENT_FN( OnWindowResize ) );

		VulkanContext::Get().OnEvent( e );

		if( m_ImGuiLayer != nullptr )
			m_ImGuiLayer->OnEvent( e );
		
		// We need to make sure we process event backwards as if the editor layer is first and we have a button that was created by the user in game then the editor layer would get the event first, and let's say that it might shoot or something. We wanted to click a button not shoot.
		for( auto itr = m_Layers.end(); itr != m_Layers.begin(); )
		{
			( *--itr )->OnEvent( e );
			if( e.Handled )
				break;
		}
	}

	bool Application::OnWindowResize( WindowResizeEvent& e )
	{
		int width = e.Width(), height = e.Height();
		
		if( width == 0 && height == 0 )
			return false;
		
		VulkanContext::Get().ResizeEvent();

		return true;
	}

	std::string Application::OpenFileInternal( const char* pFilter ) const
	{
#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[ 260 ] = { 0 };

		ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = glfwGetWin32Window( ( GLFWwindow* ) Window::Get().NativeWindow() );
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		ofn.lpstrFilter = pFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if( GetOpenFileNameA( &ofn ) == TRUE )
		{
			return ofn.lpstrFile;
		}

		return "";
#endif

#ifdef  SAT_PLATFORM_LINUX
		return "";
#endif
	}

	std::string Application::SaveFileInternal( const char* pFilter ) const
	{
#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;
		CHAR szFile[ 260 ] = { 0 };

		ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = glfwGetWin32Window( ( GLFWwindow* ) Window::Get().NativeWindow() );
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		ofn.lpstrFilter = pFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if( GetSaveFileNameA( &ofn ) == TRUE )
		{
			return ofn.lpstrFile;
		}

		return "";
#endif

#ifdef  SAT_PLATFORM_LINUX
		return "";
#endif
	}

	std::string Application::OpenFolderInternal() const
	{
#ifdef  SAT_PLATFORM_WINDOWS
		IFileOpenDialog* pFileOpen;
		PWSTR pszFilePath;
		std::string path;

		CoInitialize( nullptr );

		// Create the object.
		HRESULT hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, ( void** ) &pFileOpen );

		if( SUCCEEDED( hr ) )
		{
			DWORD dwOptions;
			pFileOpen->GetOptions( &dwOptions );
			pFileOpen->SetOptions( dwOptions | FOS_PICKFOLDERS );

			// Show the dialog.
			hr = pFileOpen->Show( NULL );

			// Get the file name from the dialog.
			if( SUCCEEDED( hr ) )
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult( &pItem );

				if( SUCCEEDED( hr ) )
				{
					hr = pItem->GetDisplayName( SIGDN_DESKTOPABSOLUTEPARSING, &pszFilePath );

					auto wstr = std::wstring( pszFilePath );

					std::transform( wstr.begin(), wstr.end(), std::back_inserter( path ), []( wchar_t c )
						{
							return ( char ) c;
						} );

					CoTaskMemFree( pszFilePath );
				}

				pItem->Release();
			}
			else
			{
				SAT_CORE_ASSERT( false );
			}

			pFileOpen->Release();
			pFileOpen = NULL;
		}
		else
		{
			SAT_CORE_ASSERT( false );
		}

		return path;
#endif

		return "";
	}

	bool OnOptickStateChanged( Optick::State::Type state )
	{
		switch( state )
		{
			case Optick::State::DUMP_CAPTURE: 
			{
				Optick::AttachSummary( "Version", "0.0.1" );
				Optick::AttachSummary( "Build", __DATE__ " " __TIME__ );

				for( const auto& devices : VulkanContext::Get().GetPhysicalDeviceProperties() )
				{
					Optick::AttachSummary( "Device Name", devices.DeviceProps.deviceName );
					Optick::AttachSummary( "Vulkan Version", "1.2.128" );
				}

				Optick::AttachSummary( "Configuration", Application::Get().GetConfigName() );
			} break;

			case Optick::State::START_CAPTURE:
			case Optick::State::STOP_CAPTURE:
			case Optick::State::CANCEL_CAPTURE:
			default:
				break;
		}

		return true;
	}

}
