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

RubyWindow::RubyWindow( const RubyWindowSpecification& rSpec )
	: m_WindowTitle( rSpec.Name ), m_Width( rSpec.Width ), m_Height( rSpec.Height ), m_GraphicsAPI( rSpec.GraphicsAPI ), m_Style( rSpec.Style )
{
#if defined(_WIN32)
	m_pDefaultBackend = new RubyWindowsBackend( rSpec, this );
#endif

	m_pDefaultBackend->Create();

	if( rSpec.ShowNow )
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
	// Win32 will handle if we should maximize or restore the window natively.
	// However if we aren't using a border then we'll need to do this our self.
	switch( m_Style )
	{
		case RubyStyle::Default: 
		{
			m_pDefaultBackend->Maximize();
		} break;

		case RubyStyle::Borderless:
		{
			if( Maximized() )
			{
				Restore();
			}
			else
			{
				m_pDefaultBackend->Maximize();
			}
		}	break;

		default:
			break;
	}
}

void RubyWindow::Minimize()
{
	// Win32 will handle if we should maximize or restore the window natively.
	// However if we aren't using a border then we'll need to do this our self.
	switch( m_Style )
	{
		case RubyStyle::Default: 
		{
			m_pDefaultBackend->Minimize();
		} break;

		case RubyStyle::Borderless:
		{
			if( Minimized() )
			{
				Restore();
			}
			else
			{
				m_pDefaultBackend->Minimize();
			}
		}	break;
		
		default:
			break;
	}
}

void RubyWindow::Restore()
{
	m_pDefaultBackend->Restore();
}

void RubyWindow::Resize( uint32_t Width, uint32_t Height )
{
	m_Width = Width;
	m_Height = Height;

	m_pDefaultBackend->ResizeWindow( Width, Height );
}

void RubyWindow::Show()
{
	m_pDefaultBackend->PresentWindow();
}

void RubyWindow::SetPosition( int x, int y )
{
	m_Position.x = x;
	m_Position.y = y;

	m_pDefaultBackend->MoveWindow( x, y );
}

void RubyWindow::SetMousePos( double x, double y )
{
	m_pDefaultBackend->SetMousePos( x, y );
}

void RubyWindow::GetMousePos( double* x, double* y )
{
	m_pDefaultBackend->GetMousePos( x, y );
}

bool RubyWindow::IsFocused()
{
	return m_pDefaultBackend->Focused();
}

bool RubyWindow::Minimized()
{
	return m_pDefaultBackend->Minimized();
}

bool RubyWindow::Maximized()
{
	return m_pDefaultBackend->Maximized();
}

void RubyWindow::ChangeTitle( std::string_view Title )
{
	m_pDefaultBackend->SetTitle( Title );
}

bool RubyWindow::IsKeyDown( RubyKey key )
{
	return m_CurrentKey == key;
}

bool RubyWindow::IsMouseButtonDown( RubyMouseButton button )
{
	return m_CurrentMouseButton == button;
}

void* RubyWindow::GetNativeHandle()
{
	return (void*)m_pDefaultBackend->GetNativeHandle();
}

void RubyWindow::GLSwapBuffers()
{
	m_pDefaultBackend->IssueSwapBuffers();
}

std::vector<const char*> RubyWindow::GetVulkanRequiredExtensions()
{
	std::vector<const char*> Extensions;
	Extensions.push_back( "VK_KHR_surface" );

#if defined(_WIN32)
	Extensions.push_back( "VK_KHR_win32_surface" );
#endif

	return Extensions;
}

VkResult RubyWindow::CreateVulkanWindowSurface( VkInstance Instance, VkSurfaceKHR* pOutSurface )
{
	return m_pDefaultBackend->CreateVulkanWindowSurface( Instance, pOutSurface );
}

void RubyWindow::SetSize( uint32_t width, uint32_t height )
{
	m_Width = width;
	m_Height = height;
}

void RubyWindow::SetPos( int x, int y )
{
	m_Position.x = x;
	m_Position.y = y;
}
