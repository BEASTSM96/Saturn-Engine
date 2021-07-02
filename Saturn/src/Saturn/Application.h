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

#pragma once

#include "Core/Base.h"

#include "Window.h"
#include "Saturn/LayerStack.h"
#include "Saturn/Events/Event.h"
#include "Saturn/Events/ApplicationEvent.h"
#include "Saturn/ImGui/ImGuiLayer.h"
#include "Saturn/Core/Timestep.h"

#include <string>
#include <vector>

namespace Saturn {
	class ModuleManager;
	class Module;
	class SceneManager;
}

namespace Saturn {

	class ModuleManager;
	class Module;

	struct ApplicationProps
	{
		std::string Name;
		uint32_t WindowWidth, WindowHeight;
	};

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[]( int index ) const
		{
			SAT_CORE_ASSERT( index < Count );
			return Args[ index ];
		}
	};

	struct VersionCtrl
	{
		UUID FixedUUID = NULL;
		std::string Branch;
	};

	class SATURN_API Application
	{
	public:
		Application( ApplicationCommandLineArgs args, const ApplicationProps& props ={ "Saturn Engine(Pre Init), (????/????) ", 1280, 720 } );
		virtual ~Application( void );

		void Run( void );

		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		/*----------For Editor----------*/
		virtual void OnInit( void ) { }
		virtual void OnShutdown( void ) { }
		virtual void OnShutdownSave( void ) { }
		/*------------------------------*/

		void OnEvent( Event& e );

		Layer* PushLayer( Layer* layer );
		void PushOverlay( Layer* layer );
		void RenderImGui( void );

		void Init( void );

		Window& GetWindow() { return *m_Window; }

		static Application& Get() { return *s_Instance; }
		static bool IsRunning( void ) { return Get().m_Running; }
		static bool GetMinimized( void ) { return Get().m_Minimized; }

		static void Close() { Get().m_Running = false; }

		std::pair< std::string, std::string > OpenFile( const char* filter ) const;
		std::pair< std::string, std::string > SaveFile( const char* f ) const;

		Timestep& GetTimeStep() { return Get().m_TimeStep;  }

		static const char* GetConfigurationName();
		static const char* GetPlatformName();

		UUID& GetFixedVersionUUID() { return m_VersionCtrl.FixedUUID;  };
		VersionCtrl& GetVersionCtrl() { return m_VersionCtrl; };
	public:
		Scene& GetCurrentScene( void ) { return *Get().m_Scene; }
	private:
		bool OnWindowClose( WindowCloseEvent& e );
		bool OnWindowResize( WindowResizeEvent& e );

	protected:

		Ref< Scene > m_Scene;
		std::unique_ptr< Window > m_Window;

	private:
		ApplicationCommandLineArgs m_CommandLineArgs;
		VersionCtrl m_VersionCtrl;
		
		ImGuiLayer* m_ImGuiLayer;

		bool m_Running = true;

		LayerStack m_LayerStack;

		bool m_Crashed = false;

		float LastFrameTime = 0.0f;

		bool m_Minimized = false;

		Timestep m_TimeStep;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication( ApplicationCommandLineArgs args );
}