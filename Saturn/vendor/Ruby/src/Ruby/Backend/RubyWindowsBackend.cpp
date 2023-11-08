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

#include "Ruby/RubyWindow.h"
#include "RubyWindowsBackend.h"

#if defined( RBY_INCLUDE_VULKAN )
#include <vulkan_win32.h>
#endif

#include <codecvt>
#include <locale>

//////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK RubyWindowProc( HWND Handle, UINT Msg, WPARAM WParam, LPARAM LParam );

const wchar_t* DefaultClassName = L"RUBY_WINDOW";

struct RubyWindowRegister
{
	RubyWindowRegister() 
	{
		WNDCLASS wc = {};
		wc.lpfnWndProc = RubyWindowProc;
		wc.hInstance = GetModuleHandle( NULL );
		wc.lpszClassName = DefaultClassName;
		//wc.hbrBackground = ( HBRUSH ) COLOR_WINDOW;
		wc.hCursor = LoadCursor( nullptr, IDC_ARROW );
		wc.style = CS_OWNDC;

		::RegisterClass( &wc );
	}

	~RubyWindowRegister()
	{
		::UnregisterClass( DefaultClassName, GetModuleHandle( NULL ) );
	}
} static s_RubyWindowRegister;

//////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK RubyWindowProc( HWND Handle, UINT Msg, WPARAM WParam, LPARAM LParam ) 
{
	RubyWindowsBackend* pThis = ( RubyWindowsBackend* ) ::GetPropW( Handle, L"RubyData" );

	if( !pThis )
		return ::DefWindowProc( Handle, Msg, WParam, LParam );

	switch( Msg )
	{
		case WM_CLOSE:
		{
			pThis->CloseWindow();
		} break;

		case WM_DESTROY:
		{
			::PostQuitMessage( 0 );
		} break;

		//////////////////////////////////////////////////////////////////////////
		// Resize

		case WM_SIZE: 
		{
			UINT width = LOWORD( LParam );
			UINT height = HIWORD( LParam );

			if( WParam == SIZE_MAXIMIZED )
			{
				pThis->GetParent()->DispatchEvent<RubyMaximizeEvent>( RubyEventType::WindowMaximized, true );
			}
			else if( WParam == SIZE_MINIMIZED ) 
			{
				pThis->GetParent()->DispatchEvent<RubyMinimizeEvent>( RubyEventType::WindowMinimized, true );
			}
			else if( WParam == SIZE_RESTORED )
			{
				//pThis->GetParent()->DispatchEvent<RubyMinimizeEvent>( RubyEventType::WindowRestored, true );
			}

			pThis->GetParent()->SetSize( width, height );
			pThis->GetParent()->DispatchEvent<RubyWindowResizeEvent>( RubyEventType::Resize, static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) );
		} break;

		case WM_ENTERSIZEMOVE:
		{
			UINT width = LOWORD( LParam );
			UINT height = HIWORD( LParam );

			pThis->GetParent()->SetSize( width, height );
			pThis->GetParent()->DispatchEvent<RubyWindowResizeEvent>( RubyEventType::Resize, static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) );
		} break;

		//////////////////////////////////////////////////////////////////////////
		// Window Position & Focus

		case WM_WINDOWPOSCHANGING: 
		{
			WINDOWPOS* Info = ( WINDOWPOS* ) LParam;

			pThis->GetParent()->SetPos( Info->x, Info->y );
		} break;

		case WM_SETFOCUS: 
		{
			pThis->GetParent()->SetFocus( true );
		} break;

		case WM_KILLFOCUS:
		{
			pThis->GetParent()->SetFocus( false );
		} break;

		//////////////////////////////////////////////////////////////////////////
		// BEGIN: Mouse Events
		// Mouse Move

		case WM_MOUSEMOVE:
		{
			float x = (float)LOWORD( LParam );
			float y = (float)HIWORD( LParam );

			pThis->GetParent()->DispatchEvent<RubyMouseMoveEvent>( RubyEventType::MouseMoved, x, y );
		} break;

		//////////////////////////////////////////////////////////////////////////
		// Mouse Button Pressed & Released

		case WM_LBUTTONDOWN:
		{
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MousePressed, 0 );
		} break;

		case WM_RBUTTONDOWN:
		{
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MousePressed, 1 );
		} break;

		case WM_MBUTTONDOWN:
		{
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MousePressed, 2 );
		} break;

		case WM_LBUTTONUP:
		{
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MouseReleased, 0 );
		} break;

		case WM_RBUTTONUP:
		{
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MouseReleased, 1 );
		} break;

		case WM_MBUTTONUP:
		{
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MouseReleased, 2 );
		} break;

		case WM_MOUSEHOVER:
		{
			pThis->GetParent()->DispatchEvent<RubyEvent>( RubyEventType::MouseEnterWindow );
		} break;

		case WM_MOUSELAST:
		{
			pThis->GetParent()->DispatchEvent<RubyEvent>( RubyEventType::MouseLeaveWindow );
		} break;

		// END: Mouse Events
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Key Events

		case WM_KEYDOWN:
		{
			int nativeCode = ( int ) WParam;

			pThis->GetParent()->DispatchEvent<RubyKeyEvent>( RubyEventType::KeyPressed, nativeCode );
		} break;

		case WM_KEYUP:
		{
			int nativeCode = ( int ) WParam;

			pThis->GetParent()->DispatchEvent<RubyKeyEvent>( RubyEventType::KeyReleased, nativeCode );
		} break;
	}

	return ::DefWindowProc( Handle, Msg, WParam, LParam );
}

