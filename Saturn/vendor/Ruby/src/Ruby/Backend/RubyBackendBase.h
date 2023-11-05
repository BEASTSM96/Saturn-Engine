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

struct WindowSpecification;
class RubyWindow;

enum RubyGraphicsAPI
{
	OpenGL = RBY_BIT( 1 ),
	Vulkan = RBY_BIT( 2 ),
	DirectX11 = RBY_BIT( 3 ),
	DirectX12 = RBY_BIT( 4 ),
	None = RBY_BIT( 5 )
};

class RubyBackendBase
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

	virtual void SetTitle( std::wstring_view Title ) = 0;

	virtual void CreateGraphics( RubyGraphicsAPI api ) = 0;

	virtual void IssueSwapBuffers() = 0;

	virtual void Maximize() = 0;
	virtual void Minimize() = 0;
	virtual void Restore() = 0;

public:
	virtual void PollEvents() = 0;
	virtual bool PendingClose() = 0;

protected:
	bool m_ShouldClose = false;
	RubyGraphicsAPI m_GraphicsAPI = RubyGraphicsAPI::None;
	RubyWindow* m_pWindow = nullptr;
private:
	friend class RubyWindow;
};