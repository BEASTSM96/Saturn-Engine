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

#include "Saturn/Core/Base.h"
#include "KeyCodes.h"
#include "MouseButtons.h"

namespace Saturn {

	class SATURN_API Input
	{
	public:
		static bool IsKeyPressed( int keycode ) { return s_Instance->IsKeyPressedImpl( keycode ); }

		static float GetMouseX() { return s_Instance->GetMouseXImpl(); }

		static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

		static std::pair< float, float > GetMousePos() { return s_Instance->GetMousePosImpl(); }

		static bool IsMouseButtonPressed( int button ) { return s_Instance->IsMouseButtonPressedImpl( button ); }

	protected:
		virtual bool IsKeyPressedImpl( int keycode ) = 0;

		virtual float GetMouseXImpl( void ) = 0;

		virtual float GetMouseYImpl( void ) = 0;

		virtual std::pair< float, float > GetMousePosImpl() = 0;

		virtual bool IsMouseButtonPressedImpl( int button ) = 0;
	private:
		static Input* s_Instance;

	};
}