//////////////////////////////////////////////////////////////////////////

RubyWindowsBackend::RubyWindowsBackend( const RubyWindowSpecification& rSpec, RubyWindow* pWindow )
{
	m_pWindow = pWindow;
	m_WindowSpecification = rSpec;

	if( m_WindowSpecification.GraphicsAPI == RubyGraphicsAPI::OpenGL )
	{
		CreateDummyWindow(); 
	}
}

void RubyWindowsBackend::CreateDummyWindow()
{
	WNDCLASS DummyClass { .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC, .lpfnWndProc = DefWindowProcA, .hInstance = GetModuleHandle( NULL ), .lpszClassName = L"RubyDummyClass183613" };
	::RegisterClass( &DummyClass );

	HWND DummyWindow = ::CreateWindowEx( 0, L"RubyDummyClass183613", L"DymWind", 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle( NULL ), NULL );

	HDC DrawContext = ::GetDC( DummyWindow );

	PIXELFORMATDESCRIPTOR pfd
	{
		.nSize = sizeof( PIXELFORMATDESCRIPTOR ),
		.nVersion = 1,
		.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		.iPixelType = PFD_TYPE_RGBA,
		.cColorBits = 32,
		.cDepthBits = 24,
		.cStencilBits = 8,
		.iLayerType = PFD_MAIN_PLANE,
	};

	int PixelFormat = ::ChoosePixelFormat( DrawContext, &pfd );
	::SetPixelFormat( DrawContext, PixelFormat, &pfd );

	HGLRC Context = wglCreateContext( DrawContext );

	wglMakeCurrent( DrawContext, Context );

	wglCreateContextAttribsARB = ( wglCreateContextAttribsARBFn* )wglGetProcAddress( "wglCreateContextAttribsARB" );
	wglChoosePixelFormatARB = ( wglChoosePixelFormatARBFn* )wglGetProcAddress( "wglChoosePixelFormatARB" );

	wglMakeCurrent( DrawContext, 0 );
	wglDeleteContext( Context );

	ReleaseDC( DummyWindow, DrawContext );

	::DestroyWindow( DummyWindow );

	::UnregisterClass( L"RubyDummyClass183613", GetModuleHandle( 0 ) );
}

RubyWindowsBackend::~RubyWindowsBackend()
{
	DestroyWindow();
}

void RubyWindowsBackend::Create()
{
	DWORD WindowStyle = ChooseStyle();

	// Temp:
	// Ruby supports wstrings as titles however ImGui does not use them so will convert our title into a wstring.
	std::wstring name = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes( m_pWindow->m_WindowTitle.data() );

	m_Handle = ::CreateWindowEx( 0, DefaultClassName, name.data(), WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, ( int ) m_pWindow->GetWidth(), ( int ) m_pWindow->GetHeight(), NULL, NULL, GetModuleHandle( NULL ), NULL );

	::SetPropW( m_Handle, L"RubyData", this );

	CreateGraphics( m_pWindow->GetGraphicsAPI() );
}

DWORD RubyWindowsBackend::ChooseStyle()
{
	switch( m_WindowSpecification.Style )
	{
		case RubyStyle::Default:
			return WS_OVERLAPPEDWINDOW;
		case RubyStyle::Borderless:
			return WS_POPUP | WS_EX_TOPMOST;
		
		default:
			return 0;
	}
}

void RubyWindowsBackend::SetTitle( std::string_view Title )
{
	::SetWindowTextA( m_Handle, Title.data() );
}

void RubyWindowsBackend::Maximize()
{
	::ShowWindow( m_Handle, SW_MAXIMIZE );
}

void RubyWindowsBackend::Minimize()
{
	::ShowWindow( m_Handle, SW_MINIMIZE );
}

void RubyWindowsBackend::Restore()
{
	::ShowWindow( m_Handle, SW_RESTORE );
}

void* RubyWindowsBackend::GetNativeHandle()
{
	return m_Handle;
}

