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

#include <string_view>
#include <unordered_map>

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

	void ChangeTitle( std::wstring_view Title );

	uint32_t GetWidth() { return m_Width; }
	uint32_t GetHeight() { return m_Height; }
	RubyGraphicsAPI GetGraphicsAPI() { return m_GraphicsAPI; }

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

	template<typename Ty, typename... Args>
	bool DispatchEvent( RubyEventType Type, Args&&... args ) 
	{
		Ty event( Type, std::forward<Args>( args )... );

		if( m_pEventTarget )
			return m_pEventTarget->OnEvent( event );

		return false;
	}

private:
	RubyBackendBase* m_pDefaultBackend = nullptr;
	RubyEventTarget* m_pEventTarget = nullptr;

	std::wstring m_WindowTitle = L"";
	RubyGraphicsAPI m_GraphicsAPI = RubyGraphicsAPI::None;

	uint32_t m_Height = 0;
	uint32_t m_Width = 0;

private:
	friend class RubyBackendBase;

#if defined(_WIN32)
	friend class RubyWindowsBackend;
#endif
};
