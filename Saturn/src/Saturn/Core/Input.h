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

#pragma once

#include "Saturn/Core/Base.h"

#include <Ruby/RubyEventType.h>

#include <sstream>
#include <fstream>
#include <algorithm>
#include <utility>

#include <glm/glm.hpp>

namespace Saturn {

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	class Input
	{
	public:
		static inline Input& Get() { return *SingletonStorage::Get().GetOrCreateSingleton<Input>(); }
	public:
		Input();
		~Input() { }

		bool KeyPressed( RubyKey key );
		bool MouseButtonPressed( RubyMouseButton button );

		float MouseX();
		float MouseY();

		glm::vec2 MousePosition();

		void SetCursorMode( CursorMode mode );
		CursorMode GetCursorMode();
	};
}