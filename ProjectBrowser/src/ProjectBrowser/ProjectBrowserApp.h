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

#include "Saturn/Application.h"
#include "Saturn/Core/Base.h"

namespace ProjectBrowser {

	class ProjectBrowserLayer;

	class ProjectBrowserApp
	{
	public:
		ProjectBrowserApp( Saturn::ApplicationCommandLineArgs args, const Saturn::ApplicationProps& props );
		~ProjectBrowserApp();

		void Run( void );

		static ProjectBrowserApp& Get() { return *s_Instance; }

		Saturn::ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		void SetPendingClose( bool close ) { m_PendingClose = close; }

		void OnEvent( Saturn::Event& e );

		Saturn::Layer* PushLayer( Saturn::Layer* layer );
		void PushOverlay( Saturn::Layer* layer );
		void RenderImGui( void );

		void Init( void );

		Saturn::Window& GetWindow() { return *m_Window; }

	protected:
		
	private:
		bool OnWindowClose( Saturn::WindowCloseEvent& e );
		bool OnWindowResize( Saturn::WindowResizeEvent& e );
	private:
		Saturn::ApplicationCommandLineArgs m_CommandLineArgs;
		Saturn::VersionCtrl m_VersionCtrl;

		ProjectBrowserLayer* m_ImGuiLayer;

		bool m_Running = true;
		bool m_PendingClose = false;

		Saturn::LayerStack m_LayerStack;

		bool m_Crashed = false;

		float LastFrameTime = 0.0f;

		bool m_Minimized = false;

		Saturn::Timestep m_TimeStep;

		std::unique_ptr<Saturn::Window> m_Window;
	private:
		static ProjectBrowserApp* s_Instance;
	};
}