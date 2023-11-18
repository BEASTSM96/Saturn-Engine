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
#include <windowsx.h>

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
		wc.hCursor = LoadCursor( nullptr, IDC_ARROW );
		wc.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

		::RegisterClass( &wc );
	}

	~RubyWindowRegister()
	{
		::UnregisterClass( DefaultClassName, GetModuleHandle( NULL ) );
	}
} static s_RubyWindowRegister;

//////////////////////////////////////////////////////////////////////////

int HandleKeyMods() 
{
	int Modifiers = RubyKey::UnknownKey;

	if( GetKeyState( VK_LSHIFT ) & 0x8000 )
	{
		Modifiers |= RubyKey::LeftShift;
	}

	if( GetKeyState( VK_RSHIFT ) & 0x8000 )
	{
		Modifiers |= RubyKey::RightShift;
	}

	if( GetKeyState( VK_LMENU ) & 0x8000 )
	{
		Modifiers |= RubyKey::LeftAlt;
	}

	if( GetKeyState( VK_RMENU ) & 0x8000 )
	{
		Modifiers |= RubyKey::RightAlt;
	}

	if( GetKeyState( VK_LCONTROL ) & 0x8000 )
	{
		Modifiers |= RubyKey::LeftCtrl;
	}

	if( GetKeyState( VK_RCONTROL ) & 0x8000 )
	{
		Modifiers |= RubyKey::RightCtrl;
	}

	return Modifiers;
}

