// dear imgui: Platform Backend for Ruby
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)

#include "imgui.h"
#include "imgui_impl_ruby.h"

// Ruby
#include <Ruby/RubyWindow.h>
#include <Ruby/RubyMonitor.h>

enum class RubyClientAPI
{
    Unknown,
    OpenGL,
    Vulkan
};

class ImGui_ImplRuby_EventHandler;
struct ImGui_ImplRuby_Data
{
    RubyWindow*             Window;
    RubyClientAPI           ClientApi;
    double                  Time;
    RubyWindow*             MouseWindow;
    bool                    MouseJustPressed[ImGuiMouseButton_COUNT];
    RubyWindow*             KeyOwnerWindows[512];
    bool                    InstalledCallbacks;
    bool                    WantUpdateMonitors;
    ImGui_ImplRuby_EventHandler* EventHandler;

    ImGui_ImplRuby_Data()   { memset(this, 0, sizeof(*this)); }
};

class ImGui_ImplRuby_EventHandler : public RubyEventTarget
{
public:
    ImGui_ImplRuby_EventHandler() {}

    ~ImGui_ImplRuby_EventHandler() 
    {
        PrevUserEventTarget = nullptr;
        BackendData = nullptr;
    }

	bool OnEvent( RubyEvent& rEvent ) override
	{
		// Call the users event target first.
        bool handled = false;
        
        if( PrevUserEventTarget )
            handled = PrevUserEventTarget->OnEvent( rEvent );

		switch( rEvent.Type )
		{
			case RubyEventType::MouseReleased:
			{
				RubyMouseEvent MouseEvent = ( RubyMouseEvent& ) rEvent;
				ImGui_ImplRuby_MouseButtonCallback( BackendData->Window, MouseEvent.GetButton(), false );
			} break;

			case RubyEventType::MousePressed:
			{
				RubyMouseEvent MouseEvent = ( RubyMouseEvent& ) rEvent;
				ImGui_ImplRuby_MouseButtonCallback( BackendData->Window, MouseEvent.GetButton(), true );
			} break;

			case RubyEventType::KeyPressed:
			{
				RubyKeyEvent KeyEvent = ( RubyKeyEvent& ) rEvent;
                ImGui_ImplRuby_KeyCallback( BackendData->Window, KeyEvent.GetScancode(), true, 0 );
			} break;

			case RubyEventType::KeyReleased:
			{
				RubyKeyEvent KeyEvent = ( RubyKeyEvent& ) rEvent;
                ImGui_ImplRuby_KeyCallback( BackendData->Window, KeyEvent.GetScancode(), false, 0 );
			} break;

			case RubyEventType::MouseEnterWindow:
			{
                ImGui_ImplRuby_MouseHoverWindowCallback( true );
			} break;

			case RubyEventType::MouseLeaveWindow:
			{
                ImGui_ImplRuby_MouseHoverWindowCallback( false );
			} break;
		}

		return handled;
	}

public:
    RubyEventTarget* PrevUserEventTarget = nullptr;
    ImGui_ImplRuby_Data* BackendData = nullptr;
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplRuby_Data* ImGui_ImplRuby_GetBackendData()
{
    return ImGui::GetCurrentContext() ? ( ImGui_ImplRuby_Data* )ImGui::GetIO().BackendPlatformUserData : NULL;
}

// Forward Declarations
static void ImGui_ImplRuby_UpdateMonitors();
static void ImGui_ImplRuby_InitPlatformInterface();
static void ImGui_ImplRuby_ShutdownPlatformInterface();

// Functions
static const char* ImGui_ImplRuby_GetClipboardText(void* user_data)
{
    return "";
}

static void ImGui_ImplRuby_SetClipboardText(void* user_data, const char* text)
{
}

void ImGui_ImplRuby_MouseButtonCallback(RubyWindow* window, int button, bool state)
{
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();

    if( state && button >= 0 && button < IM_ARRAYSIZE( bd->MouseJustPressed ) )
        bd->MouseJustPressed[ button ] = true;
}

void ImGui_ImplRuby_ScrollCallback( RubyWindow* window, double xoffset, double yoffset)
{
}

void ImGui_ImplRuby_KeyCallback( RubyWindow* window, int scancode, bool state, int mods)
{
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();

    ImGuiIO& io = ImGui::GetIO();

	if( scancode >= 0 && scancode < IM_ARRAYSIZE( io.KeysDown ) )
	{
		if( state )
		{
			io.KeysDown[ scancode ] = true;
			bd->KeyOwnerWindows[ scancode ] = window;
		}
        else
		{
			io.KeysDown[ scancode ] = false;
			bd->KeyOwnerWindows[ scancode ] = NULL;
		}
	}

#ifdef _WIN32
	io.KeySuper = false;
#else
    // TODO:
#endif

}

void ImGui_ImplRuby_MouseHoverWindowCallback( bool state )
{
    if( !state )
        return;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	ImGuiIO& io = ImGui::GetIO();

    for( int n = 0; n < platform_io.Viewports.Size; n++ )
    {
        ImGuiViewport* viewport = platform_io.Viewports[ n ];
        io.MouseHoveredViewport = viewport->ID;
    }
}

void ImGui_ImplRuby_WindowFocusCallback( RubyWindow* window, int focused)
{
}

void ImGui_ImplRuby_CursorEnterCallback( RubyWindow* window, int entered)
{
}

void ImGui_ImplRuby_CharCallback( RubyWindow* window, unsigned int c)
{
}

static bool ImGui_ImplRuby_Init( RubyWindow* window, bool install_callbacks, RubyClientAPI client_api)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplRuby_Data* bd = IM_NEW(ImGui_ImplRuby_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_ruby";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
#if defined(_WIN32)
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
#endif

    bd->Window = window;
    bd->Time = 0.0;
    bd->WantUpdateMonitors = true;

    bd->EventHandler = IM_NEW( ImGui_ImplRuby_EventHandler )();
    bd->EventHandler->BackendData = bd;
    bd->EventHandler->PrevUserEventTarget = bd->Window->GetEventTarget();

    bd->Window->SetEventTarget( bd->EventHandler );

    // Keyboard mapping. Dear ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab]         = (int)RubyKey::Tab;
    io.KeyMap[ImGuiKey_LeftArrow]   = (int)RubyKey::LeftArrow;
    io.KeyMap[ImGuiKey_RightArrow]  = (int)RubyKey::RightArrow;
    io.KeyMap[ImGuiKey_UpArrow]     = (int)RubyKey::UpArrow;
    io.KeyMap[ImGuiKey_DownArrow]   = (int)RubyKey::DownArrow;
    io.KeyMap[ImGuiKey_PageUp]      = (int)RubyKey::PageUp;
    io.KeyMap[ImGuiKey_PageDown]    = (int)RubyKey::PageDown;
    io.KeyMap[ImGuiKey_Home]        = (int)RubyKey::Home;
    io.KeyMap[ImGuiKey_End]         = (int)RubyKey::End;
    io.KeyMap[ImGuiKey_Insert]      = (int)RubyKey::Insert;
    io.KeyMap[ImGuiKey_Delete]      = (int)RubyKey::Delete;
    io.KeyMap[ImGuiKey_Backspace]   = (int)RubyKey::Backspace;
    io.KeyMap[ImGuiKey_Space]       = (int)RubyKey::Space; 
    io.KeyMap[ImGuiKey_Enter]       = (int)RubyKey::Enter;
    io.KeyMap[ImGuiKey_Escape]      = (int)RubyKey::Esc;
    io.KeyMap[ImGuiKey_KeyPadEnter] = (int)RubyKey::NumpadEnter;
    io.KeyMap[ImGuiKey_A]           = (int)RubyKey::A;
    io.KeyMap[ImGuiKey_C]           = (int)RubyKey::C;
    io.KeyMap[ImGuiKey_V]           = (int)RubyKey::V;
    io.KeyMap[ImGuiKey_X]           = (int)RubyKey::X;
    io.KeyMap[ImGuiKey_Y]           = (int)RubyKey::Y;
    io.KeyMap[ImGuiKey_Z]           = (int)RubyKey::Z;

    io.SetClipboardTextFn = ImGui_ImplRuby_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplRuby_GetClipboardText;
    io.ClipboardUserData = bd->Window;

    ImGui_ImplRuby_UpdateMonitors();

    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void*)bd->Window;
#ifdef _WIN32
    main_viewport->PlatformHandleRaw = (HWND)bd->Window->GetNativeHandle();
#endif
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui_ImplRuby_InitPlatformInterface();

