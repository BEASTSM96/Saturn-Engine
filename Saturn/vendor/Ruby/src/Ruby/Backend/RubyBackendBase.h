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

#include "RubyCore.h"

#include <string>
#include <stdint.h>

#if defined( RBY_INCLUDE_VULKAN )
#include <vulkan.h>
#endif

struct RubyWindowSpecification;
class RubyWindow;

class RBY_API RubyBackendBase
{
public:
	RubyBackendBase() {}
	virtual ~RubyBackendBase() = default;

public:
	virtual void Create() = 0;
	virtual void DestroyWindow() = 0;

	virtual void CloseWindow() = 0;
	virtual void PresentWindow() = 0;

	virtual void ResizeWindow( uint32_t Width, uint32_t Height ) = 0;

	virtual void SetTitle( std::string_view Title ) = 0;

	virtual void CreateGraphics( RubyGraphicsAPI api ) = 0;

	virtual void IssueSwapBuffers() = 0;

	virtual void Maximize() = 0;
	virtual void Minimize() = 0;
	virtual void Restore() = 0;

	virtual void MoveWindow( int x, int y ) = 0;

	virtual void SetMousePos( double x, double y ) = 0;
	virtual void GetMousePos( double* x, double* y ) = 0;

	virtual void* GetNativeHandle() = 0;

	virtual VkResult CreateVulkanWindowSurface( VkInstance Instance, VkSurfaceKHR* pOutSurface ) = 0;

	virtual bool Minimized() = 0;
	virtual bool Maximized() = 0;
	virtual bool Focused() = 0;

	virtual void SetMouseCursor( RubyCursor Cursor ) = 0;
	virtual void SetMouseCursorMode( RubyCursorMode mode ) = 0;

public:
	virtual void PollEvents() = 0;
	virtual bool PendingClose() = 0;

protected:
	bool m_ShouldClose = false;
	bool m_BlockMouseCursor = false;

	RubyWindowSpecification m_WindowSpecification{};
	RubyWindow* m_pWindow = nullptr;
private:
	friend class RubyWindow;
};