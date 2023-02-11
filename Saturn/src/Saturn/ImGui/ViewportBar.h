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

#include "Panel/Panel.h"

#include "Saturn/Core/Base.h"
#include "Saturn/Vulkan/Texture.h"

namespace Saturn {

	class ViewportBar : public Panel
	{
	public:
		ViewportBar();
		~ViewportBar();
		
		virtual void Draw() override;

		bool RequestedStartRuntime() { return m_WantsToStartRuntime; }

	private:
		bool m_WantsToStartRuntime = false;

		Ref< Texture2D > m_PlayImage;
		Ref< Texture2D > m_PauseImage;
		Ref< Texture2D > m_StopImage;
		Ref< Texture2D > m_CursorTexture;
		Ref< Texture2D > m_MoveTexture;
		Ref< Texture2D > m_ScaleTexture;
		Ref< Texture2D > m_RotateTexture;
		Ref< Texture2D > m_SettingsTexture;
	};
}