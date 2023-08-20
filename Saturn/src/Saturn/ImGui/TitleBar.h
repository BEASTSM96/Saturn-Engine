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

#include "Saturn/Core/App.h"

#include "Saturn/Vulkan/Texture.h"

#include "Panel/Panel.h"

#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace Saturn {

	class TitleBar : public Panel
	{
		using MenuBarFunction = std::function<void()>;
	public:
		TitleBar();
		~TitleBar();

		void Draw() override;

		float Height() const { return m_Height; }

		void AddMenuBarFunction( MenuBarFunction&& rrFunc );
		void AddOnRuntimeStateChanged( std::function<void( int state )>&& rrFunc );
		void AddOnExitFunction( std::function<void()>&& rrFunc );
		
	private:
		std::vector<MenuBarFunction> m_MenuBarFunctions;
		std::function<void()> m_OnExitFunction = nullptr;
		std::function<void(int state)> m_OnRuntimeStateChanged = nullptr;

		float m_Height = 0.0f;

		Ref<Texture2D> m_PlayImage = nullptr;
		Ref<Texture2D> m_StopImage = nullptr;

		// Yes, this could be a bool however, I might want to add more states and a bool will not do that.
		// 0 not running, 1 running
		int m_RuntimeState = 0;
	};

}