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

#include "RubyBackendBase.h"

#include <Windows.h>

class RBY_API RubyWindowsBackend : public RubyBackendBase
{
	typedef HGLRC WINAPI wglCreateContextAttribsARBFn( HDC, HGLRC, const int* );
	typedef BOOL WINAPI wglChoosePixelFormatARBFn( HDC, const int*, const FLOAT*, UINT, int*, UINT* );
public:
	RubyWindowsBackend( const RubyWindowSpecification& rSpec, RubyWindow* pWindow );
	~RubyWindowsBackend();

	RubyWindow* GetParent() { return m_pWindow; }

	void Maximize() override;
	void Minimize() override;
	void Restore() override;


public:
	void Create() override;
	void DestroyWindow() override;

	void CloseWindow() override;
	void PresentWindow() override;

	void ResizeWindow( uint32_t Width, uint32_t Height ) override;

	void SetTitle( std::wstring_view Title ) override;

	void CreateGraphics( RubyGraphicsAPI api ) override;

	void IssueSwapBuffers() override;
	VkResult CreateVulkanWindowSurface( VkInstance Instance, VkSurfaceKHR* pOutSurface ) override;

public:
	// Events
	void PollEvents() override;
	bool PendingClose() override;

private:
	void CreateDummyWindow();
	DWORD ChooseStyle();

private:
	HWND m_Handle = nullptr;
	HDC m_DrawContent = nullptr;
	HGLRC m_OpenGLRenderContext = nullptr;

	wglCreateContextAttribsARBFn* wglCreateContextAttribsARB = nullptr;
	wglChoosePixelFormatARBFn* wglChoosePixelFormatARB = nullptr;
};