    bd->ClientApi = client_api;
    return true;
}

bool ImGui_ImplRuby_InitForOpenGL( RubyWindow* window )
{
    return ImGui_ImplRuby_Init(window, false, RubyClientAPI::OpenGL );
}

bool ImGui_ImplRuby_InitForVulkan( RubyWindow* window )
{
    return ImGui_ImplRuby_Init(window, false, RubyClientAPI::Vulkan );
}

bool ImGui_ImplRuby_InitForOther( RubyWindow* window )
{
    return ImGui_ImplRuby_Init(window, false, RubyClientAPI::Unknown );
}

void ImGui_ImplRuby_Shutdown()
{
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();
    IM_ASSERT(bd != NULL && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplRuby_ShutdownPlatformInterface();

    bd->Window->SetEventTarget( bd->EventHandler->PrevUserEventTarget );
    delete bd->EventHandler;

    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;
    IM_DELETE(bd);
}

static void ImGui_ImplRuby_UpdateMousePosAndButtons()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

	const ImVec2 mouse_pos_prev = io.MousePos;
	io.MousePos = ImVec2( -FLT_MAX, -FLT_MAX );
	io.MouseHoveredViewport = 0;

	for( int i = 0; i < IM_ARRAYSIZE( io.MouseDown ); i++ )
	{
        // TODO: Ruby input
		io.MouseDown[ i ] = bd->MouseJustPressed[ i ];
		bd->MouseJustPressed[ i ] = false;
	}

    for( int n = 0; n < platform_io.Viewports.Size; n++ )
    {
		ImGuiViewport* viewport = platform_io.Viewports[ n ];
		RubyWindow* window = ( RubyWindow* ) viewport->PlatformHandle;

        RubyWindow* mouse_window = ( bd->MouseWindow == window || window->IsFocused() ) ? window : NULL;

        if( io.WantSetMousePos && window->IsFocused() ) 
        {
            window->SetMousePos( (double) mouse_pos_prev.x - viewport->Pos.x, (double) mouse_pos_prev.y - viewport->Pos.y );
        }

		if( mouse_window )
		{
            double x, y;
            mouse_window->GetMousePos( &x, &y );

            if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
            {
                RubyIVec2 pos = mouse_window->GetPosition();

                io.MousePos = ImVec2( static_cast< float >( x + pos.x ), static_cast< float >( y + pos.y ) );
            }
            else
            {
                io.MousePos = ImVec2( static_cast< float >( x ), static_cast< float >( y ) );
            }
		}
    }
}