LRESULT CALLBACK RubyWindowProc( HWND Handle, UINT Msg, WPARAM WParam, LPARAM LParam ) 
{
	RubyWindowsBackend* pThis = ( RubyWindowsBackend* ) ::GetPropW( Handle, L"RubyData" );

	if( !pThis )
		return ::DefWindowProc( Handle, Msg, WParam, LParam );

	switch( Msg )
	{
		case WM_QUIT: 
		case WM_CLOSE:
		{
			// Send a last minute event to tell the client that the window is about to close.
			pThis->GetParent()->DispatchEvent<RubyEvent>( RubyEventType::Close );

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

			pThis->GetParent()->SetSize( width, height );

			if( pThis->GetParent()->GetCursorMode() == RubyCursorMode::Locked ) 
			{
				pThis->ConfigureClipRect();
				pThis->RecenterMousePos();
			}

			if( WParam == SIZE_MAXIMIZED )
			{
				pThis->GetParent()->DispatchEvent<RubyMaximizeEvent>( RubyEventType::WindowMaximized, true );
			}
			else if( WParam == SIZE_MINIMIZED ) 
			{
				pThis->GetParent()->DispatchEvent<RubyMinimizeEvent>( RubyEventType::WindowMinimized, true );
			}

			pThis->GetParent()->DispatchEvent<RubyWindowResizeEvent>( RubyEventType::Resize, static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) );
		} break;

		//////////////////////////////////////////////////////////////////////////
		// Window Position & Focus

		case WM_WINDOWPOSCHANGING: 
		{
			pThis->GetParent()->DispatchEvent<RubyEvent>( RubyEventType::WindowMoved );

			if( pThis->GetParent()->GetCursorMode() == RubyCursorMode::Locked )
			{
				pThis->ConfigureClipRect();
				pThis->RecenterMousePos();
			}
		} break;

		//////////////////////////////////////////////////////////////////////////
		
		case WM_DISPLAYCHANGE: 
		{
			pThis->GetParent()->DispatchEvent<RubyEvent>( RubyEventType::DisplayChanged );
		} break;

		case WM_KILLFOCUS:
		case WM_SETFOCUS: 
		{
			pThis->GetParent()->DispatchEvent<RubyFocusEvent>( RubyEventType::WindowFocus, Msg == WM_SETFOCUS );
		} break;

		//////////////////////////////////////////////////////////////////////////
		// BEGIN: Mouse Events
		// Mouse Move

		case WM_MOUSEMOVE:
		{
			const int x = LOWORD( LParam );
			const int y = HIWORD( LParam );

			if( pThis->GetParent()->GetCursorMode() == RubyCursorMode::Locked )
			{
				RubyIVec2 lastPos = pThis->GetParent()->GetLastMousePos();
				
				RubyIVec2 deltaPos = { x - lastPos.x, y - lastPos.y };
				RubyIVec2 lockedDelta = pThis->GetParent()->GetVirtualMousePos();
				lockedDelta += deltaPos;
				
				pThis->GetParent()->DispatchEvent<RubyMouseMoveEvent>( 
					RubyEventType::MouseMoved, ( float ) lockedDelta.x, ( float ) lockedDelta.y );

				pThis->GetParent()->SetLockedMousePos( lockedDelta );
			}
			else
			{
				pThis->GetParent()->DispatchEvent<RubyMouseMoveEvent>( RubyEventType::MouseMoved, ( float ) x, ( float ) y );
			}
			
			pThis->GetParent()->SetLastMousePos( { x, y } );
		} return 0;

		case WM_SETCURSOR: 
		{
			if( LOWORD( LParam ) == HTCLIENT ) 
			{
				pThis->UpdateCursorIcon();
				return TRUE;
			}
		} break;

		//////////////////////////////////////////////////////////////////////////
		// Mouse Button Pressed & Released

		case WM_LBUTTONDOWN:
		{
			pThis->GetParent()->SetMouseDown( RubyMouseButton::Left );
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MousePressed, ( int ) RubyMouseButton::Left );
		} break;

		case WM_RBUTTONDOWN:
		{
			pThis->GetParent()->SetMouseDown( RubyMouseButton::Right );
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MousePressed, ( int ) RubyMouseButton::Right );
		} break;

		case WM_MBUTTONDOWN:
		{
			pThis->GetParent()->SetMouseDown( RubyMouseButton::Middle );
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MousePressed, ( int ) RubyMouseButton::Middle );
		} break;

		case WM_LBUTTONUP:
		{
			pThis->GetParent()->SetMouseDown( RubyMouseButton::Left, false );
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MouseReleased, ( int ) RubyMouseButton::Left );
		} break;

		case WM_RBUTTONUP:
		{
			pThis->GetParent()->SetMouseDown( RubyMouseButton::Right, false );
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MouseReleased, ( int ) RubyMouseButton::Right );
		} break;

		case WM_MBUTTONUP:
		{
			pThis->GetParent()->SetMouseDown( RubyMouseButton::Middle, false );
			pThis->GetParent()->DispatchEvent<RubyMouseEvent>( RubyEventType::MouseReleased, ( int ) RubyMouseButton::Middle );
		} break;

		case WM_MOUSEHOVER:
		{
			pThis->GetParent()->DispatchEvent<RubyEvent>( RubyEventType::MouseEnterWindow );
		} break;

		case WM_MOUSELEAVE:
		{
			pThis->GetParent()->DispatchEvent<RubyEvent>( RubyEventType::MouseLeaveWindow );
		} break;
		
		// Vertical Scroll
		case WM_MOUSEWHEEL:
		{
			int yOffset = GET_WHEEL_DELTA_WPARAM( WParam );
			yOffset /= WHEEL_DELTA;

			pThis->GetParent()->DispatchEvent<RubyMouseScrollEvent>( RubyEventType::MouseScroll, 0, yOffset );
		} break;

		// Horizontal Scroll
		case WM_MOUSEHWHEEL: 
		{
			int xOffset = GET_WHEEL_DELTA_WPARAM( WParam );
			xOffset /= WHEEL_DELTA;

			pThis->GetParent()->DispatchEvent<RubyMouseScrollEvent>( RubyEventType::MouseScroll, xOffset, 0 );
		} break;

		// END: Mouse Events
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Key Events

		case WM_KEYDOWN:
		{
			// In Ruby our key codes match with the Win32 ones.
			int nativeCode = ( int ) WParam;
			int Modifiers = HandleKeyMods();

			pThis->GetParent()->SetKeyDown( ( RubyKey ) nativeCode, true );
			pThis->GetParent()->DispatchEvent<RubyKeyEvent>( RubyEventType::KeyPressed, nativeCode, Modifiers );
		} break;

		case WM_KEYUP:
		{
			// In Ruby our key codes match with the Win32 ones.
			int nativeCode = ( int ) WParam;
			int Modifiers = HandleKeyMods();

			pThis->GetParent()->SetKeyDown( ( RubyKey ) nativeCode, false );
			pThis->GetParent()->DispatchEvent<RubyKeyEvent>( RubyEventType::KeyReleased, nativeCode, Modifiers );
		} break;

		// The WM_CHAR message is sent when a printable character key is pressed.
		case WM_CHAR: 
		{
			char c = static_cast< char >( WParam );
			pThis->GetParent()->DispatchEvent<RubyKeyEvent>( RubyEventType::InputCharacter, c, 0 );
		} break;

		//////////////////////////////////////////////////////////////////////////
		// Borderless Resizing support.
		// Thank You: https://github.com/Geno-IDE/Geno/blob/master/src/Geno/C%2B%2B/GUI/MainWindow.cpp#L520-L586

		case WM_NCHITTEST: 
		{
			if( pThis->GetParent()->GetStyle() != RubyStyle::Borderless || pThis->GetParent()->GetCursorMode() == RubyCursorMode::Locked )
				break;

			POINT MousePos;
			RECT WindowRect;

			::GetCursorPos( &MousePos );
			::GetWindowRect( Handle, &WindowRect );

			if( ::PtInRect( &WindowRect, MousePos ) )
			{
				const int BorderX = ::GetSystemMetrics( SM_CXFRAME ) + ::GetSystemMetrics( SM_CXPADDEDBORDER );
				const int BorderY = ::GetSystemMetrics( SM_CYFRAME ) + ::GetSystemMetrics( SM_CXPADDEDBORDER );

				// Top Section of the window
				if( MousePos.y < ( WindowRect.top + BorderY ) )
				{
					if( MousePos.x < ( WindowRect.left + BorderX ) ) { pThis->SetResizeCursor( RubyCursorType::ResizeNWSE ); return HTTOPLEFT; }
					else if( MousePos.x >= ( WindowRect.right - BorderX ) ) { pThis->SetResizeCursor( RubyCursorType::ResizeNESW ); return HTTOPRIGHT; }
					else { pThis->SetResizeCursor( RubyCursorType::ResizeNS ); return HTTOP; }
				}
				else if( MousePos.y >= ( WindowRect.bottom - BorderY ) ) // Bottom section of the window.
				{
					if( MousePos.x < ( WindowRect.left + BorderX ) ) { pThis->SetResizeCursor( RubyCursorType::ResizeNESW ); return HTBOTTOMLEFT; }
					else if( MousePos.x >= ( WindowRect.right - BorderX ) ) { pThis->SetResizeCursor( RubyCursorType::ResizeNWSE ); return HTBOTTOMRIGHT; }
					else { pThis->SetResizeCursor( RubyCursorType::ResizeNS ); return HTBOTTOM; }
				}
				else if( MousePos.x < ( WindowRect.left + BorderX ) ) // Left section of the window.
				{
					pThis->SetResizeCursor( RubyCursorType::ResizeEW );
					return HTLEFT;
				}
				else if( MousePos.x >= ( WindowRect.right - BorderX ) ) // Right section of the window.
				{
					pThis->SetResizeCursor( RubyCursorType::ResizeEW );
					return HTRIGHT;
				}
				else
				{
					// TODO: Caption.
					// TODO: Border height.
					if( MousePos.y < WindowRect.top + 16 )
						return HTCAPTION;
				}

				pThis->ResetResizeCursor();
			}
		} break;

		case WM_NCCALCSIZE:
		{
			if( WParam == TRUE )
			{
				WINDOWPLACEMENT WindowPlacement{ .length = sizeof( WINDOWPLACEMENT ) };

				if( ::GetWindowPlacement( Handle, &WindowPlacement ) && WindowPlacement.showCmd == SW_MAXIMIZE )
				{
					NCCALCSIZE_PARAMS& rParams = *reinterpret_cast< LPNCCALCSIZE_PARAMS >( LParam );
					const int BorderX = ::GetSystemMetrics( SM_CXFRAME ) + ::GetSystemMetrics( SM_CXPADDEDBORDER );
					const int BorderY = ::GetSystemMetrics( SM_CYFRAME ) + ::GetSystemMetrics( SM_CXPADDEDBORDER );

					rParams.rgrc[ 0 ].left += BorderX;
					rParams.rgrc[ 0 ].top += BorderY;
					rParams.rgrc[ 0 ].right -= BorderX;
					rParams.rgrc[ 0 ].bottom -= BorderY;

					return WVR_VALIDRECTS;
				}
			}

			return 0;

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

	::ReleaseDC( DummyWindow, DrawContext );

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
	// "codecvt_utf8" and "wstring_convert" are going to be removed in C++26
	std::wstring name = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes( m_pWindow->m_WindowTitle.data() );

	m_Handle = ::CreateWindowEx( 0, DefaultClassName, name.data(), WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, ( int ) m_pWindow->GetWidth(), ( int ) m_pWindow->GetHeight(), NULL, NULL, GetModuleHandle( NULL ), NULL );

	::SetPropW( m_Handle, L"RubyData", this );

	if( m_WindowSpecification.Style == RubyStyle::Borderless )
		::SetWindowLong( m_Handle, GWL_STYLE, GetWindowLong( m_Handle, GWL_STYLE ) | WS_CAPTION );

	CreateGraphics( m_pWindow->GetGraphicsAPI() );
}