VkResult RubyWindowsBackend::CreateVulkanWindowSurface( VkInstance Instance, VkSurfaceKHR* pOutSurface )
{
	VkWin32SurfaceCreateInfoKHR CreateInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	CreateInfo.hinstance = GetModuleHandle( NULL );
	CreateInfo.hwnd = m_Handle;

	return vkCreateWin32SurfaceKHR( Instance, &CreateInfo, nullptr, pOutSurface );
}

void RubyWindowsBackend::CreateGraphics( RubyGraphicsAPI api )
{
	switch( api )
	{
		case RubyGraphicsAPI::OpenGL:
		{
			// Create Pixel Format Descriptor
			PIXELFORMATDESCRIPTOR pfd {};
			pfd = {
				sizeof( PIXELFORMATDESCRIPTOR ),
				1,                     
				PFD_DRAW_TO_WINDOW |  
				PFD_SUPPORT_OPENGL |
				PFD_DOUBLEBUFFER,   
				PFD_TYPE_RGBA,      
				24,                 
				0, 0, 0, 0, 0, 0,   
				0,                  
				0,                  
				0,                    
				0, 0, 0, 0,         
				32,                 
				0,                  
				0,                  
				PFD_MAIN_PLANE,     
				0,                  
				0, 0, 0             
			};

			m_DrawContent = ::GetDC( m_Handle );

			int PixelFormat;
			PixelFormat = ::ChoosePixelFormat( m_DrawContent, &pfd );
			::SetPixelFormat( m_DrawContent, PixelFormat, &pfd );

			if( wglCreateContextAttribsARB ) 
			{
				int attrib[] =
				{
					0x2091, 3,
					0x2092, 3,
					0x9126, 0x00000001, 
					0
				};

				m_OpenGLRenderContext = wglCreateContextAttribsARB( m_DrawContent, 0, attrib );

				if( m_OpenGLRenderContext )
					wglMakeCurrent( m_DrawContent, m_OpenGLRenderContext );
			}
			else 
			{
				m_OpenGLRenderContext = wglCreateContext( m_DrawContent );

				if( m_OpenGLRenderContext )
					wglMakeCurrent( m_DrawContent, m_OpenGLRenderContext );
			}

		}	break;
		
		case RubyGraphicsAPI::Vulkan: 
		{
			HDC DrawContext = ::GetDC( m_Handle );

			PIXELFORMATDESCRIPTOR pfd
			{
				.nSize = sizeof( PIXELFORMATDESCRIPTOR ),
				.nVersion = 1,
				.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER,
				.iPixelType = PFD_TYPE_RGBA,
				.cColorBits = 32,
				.cDepthBits = 24,
				.cStencilBits = 8,
				.iLayerType = PFD_MAIN_PLANE,
			};

			//int PixelFormat = ::ChoosePixelFormat( DrawContext, &pfd );
			//::SetPixelFormat( DrawContext, PixelFormat, &pfd );
		} break;

		case RubyGraphicsAPI::DirectX11:
		case RubyGraphicsAPI::DirectX12:
		case RubyGraphicsAPI::None:
		default:
			break;
	}
}

void RubyWindowsBackend::SetMousePos( double x, double y )
{
	POINT newPos { x, y };

	::SetCursorPos( x, y );
	::ScreenToClient( m_Handle, &newPos );
}

void RubyWindowsBackend::GetMousePos( double* x, double* y )
{
	POINT pos{};
	::GetCursorPos( &pos );
	::ScreenToClient( m_Handle, &pos );

	*x = pos.x;
	*y = pos.y;
}

void RubyWindowsBackend::IssueSwapBuffers()
{
	if( m_pWindow->GetGraphicsAPI() == RubyGraphicsAPI::OpenGL )
		::SwapBuffers( m_DrawContent );
}

void RubyWindowsBackend::DestroyWindow()
{
	if( m_Handle ) 
	{
		ReleaseDC( m_Handle, m_DrawContent );

		::DestroyWindow( m_Handle );
	}
}

void RubyWindowsBackend::CloseWindow()
{
	m_ShouldClose = true;
}

void RubyWindowsBackend::PresentWindow()
{
	::ShowWindow( m_Handle, SW_SHOW );
}

void RubyWindowsBackend::ResizeWindow( uint32_t Width, uint32_t Height )
{
	::MoveWindow( m_Handle, 0, 0, Width, Height, TRUE );
}

void RubyWindowsBackend::MoveWindow( int x, int y )
{
	::MoveWindow( m_Handle, x, y, m_pWindow->GetWidth(), m_pWindow->GetHeight(), TRUE );
}

void RubyWindowsBackend::PollEvents()
{
	MSG Message = {};
	while( ::PeekMessage( &Message, m_Handle, 0, 0, PM_REMOVE ) > 0 )
	{
		::TranslateMessage( &Message );
		::DispatchMessage( &Message );
	}
}

bool RubyWindowsBackend::PendingClose()
{
	return m_ShouldClose;
}