static void ImGui_ImplRuby_UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();

	if( ( io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange ) )
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

	for( int n = 0; n < platform_io.Viewports.Size; n++ )
	{
		RubyWindow* window = ( RubyWindow* ) platform_io.Viewports[ n ]->PlatformHandle;

		if( imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor )
		{
			// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            // TODO: Set Mouse cursors.
		}
		else
		{
			// Show OS mouse cursor
            // TODO: Set Mouse cursors.
		}
	}
}

static void ImGui_ImplRuby_UpdateGamepads()
{
}

static void ImGui_ImplRuby_UpdateMonitors()
{
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();

    std::vector<RubyMonitor> monitors = RubyGetAllMonitors();

    for( const auto& rMonitor : monitors )
    {
        ImGuiPlatformMonitor monitor;

        monitor.MainSize = ImVec2( rMonitor.MonitorSize.x, rMonitor.MonitorSize.x );
        monitor.WorkSize = ImVec2( rMonitor.WorkSize.x, rMonitor.WorkSize.y );

        monitor.WorkPos = ImVec2( rMonitor.WorkSize.x, rMonitor.WorkSize.y );
        monitor.MainPos = ImVec2( rMonitor.WorkSize.x, rMonitor.WorkSize.y );

        platform_io.Monitors.push_back( monitor );
    }
    bd->WantUpdateMonitors = false;
}

