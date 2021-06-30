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
#include "Saturn/Input.h"

#include <GLFW/glfw3.h>
#include "Saturn/Application.h"
#include "WindowsWindow.h"

namespace Saturn {

	bool Input::IsKeyPressed( KeyCode keycode )
	{
		auto& window = static_cast< WindowsWindow& >( Application::Get().GetWindow() );
		auto state = glfwGetKey( static_cast< GLFWwindow* >( window.GetNativeWindow() ), static_cast< int32_t >( keycode ) );
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed( MouseButton button )
	{
		auto& window = static_cast< WindowsWindow& >( Application::Get().GetWindow() );
		auto state = glfwGetMouseButton( static_cast< GLFWwindow* >( window.GetNativeWindow() ), static_cast< int32_t >( button ) );
		return state == GLFW_PRESS;	
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return ( float )x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return ( float )y;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto& window = static_cast< WindowsWindow& >( Application::Get().GetWindow() );

		double x, y;
		glfwGetCursorPos( static_cast< GLFWwindow* >( window.GetNativeWindow() ), &x, &y );
		return { ( float )x, ( float )y };
	}

	void Input::SetCursorMode( CursorMode mode )
	{
		auto& window = static_cast< WindowsWindow& >( Application::Get().GetWindow() );
		glfwSetInputMode( static_cast< GLFWwindow* >( window.GetNativeWindow() ), GLFW_CURSOR, GLFW_CURSOR_NORMAL + ( int )mode );
	}

	CursorMode Input::GetCursorMode()
	{
		auto& window = static_cast< WindowsWindow& >( Application::Get().GetWindow() );
		return ( CursorMode )( glfwGetInputMode( static_cast< GLFWwindow* >( window.GetNativeWindow() ), GLFW_CURSOR ) - GLFW_CURSOR_NORMAL );
	}
}