DWORD RubyWindowsBackend::ChooseStyle()
{
	switch( m_WindowSpecification.Style )
	{
		case RubyStyle::Default:
			return WS_OVERLAPPEDWINDOW;
		case RubyStyle::Borderless:
			// Create the borderless window as a normal window however we will then set the required styles.
			// TODO: For some reason adding WS_CAPTION does not work, so we'll add it when set the style long.
			return WS_POPUP | WS_EX_TOPMOST | WS_MAXIMIZEBOX;
		
		default:
			return 0;
	}
}

LPTSTR RubyWindowsBackend::ChooseCursor( RubyCursorType Cursor )
{
	switch( Cursor )
	{
		case RubyCursorType::Arrow:
			return IDC_ARROW;
		
		case RubyCursorType::Hand:
			return IDC_HAND;
		
		case RubyCursorType::IBeam:
			return IDC_IBEAM;

		case RubyCursorType::NotAllowed:
			return IDC_NO;

		case RubyCursorType::ResizeEW:
			return IDC_SIZEWE;

		case RubyCursorType::ResizeNS:
			return IDC_SIZENS;

		case RubyCursorType::ResizeNESW:
			return IDC_SIZENESW;

		case RubyCursorType::ResizeNWSE:
			return IDC_SIZENWSE;
	}

	return nullptr;
}

