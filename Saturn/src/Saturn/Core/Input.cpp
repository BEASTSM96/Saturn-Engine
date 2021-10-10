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
#include "Input.h"

#include <GLFW/glfw3.h>
#include "Saturn/Core/Window.h"

#include <vector>

namespace Saturn {

	bool Input::KeyPressed( KeyCode key )
	{
		auto& window = static_cast< Window& >( Window::Get() );
		auto state = glfwGetKey( static_cast< GLFWwindow* >( window.NativeWindow() ), static_cast< int32_t >( key ) );

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::MouseButtonPressed( MouseButton button )
	{
		auto& window = static_cast< Window& >( Window::Get() );
		auto state = glfwGetMouseButton( static_cast< GLFWwindow* >( window.NativeWindow() ), static_cast< int32_t >( button ) );

		return state == GLFW_PRESS;
	}

	float Input::MouseX()
	{
		auto [x, y] = MousePosition();
		return ( float )x;
	}

	float Input::MouseY()
	{
		auto [x, y] = MousePosition();
		return ( float )y;
	}

	std::pair<float, float> Input::MousePosition()
	{
		auto& window = static_cast< Window& >( Window::Get() );

		double x, y;
		glfwGetCursorPos( static_cast< GLFWwindow* >( window.NativeWindow() ), &x, &y );
		return { ( float )x, ( float )y };
	}

	void Input::SetCursorMode( CursorMode mode )
	{
		auto& window = static_cast< Window& >( Window::Get() );

		glfwSetInputMode( static_cast< GLFWwindow* >( window.NativeWindow() ), GLFW_CURSOR, GLFW_CURSOR_NORMAL + ( int )mode );
	}

	CursorMode Input::GetCursorMode()
	{
		auto& window = static_cast< Window& >( Window::Get() );

		return ( CursorMode )( glfwGetInputMode( static_cast< GLFWwindow* >( window.NativeWindow() ), GLFW_CURSOR ) - GLFW_CURSOR_NORMAL );
	}

}