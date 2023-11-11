/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2023 BEAST                                                           		*
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

#include "Backend/RubyBackendBase.h"
#include "RubyEvent.h"
#include "RubyPerfTimer.h"

#include <string_view>
#include <unordered_set>

class RBY_API RubyWindow
{
public:
	RubyWindow( const RubyWindowSpecification& rSpec );
	~RubyWindow();

	void PollEvents();
	bool ShouldClose();

	void Maximize();
	void Minimize();
	void Restore();
	void Resize( uint32_t Width, uint32_t Height );
	void Show();
	void SetPosition( int x, int y );
	void SetMousePos( double x, double y );
	void GetMousePos( double* x, double* y );
	void SetMouseCursor( RubyCursor Cursor );
	void HideMouseCursor();

	RubyIVec2 GetPosition() { return m_Position; }

	bool IsFocused();
	bool Minimized();
	bool Maximized();

	void ChangeTitle( std::string_view Title );

	uint32_t GetWidth() { return m_Width; }
	uint32_t GetHeight() { return m_Height; }
	RubyGraphicsAPI GetGraphicsAPI() { return m_GraphicsAPI; }

	bool IsKeyDown( RubyKey key );
	bool IsMouseButtonDown( RubyMouseButton button );

	double GetTime() { return m_Timer.GetTicks(); }

	RubyStyle GetStyle() { return m_Style; }

public:
	void* GetNativeHandle();
public:
	//////////////////////////////////////////////////////////////////////////
	// OpenGL Functions

	void GLSwapBuffers();

	//////////////////////////////////////////////////////////////////////////
	// Vulkan Functions
	std::vector<const char*> GetVulkanRequiredExtensions();
	VkResult CreateVulkanWindowSurface( VkInstance Instance, VkSurfaceKHR* pOutSurface );

public:

	template<typename Ty>
	void SetEventTarget( Ty* Target ) 
	{
		m_pEventTarget = Target;
	}

	RubyEventTarget* GetEventTarget() { return m_pEventTarget; }

	template<typename Ty, typename... Args>
	bool DispatchEvent( RubyEventType Type, Args&&... args ) 
	{
		Ty event( Type, std::forward<Args>( args )... );

		if( m_pEventTarget )
			return m_pEventTarget->OnEvent( event );

		return false;
	}

public:
	// Internal Functions. Do not call.
	void SetSize( uint32_t width, uint32_t height );
	void SetPos( int x, int y );

	void SetKeyDown( RubyKey key, bool value )
	{
		if( value )
			m_Keys.insert( key );
		else
			m_Keys.erase( key );
	}

	void SetMouseDown( RubyMouseButton button, bool value = true )
	{ 
		if( value )
			m_CurrentMouseButton = button;
		else
			m_CurrentMouseButton = RubyMouseButton::Unknown;
	}

protected:
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;

	std::unordered_set<RubyKey> m_Keys;
	RubyMouseButton m_CurrentMouseButton = RubyMouseButton::Unknown;

	RubyIVec2 m_Position{};
	RubyPerfTimer m_Timer;

private:
	RubyBackendBase* m_pDefaultBackend = nullptr;
	RubyEventTarget* m_pEventTarget = nullptr;

	std::string m_WindowTitle = "";
	RubyGraphicsAPI m_GraphicsAPI = RubyGraphicsAPI::None;
	RubyStyle m_Style = RubyStyle::Default;

private:
	friend class RubyBackendBase;

#if defined(_WIN32)
	friend class RubyWindowsBackend;
#endif
};
