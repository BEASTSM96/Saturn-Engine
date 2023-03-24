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

#include "Saturn/Vulkan/VulkanContext.h"
#include "Saturn/Vulkan/SceneRenderer.h"

#include "OptickProfiler.h"

#include "Saturn/GameFramework/GameThread.h"

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

		pWindow->SetEventCallback( APP_BIND_EVENT_FN( OnEvent ) );

		pWindow->RemoveBorder();
		pWindow->Show();

		if( m_Specification.WindowWidth != 0 && m_Specification.WindowHeight != 0 )
			pWindow->Resize( m_Specification.WindowWidth, m_Specification.WindowHeight );

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

			Window::Get().OnUpdate();
			Window::Get().Render();
		
			if( !Window::Get().Minimized() )
			{
				Renderer::Get().BeginFrame();
				{
					// Try to render scene.
					SceneRenderer::Get().RenderScene();

					// Render UI
					// TODO: Make new threads.
					//       One for rendering and one for UI.
					{
						RenderImGui();
					}
				}
				Renderer::Get().EndFrame();
			}

			float time = ( float ) glfwGetTime();

			float frametime = time - m_LastFrameTime;

			m_Timestep = std::min<float>( frametime, 0.0333f );
			
			m_LastFrameTime = time;
		}

		OnShutdown();
		
		GameThread::Get().Terminate();

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

		OPTICK_SHUTDOWN();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	std::string Application::OpenFile( const char* pFilter ) const
	{
	#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[ 260 ] ={ 0 };       // if using TCHAR macros

										// Initialize OPENFILENAME
		ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = glfwGetWin32Window( ( GLFWwindow* )Window::Get().NativeWindow() );
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		ofn.lpstrFilter = pFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if( GetOpenFileNameA( &ofn ) == TRUE )
		{
			return ofn.lpstrFile;
		}
		return std::string();
	#endif

	#ifdef  SAT_PLATFORM_LINUX
		return std::string();
	#endif

		return std::string();
	}

	std::string Application::SaveFile( const char* f ) const
	{
#ifdef  SAT_PLATFORM_WINDOWS
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[ 260 ] = { 0 };       // if using TCHAR macros

										// Initialize OPENFILENAME
		ZeroMemory( &ofn, sizeof( OPENFILENAME ) );
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = glfwGetWin32Window( ( GLFWwindow* ) Window::Get().NativeWindow() );
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof( szFile );
		ofn.lpstrFilter = f;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if( GetSaveFileNameA( &ofn ) == TRUE )
		{
			return ofn.lpstrFile;
		}
		return std::string();
#endif

#ifdef  SAT_PLATFORM_LINUX
		return std::string();
#endif

		return std::string();
	}

	std::string Application::OpenFolder() const
	{
#ifdef  SAT_PLATFORM_WINDOWS
		IFileOpenDialog* pFileOpen;
		PWSTR pszFilePath;
		std::string path;
		
		CoInitialize( nullptr );

		// Create the object.
		HRESULT hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&pFileOpen );

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
		
		//std::replace( path.begin(), path.end(), '\\', '/' );

		return path;
#endif

		return std::string();
	}

	const char* Application::GetPlatformName()
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
		
		for( auto& layer : m_Layers )
		{
			layer->OnEvent( e );
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

	void Application::RenderImGui()
	{
		SAT_PF_EVENT();

		m_ImGuiLayer->Begin();

		for( auto& layer : m_Layers )
		{
			layer->OnUpdate( m_Timestep );
			layer->OnImGuiRender();
		}

		m_ImGuiLayer->End( Renderer::Get().ActiveCommandBuffer() );
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

				Optick::AttachSummary( "Configuration", Application::Get().GetPlatformName() );
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
