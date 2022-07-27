/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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
	
	Application* Application::s_Instance = nullptr;
	
	void Application::Run()
	{
		SAT_CORE_ASSERT( !s_Instance, "An app was alreay created!" );

		s_Instance = this;

		Window::Get();
		VulkanContext::Get();

		VulkanContext::Get().Init();
		
		Window::Get().SetEventCallback( APP_BIND_EVENT_FN( OnEvent ) );
		
		Window::Get().RemoveBorder();
		Window::Get().Show();

		m_ImGuiLayer = new ImGuiLayer();
		m_ImGuiLayer->OnAttach();
		
		// Tell children to create what ever they need.
		OnInit();
		
		while( m_Running )
		{
			Window::Get().OnUpdate();
			Window::Get().Render();
			
			if ( !Window::Get().Minimized() )
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

			m_Timestep = time - m_LastFrameTime;

			m_LastFrameTime = time;
		}

		OnShutdown();
		
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
		
		VulkanContext::Get().Terminate();
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
					path = std::string( wstr.begin(), wstr.end() );
					
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
	}

	void Application::RenderImGui()
	{
		m_ImGuiLayer->Begin();

		for( auto& layer : m_Layers )
		{
			layer->OnUpdate( m_Timestep );
			layer->OnImGuiRender();
		}

		m_ImGuiLayer->End( Renderer::Get().ActiveCommandBuffer() );
	}

}
