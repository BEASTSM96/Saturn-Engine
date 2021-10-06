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

#include "Base.h"

#include "Saturn/ImGui/TitleBar.h"

#include <string>

#if defined ( SAT_LINUX ) || defined ( SAT_MAC ) 
#include <cstring>
#endif

#if defined( SAT_WINDOWS )
#include <Windows.h>
#endif

struct GLFWwindow;

namespace Saturn {

	class Window
	{
		SINGLETON( Window );

		Window();
		~Window();

	public:

		void OnUpdate();

		void Maximize();
		void Minimize();

		void Restore();
		void SetTitle( const std::string& title );

		void Render();

		int Width() { return m_Width; }
		int Height() { return m_Height; }

	private:

		static void SizeCallback( GLFWwindow* wind, int h, int w );

	private:
		GLFWwindow* m_Window = nullptr;

		int m_Height = 720;
		int m_Width  = 1200;
		std::string m_Title = "Saturn";

		bool Minimized = false;

		// Widgets

		TitleBar* m_TitleBar;

	#if defined ( SAT_WINDOWS )
		WNDPROC  m_WindowProc  = nullptr;
		static LRESULT WindowProc( HWND handle, UINT msg, WPARAM WParam, LPARAM LParam );
	#endif
	};
}
