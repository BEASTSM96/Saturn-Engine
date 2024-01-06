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

#include "Ruby/RubyWindow.h"
#include "Ruby/RubyMonitor.h"

#include "Saturn/Vulkan/SceneRenderer.h"
#include "Saturn/Vulkan/Renderer2D.h"
#include "Saturn/Vulkan/VulkanContext.h"

#include "Saturn/GameFramework/Core/GameThread.h"
#include "Renderer/RenderThread.h"

#include "Saturn/Audio/AudioSystem.h"

#include "Saturn/Asset/AssetManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "OptickProfiler.h"

#if defined( SAT_WINDOWS )
#include <ShObjIdl.h>
#include <ShlObj.h>
#endif

#define APP_BIND_EVENT_FN(_) std::bind(&Application::_, this, std::placeholders::_1)

namespace Saturn {

	Application::Application( const ApplicationSpecification& spec )
		: m_Specification( spec )
	{
		SingletonStorage::AddSingleton( this );

		const RubyMonitor& rPrimaryMonitor = RubyGetPrimaryMonitor();
		uint32_t width = 0, height = 0;

		width = 3 * rPrimaryMonitor.MonitorSize.x / 4;
		height = 3 * rPrimaryMonitor.MonitorSize.y / 4;

		// TEMP! Vulkan does not like default windows for some reason?
		RubyStyle WindowStyle = HasFlag( ApplicationFlag_Titlebar ) ? RubyStyle::Default : RubyStyle::Borderless;

		RubyWindowSpecification windowSpec { .Name = "Saturn", .Width = width, .Height = height, .GraphicsAPI = RubyGraphicsAPI::Vulkan, .Style = WindowStyle, .ShowNow = false };
		m_Window = new RubyWindow( windowSpec );
		m_Window->SetEventTarget( this );

		// This may not be the best way... but it's better than lazy loading.
		m_VulkanContext = new VulkanContext();

		m_VulkanContext->Init();

		// If we are in Dist we don't want to create the scene renderer just right now because we want to load the project and then load the shader bundle so then we can't right now create the scene renderer.
#if !defined( SAT_DIST )
		m_SceneRenderer = new SceneRenderer();
#endif

		if( m_Specification.WindowWidth != 0 && m_Specification.WindowHeight != 0 )
			m_Window->Resize( m_Specification.WindowWidth, m_Specification.WindowHeight );

		// Lazy load.
		AudioSystem::Get();
		RenderThread::Get().Enable( HasFlag( ApplicationFlag_UseGameThread ) );
		RenderThread::Get().Initialise();

		// ImGui is only used if we have the editor, and ImGui should not be used when building the game.
		m_ImGuiLayer = new ImGuiLayer();

		if( !HasFlag( ApplicationFlag_GameDist ) )
			m_ImGuiLayer->OnAttach();

#if defined( SAT_DIST )
		m_Window->Show( RubyWindowShowCmd::Fullscreen );
#else
		m_Window->Show();
#endif
	}

	Application::~Application()
	{
		SingletonStorage::RemoveSingleton( this );
	}

	void Application::Run()
	{
		// Tell children to create what ever they need.
		OnInit();

		while( m_Running )
		{
			m_Window->PollEvents();
		
			for( auto&& fn : m_MainThreadQueue )
				fn();

			m_MainThreadQueue.clear();

			if( !m_Window->Minimized() )
			{
				Renderer::Get().BeginFrame();
				{
					RenderThread::Get().Queue( [=] { m_SceneRenderer->RenderScene(); } );
					RenderThread::Get().Queue( [=] { Renderer2D::Get().Render(); } );

					// Render UI
					{
						RenderImGui();
					}
				}
				// End this frame on render thread.
				RenderThread::Get().Queue( [=] { Renderer::Get().EndFrame(); } );
			}

			// Execute render thread (last frame).
			RenderThread::Get().WaitAll();

			float time = ( float ) m_Window->GetTime();

			float frametime = time - m_LastFrameTime;

			m_Timestep = std::min<float>( frametime, 0.0333f );

			m_LastFrameTime = time;
		}

		OnShutdown();
		
		// So the difference between "Terminate" and delete is delete will completely destroy the class and remove it from the singleton list. 
		// However "Terminate" is used to destroy any data in the class but will not remove it from the singleton list, it is also used because we don't own the class so we can just implicitly destroy them.
		GameThread::Get().Terminate();
		RenderThread::Get().Terminate();

		VulkanContext::Get().SubmitTerminateResource( [&]() 
		{
			for ( auto& layer : m_Layers )
			{
				delete layer;
			}

			if( !HasFlag( ApplicationFlag_GameDist ) )
				m_ImGuiLayer->OnDetach();
			
			delete m_ImGuiLayer;

			m_ImGuiLayer = nullptr;
		} );

		AudioSystem::Get().Terminate();

		AssetManager::Get().Terminate();
		
		delete m_SceneRenderer;
		delete m_VulkanContext;

		delete m_Window;
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::RenderImGui()
	{
		SAT_PF_EVENT();

		// Begin on main thread.
		if( !HasFlag( ApplicationFlag_GameDist ) )
			m_ImGuiLayer->Begin();

		// Update on the main thread.
		for( auto& layer : m_Layers )
		{
			layer->OnUpdate( m_Timestep );
		}

		if( !HasFlag( ApplicationFlag_GameDist ) )
		{
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
	}

	bool Application::HasFlag( ApplicationFlags flag )
	{
		return ( m_Specification.Flags & (uint32_t)flag ) != 0;
	}

	std::filesystem::path Application::GetAppDataFolder()
	{
		std::filesystem::path path = L"";

#if defined(SAT_PLATFORM_WINDOWS)
		PWSTR Path = 0;
		SHGetKnownFolderPath( FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, &Path );

		std::wstring fp = Path;
		std::replace( fp.begin(), fp.end(), L'\\', L'/' );

		path = fp + L"/Saturn";
#elif defined(SAT_PLATFORM_LINUX)

#endif

		if( !std::filesystem::exists( path ) )
			std::filesystem::create_directories( path );

		return path;
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

	bool Application::OnEvent( RubyEvent& rEvent )
	{
		switch( rEvent.Type )
		{
			case RubyEventType::Resize:
			{
				OnWindowResize( ( RubyWindowResizeEvent& )rEvent );
			} break;

			case RubyEventType::Close: 
			{
				Close();
			} break;
		}

		VulkanContext::Get().OnEvent( rEvent );

		if( m_ImGuiLayer )
			m_ImGuiLayer->OnEvent( rEvent );

		// Pass events to layers, this is the only place in the engine where we actually care if an event is handled or not.
		// Process Events backwards. This is so that if we are in a game and we click a button if the first layer gets that event it might shoot in the game however we wanted to click a button not shoot.
		for( auto itr = m_Layers.end(); itr != m_Layers.begin(); )
		{
			( *--itr )->OnEvent( rEvent );

			if( rEvent.Handled )
				break;
		}

		return true;
	}

	bool Application::OnWindowResize( RubyWindowResizeEvent& e )
	{
		int width = e.GetWidth(), height = e.GetHeight();

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
		ofn.hwndOwner = ( HWND ) m_Window->GetNativeHandle();
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
		ofn.hwndOwner = ( HWND ) m_Window->GetNativeHandle();
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
}