void ImGui_ImplRuby_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplRuby_InitForXXX()?");

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;

    w = bd->Window->GetWidth();
    h = bd->Window->GetHeight();
    display_w = w;
    display_h = h;

    io.DisplaySize = ImVec2( (float)w, (float)h );
   
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
    
    if (bd->WantUpdateMonitors)
        ImGui_ImplRuby_UpdateMonitors();

    // Setup time step
    double current_time = 0.0f;
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    ImGui_ImplRuby_UpdateEvents();
    ImGui_ImplRuby_UpdateMousePosAndButtons();
    ImGui_ImplRuby_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    ImGui_ImplRuby_UpdateGamepads();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

// Helper structure we store in the void* RenderUserData field of each ImGuiViewport to easily retrieve our backend data.
struct ImGui_ImplRuby_ViewportData
{
    RubyWindow* Window;
    bool        WindowOwned;
    int         IgnoreWindowPosEventFrame;
    int         IgnoreWindowSizeEventFrame;

    ImGui_ImplRuby_ViewportData()  { Window = NULL; WindowOwned = false; IgnoreWindowSizeEventFrame = IgnoreWindowPosEventFrame = -1; }
    ~ImGui_ImplRuby_ViewportData() { IM_ASSERT(Window == NULL); }
};

static void ImGui_ImplRuby_WindowCloseCallback( RubyWindow* window )
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
        viewport->PlatformRequestClose = true;
}

// GLFW may dispatch window pos/size events after calling glfwSetWindowPos()/glfwSetWindowSize().
// However: depending on the platform the callback may be invoked at different time:
// - on Windows it appears to be called within the glfwSetWindowPos()/glfwSetWindowSize() call
// - on Linux it is queued and invoked during glfwPollEvents()
// Because the event doesn't always fire on glfwSetWindowXXX() we use a frame counter tag to only
// ignore recent glfwSetWindowXXX() calls.
static void ImGui_ImplRuby_WindowPosCallback( RubyWindow* window, int, int)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
    {
        if (ImGui_ImplRuby_ViewportData* vd = (ImGui_ImplRuby_ViewportData*)viewport->PlatformUserData)
        {
            bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowPosEventFrame + 1);
            //data->IgnoreWindowPosEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestMove = true;
    }
}

static void ImGui_ImplRuby_WindowSizeCallback( RubyWindow* window, int, int)
{
    if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
    {
        if (ImGui_ImplRuby_ViewportData* vd = (ImGui_ImplRuby_ViewportData*)viewport->PlatformUserData)
        {
            bool ignore_event = (ImGui::GetFrameCount() <= vd->IgnoreWindowSizeEventFrame + 1);
            //data->IgnoreWindowSizeEventFrame = -1;
            if (ignore_event)
                return;
        }
        viewport->PlatformRequestResize = true;
    }
}

