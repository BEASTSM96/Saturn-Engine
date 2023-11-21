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
#include "Input.h"

#include "App.h"

#include <Ruby/RubyWindow.h>

#include <vector>

namespace Saturn {

	Input::Input()
	{
		SingletonStorage::Get().AddSingleton( this );
	}

	bool Input::KeyPressed( RubyKey key )
	{
		return Application::Get().GetWindow()->IsKeyDown( key );
	}

	bool Input::MouseButtonPressed( RubyMouseButton button )
	{
		return Application::Get().GetWindow()->IsMouseButtonDown( button );
	}

	float Input::MouseX()
	{
		return MousePosition().x;
	}

	float Input::MouseY()
	{
		return MousePosition().y;
	}

	glm::vec2 Input::MousePosition()
	{
		double x = 0.0;
		double y = 0.0;

		Application::Get().GetWindow()->GetMousePos( &x, &y );

		return { ( float ) x, ( float ) y };
	}

	void Input::SetCursorMode( RubyCursorMode mode )
	{
		Application::Get().GetWindow()->SetMouseCursorMode( mode );
	}

	RubyCursorMode Input::GetCursorMode()
	{
		return Application::Get().GetWindow()->GetCursorMode();
	}

}