void RubyWindowsBackend::FindMouseRestorePoint()
{
	if( m_pWindow->GetLastCursorMode() < RubyCursorMode::Locked )
	{
		POINT pos{};
		::GetCursorPos( &pos );
		::ScreenToClient( m_Handle, &pos );

		m_MouseRestorePoint.x = pos.x;
		m_MouseRestorePoint.y = pos.y;
	}
}

void RubyWindowsBackend::SetResizeCursor( RubyCursorType Type )
{
	// Safety
	UnblockMouseCursor();

	SetMouseCursor( Type );

	BlockMouseCursor();
}

void RubyWindowsBackend::ResetResizeCursor()
{
	UnblockMouseCursor();

	// TODO: What if we did not have the arrow before?
	SetMouseCursor( RubyCursorType::Arrow );
}

void RubyWindowsBackend::ConfigureClipRect() 
{
	RECT WindowRect;
	::GetClientRect( m_Handle, &WindowRect );
	::ClientToScreen( m_Handle, ( POINT* ) &WindowRect.left );
	::ClientToScreen( m_Handle, ( POINT* ) &WindowRect.right );

	::ClipCursor( &WindowRect );
}

void RubyWindowsBackend::RecenterMousePos() 
{
	SetMousePos( m_pWindow->GetWidth() / 2.0, m_pWindow->GetHeight() / 2.0 );
}

