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

#include "sppch.h"

#include "Core/Base.h"
#include "Events/Event.h"

namespace Saturn {

	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

#ifdef SAT_DEBUG
		WindowProps( const std::string& title = "Saturn Engine Mode : Debug",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title( title ), Width( width ), Height(height)
		{
		}
#endif // SAT_DEBUG

#ifdef SAT_DIST
		WindowProps( const std::string& title = "Saturn Engine",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title( title ), Width( width ), Height( height )
		{
		}
#endif // SAT_DIST

#ifdef SAT_RELEASE
		WindowProps( const std::string& title = "Saturn Engine Mode : Release",
			unsigned int width = 1280,
			unsigned int height = 720)
			: Title( title ), Width( width ), Height( height )
		{
		}
#endif // SAT_RELEASE
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate( void ) = 0;

		virtual unsigned int GetWidth( void ) const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback( const EventCallbackFn& callback ) = 0;
		virtual void SetVSync( bool enabled ) = 0;
		virtual bool IsVSync( void ) const = 0;
		virtual void Maximize( void ) = 0;
		virtual void SetTitle( const std::string& title ) = 0;

		virtual void* GetNativeWindow( void ) const = 0;

		static Window* Create( const WindowProps& props = WindowProps() );
	};

}