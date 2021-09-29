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

// Fix GLFW defines not being defined when they are defined
#if defined ( SAT_PLATFORM_LINUX )
#define _GLFW_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif defined ( SAT_PLATFORM_MAC )
#define _GLFW_COCOA
#define GLFW_EXPOSE_NATIVE_COCOA
#endif // SAT_PLATFORM_LINUX

#if defined( SAT_PLATFORM_WINDOWS )
#include <Windows.h>
#endif // SAT_PLATFORM_WINDOWS

#include "Saturn/Core/App.h"

int main( int count, char** args ) 
{
	Saturn::Application::Get();

	Saturn::Application::Get().Run();
}

#if defined ( SAT_PLATFORM_WINDOWS )

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd ) 
{
	return main( __argc, __argv );
}

#endif // SAT_PLATFORM_WINDOWS