static void ImGui_ImplRuby_CreateWindow(ImGuiViewport* viewport)
{
	ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();
    ImGui_ImplRuby_ViewportData* vd = IM_NEW( ImGui_ImplRuby_ViewportData )( );
	viewport->PlatformUserData = vd;

    RubyWindowSpecification spec;
    spec.Style = ( viewport->Flags & ImGuiViewportFlags_NoDecoration ) ? RubyStyle::Borderless : RubyStyle::Default;
    spec.GraphicsAPI = (RubyGraphicsAPI) bd->ClientApi;
    spec.Name = "No Name Yet.";
    spec.ShowNow = false;
    spec.Width = viewport->Size.x;
    spec.Height = viewport->Size.y;

    vd->Window = new RubyWindow( spec );
    vd->Window->SetEventTarget( bd->EventHandler );
    vd->WindowOwned = true;

	viewport->PlatformHandleRaw = vd->Window->GetNativeHandle();

#ifdef _WIN32
    viewport->PlatformHandle = (void*)vd->Window;
#endif

    vd->Window->SetPosition( ( int ) viewport->Pos.x, ( int ) viewport->Pos.y );
}

static void ImGui_ImplRuby_DestroyWindow(ImGuiViewport* viewport)
{
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();

    if( ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData )
    {
        if( vd->WindowOwned )
        {
           delete vd->Window;
        }

        vd->Window = NULL;
        IM_DELETE( vd );
    }
    viewport->PlatformUserData = viewport->PlatformHandle = NULL;
}

static void ImGui_ImplRuby_ShowWindow(ImGuiViewport* viewport)
{
    ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    // Temp: Ruby does not support not showing a icon on the task bar so we will ask win32 to do it for us
#if defined( _WIN32 )
    HWND hwnd = ( HWND ) viewport->PlatformHandleRaw;

    if( viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon )
    {
        LONG Style = ::GetWindowLong( hwnd, GWL_EXSTYLE );
        Style &= ~( WS_EX_APPWINDOW );
        Style |= WS_EX_TOOLWINDOW;

        ::SetWindowLong( hwnd, GWL_EXSTYLE, Style );
    }

#endif

    vd->Window->Show();
}

void ImGui_ImplRuby_UpdateEvents()
{
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

	for( int n = 0; n < platform_io.Viewports.Size; n++ )
	{
		ImGuiViewport* viewport = platform_io.Viewports[ n ];
		ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

		RubyWindow* window = ( RubyWindow* ) vd->Window;

		if( window && vd->WindowOwned )
			window->PollEvents();
	}
}

static ImVec2 ImGui_ImplRuby_GetWindowPos(ImGuiViewport* viewport)
{
    ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    return { ( float ) vd->Window->GetPosition().x, ( float ) vd->Window->GetPosition().y };
}

static void ImGui_ImplRuby_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
    ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    vd->Window->SetPosition( pos.x, pos.y );
}

static ImVec2 ImGui_ImplRuby_GetWindowSize(ImGuiViewport* viewport)
{
	ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    return { ( float ) vd->Window->GetWidth(), ( float ) vd->Window->GetHeight() };
}

static void ImGui_ImplRuby_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    vd->Window->Resize( size.x, size.y );
}

static void ImGui_ImplRuby_SetWindowTitle(ImGuiViewport* viewport, const char* title)
{
    ImGui_ImplRuby_ViewportData* vd = (ImGui_ImplRuby_ViewportData*)viewport->PlatformUserData;

   vd->Window->ChangeTitle( title );
}

static void ImGui_ImplRuby_SetWindowFocus(ImGuiViewport* viewport)
{
#if GLFW_HAS_FOCUS_WINDOW
    ImGui_ImplRuby_ViewportData* vd = (ImGui_ImplRuby_ViewportData*)viewport->PlatformUserData;
    glfwFocusWindow(vd->Window);
#else
    // FIXME: What are the effect of not having this function? At the moment imgui doesn't actually call SetWindowFocus - we set that up ahead, will answer that question later.
    (void)viewport;
#endif
}

static bool ImGui_ImplRuby_GetWindowFocus(ImGuiViewport* viewport)
{
    ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    return vd->Window->IsFocused();
}

static bool ImGui_ImplRuby_GetWindowMinimized(ImGuiViewport* viewport)
{
    ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    return vd->Window->Minimized();
}

static void ImGui_ImplRuby_RenderWindow(ImGuiViewport* viewport, void*)
{
}

