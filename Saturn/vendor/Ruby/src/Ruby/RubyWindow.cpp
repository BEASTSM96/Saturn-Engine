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

#include "RubyWindow.h"

#if defined(_WIN32)
#include "Backend/RubyWindowsBackend.h"
#endif

RubyWindow::RubyWindow( const WindowSpecification& rSpec )
	: m_WindowTitle( rSpec.Name ), m_Width( rSpec.Width ), m_Height( rSpec.Height ), m_GraphicsAPI( rSpec.GraphicsAPI )
{
#if defined(_WIN32)
	m_pDefaultBackend = new RubyWindowsBackend( rSpec, this );
#endif

	m_pDefaultBackend->Create();
	m_pDefaultBackend->PresentWindow();
}

RubyWindow::~RubyWindow()
{
	m_pDefaultBackend->DestroyWindow();
	delete m_pDefaultBackend;
}

void RubyWindow::PollEvents()
{
	m_pDefaultBackend->PollEvents();
}

bool RubyWindow::ShouldClose()
{
	return !m_pDefaultBackend->PendingClose();
}

void RubyWindow::Maximize()
{
	m_pDefaultBackend->Maximize();
}

void RubyWindow::Minimize()
{
	m_pDefaultBackend->Minimize();
}

void RubyWindow::Restore()
{
	m_pDefaultBackend->Restore();
}

void RubyWindow::Resize( uint32_t Width, uint32_t Height )
{
	m_pDefaultBackend->ResizeWindow( Width, Height );
}

void RubyWindow::ChangeTitle( std::wstring_view Title )
{
	m_pDefaultBackend->SetTitle( Title );
}

void RubyWindow::GLSwapBuffers()
{
	m_pDefaultBackend->IssueSwapBuffers();
}