void RubyWindowsBackend::DisableCursor()
{
	FindMouseRestorePoint();

	UpdateCursorIcon();

	// Keep the mouse in the center of the window so we don't move out of the window.
	SetMousePos( m_pWindow->GetWidth() / 2.0, m_pWindow->GetHeight() / 2.0 );

	ConfigureClipRect();
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

bool RubyWindowsBackend::Minimized()
{
	return ::IsIconic( m_Handle );
}

bool RubyWindowsBackend::Maximized()
{
	return ::IsZoomed( m_Handle );
}

bool RubyWindowsBackend::Focused()
{
	return ::GetActiveWindow() == m_Handle;
}

VkResult RubyWindowsBackend::CreateVulkanWindowSurface( VkInstance Instance, VkSurfaceKHR* pOutSurface )
{
	VkWin32SurfaceCreateInfoKHR CreateInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	CreateInfo.hinstance = GetModuleHandle( NULL );
	CreateInfo.hwnd = m_Handle;

	return vkCreateWin32SurfaceKHR( Instance, &CreateInfo, nullptr, pOutSurface );
}

void RubyWindowsBackend::SetMouseCursor( RubyCursorType Cursor )
{
	if( m_BlockMouseCursor )
		return;

	LPTSTR NativeCursorRes = ChooseCursor( Cursor );

	m_CurrentMouseCursorIcon = ::LoadCursor( nullptr, NativeCursorRes );
	
	UpdateCursorIcon();
}

void RubyWindowsBackend::UpdateCursorIcon() 
{
	if( m_pWindow->GetCursorMode() == RubyCursorMode::Locked )
	{
		::SetCursor( NULL );
		m_CurrentMouseCursorIcon = nullptr;
	}
	else if( m_CurrentMouseCursorIcon )
		::SetCursor( m_CurrentMouseCursorIcon );
	else
		m_CurrentMouseCursorIcon = ::SetCursor( ::LoadCursor( nullptr, IDC_ARROW ) );
}

void RubyWindowsBackend::SetMouseCursorMode( RubyCursorMode mode )
{
	switch( mode )
	{
		case RubyCursorMode::Normal: 
		{
			if( m_pWindow->GetLastCursorMode() == RubyCursorMode::Locked )
			{
				// Unclip the mouse
				::ClipCursor( nullptr );

				SetMousePos( m_MouseRestorePoint.x, m_MouseRestorePoint.y );

				// Rest the restore point and the locked mouse position.
				m_MouseRestorePoint = {};
				m_pWindow->m_LockedMousePosition = {};
			}

			::ShowCursor( TRUE );
			SetMouseCursor( RubyCursorType::Arrow );

		} break;
		
		case RubyCursorMode::Hidden: 
		{
			::ClipCursor( nullptr );
			::ShowCursor( FALSE );
		} break;
		
		case RubyCursorMode::Locked:
		{
			if( !Focused() )
				break;

			DisableCursor();
		} break;
	
		default:
			break;
	}
}

void RubyWindowsBackend::CreateGraphics( RubyGraphicsAPI api )
{
	switch( api )
	{
		case RubyGraphicsAPI::OpenGL:
		{
			// TODO: Maybe 32 bits for depth.

			// Create Pixel Format Descriptor
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

			m_DrawContent = ::GetDC( m_Handle );

			int PixelFormat;
			PixelFormat = ::ChoosePixelFormat( m_DrawContent, &pfd );
			::SetPixelFormat( m_DrawContent, PixelFormat, &pfd );

			if( wglCreateContextAttribsARB ) 
			{
				// TODO: Allow for the spec to set the OpenGL version.
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
		case RubyGraphicsAPI::DirectX11:
		case RubyGraphicsAPI::DirectX12:
		case RubyGraphicsAPI::None:
		default:
			break;
	}
}

void RubyWindowsBackend::SetMousePos( double x, double y )
{
	m_pWindow->SetLastMousePos( { ( int ) x, ( int ) y } );

	POINT newPos{ ( int ) x, ( int ) y };

	::ClientToScreen( m_Handle, &newPos );
	::SetCursorPos( newPos.x, newPos.y );
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
	// TODO: I don't want to just update our window. 
	// We should update all windows all at once.
	
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

void RubyWindowsBackend::Focus()
{
	::BringWindowToTop( m_Handle );
	::SetForegroundWindow( m_Handle );
	::SetFocus( m_Handle );
}

RubyIVec2 RubyWindowsBackend::GetWindowPos()
{
	RECT WindowRect;
	::GetWindowRect( m_Handle, &WindowRect );

	::MapWindowPoints( HWND_DESKTOP, ::GetParent( m_Handle ), (LPPOINT)&WindowRect, 2 );

	return { WindowRect.left, WindowRect.top };
}