static void ImGui_ImplRuby_SwapBuffers(ImGuiViewport* viewport, void*)
{
    ImGui_ImplRuby_ViewportData* vd = ( ImGui_ImplRuby_ViewportData* ) viewport->PlatformUserData;

    vd->Window->GLSwapBuffers();
}

//--------------------------------------------------------------------------------------------------------
// IME (Input Method Editor) basic support for e.g. Asian language users
//--------------------------------------------------------------------------------------------------------

// We provide a Win32 implementation because this is such a common issue for IME users
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS)
#define HAS_WIN32_IME   1
#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif
static void ImGui_ImplWin32_SetImeInputPos(ImGuiViewport* viewport, ImVec2 pos)
{
    COMPOSITIONFORM cf = { CFS_FORCE_POSITION, { (LONG)(pos.x - viewport->Pos.x), (LONG)(pos.y - viewport->Pos.y) }, { 0, 0, 0, 0 } };
    if (HWND hwnd = (HWND)viewport->PlatformHandleRaw)
        if (HIMC himc = ::ImmGetContext(hwnd))
        {
            ::ImmSetCompositionWindow(himc, &cf);
            ::ImmReleaseContext(hwnd, himc);
        }
}
#else
#define HAS_WIN32_IME   0
#endif

//--------------------------------------------------------------------------------------------------------
// Vulkan support (the Vulkan renderer needs to call a platform-side support function to create the surface)
//--------------------------------------------------------------------------------------------------------

#ifndef VULKAN_H_
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
struct VkAllocationCallbacks;
enum VkResult { VK_RESULT_MAX_ENUM = 0x7FFFFFFF };
#endif // VULKAN_H_

static int ImGui_ImplRuby_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();
    ImGui_ImplRuby_ViewportData* vd = (ImGui_ImplRuby_ViewportData*)viewport->PlatformUserData;

    IM_UNUSED(bd);
    IM_ASSERT(bd->ClientApi == RubyClientAPI::Vulkan);

    VkResult err = vd->Window->CreateVulkanWindowSurface( ( VkInstance ) vk_instance, ( VkSurfaceKHR* ) out_vk_surface );

    return (int)err;
}

static void ImGui_ImplRuby_InitPlatformInterface()
{
    // Register platform interface (will be coupled with a renderer interface)
    ImGui_ImplRuby_Data* bd = ImGui_ImplRuby_GetBackendData();
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_CreateWindow = ImGui_ImplRuby_CreateWindow;
    platform_io.Platform_DestroyWindow = ImGui_ImplRuby_DestroyWindow;
    platform_io.Platform_ShowWindow = ImGui_ImplRuby_ShowWindow;
    platform_io.Platform_SetWindowPos = ImGui_ImplRuby_SetWindowPos;
    platform_io.Platform_GetWindowPos = ImGui_ImplRuby_GetWindowPos;
    platform_io.Platform_SetWindowSize = ImGui_ImplRuby_SetWindowSize;
    platform_io.Platform_GetWindowSize = ImGui_ImplRuby_GetWindowSize;
    platform_io.Platform_SetWindowFocus = ImGui_ImplRuby_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = ImGui_ImplRuby_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = ImGui_ImplRuby_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = ImGui_ImplRuby_SetWindowTitle;
    platform_io.Platform_RenderWindow = ImGui_ImplRuby_RenderWindow;
    platform_io.Platform_SwapBuffers = ImGui_ImplRuby_SwapBuffers;
    platform_io.Platform_CreateVkSurface = ImGui_ImplRuby_CreateVkSurface;

    // Register main window handle (which is owned by the main application, not by us)
    // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same logic for main and secondary viewports.
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui_ImplRuby_ViewportData* vd = IM_NEW(ImGui_ImplRuby_ViewportData)();
    vd->Window = bd->Window;
    vd->WindowOwned = false;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (void*)bd->Window;
}

static void ImGui_ImplRuby_ShutdownPlatformInterface()
{
    ImGui::DestroyPlatformWindows();
}
