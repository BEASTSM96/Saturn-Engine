// dear imgui: Platform Backend for Ruby
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan, WebGPU..)

#pragma once

#include "imgui.h"      // IMGUI_IMPL_API

class RubyWindow;

IMGUI_IMPL_API bool     ImGui_ImplRuby_InitForOpenGL( RubyWindow* pWindow );
IMGUI_IMPL_API bool     ImGui_ImplRuby_InitForVulkan( RubyWindow* pWindow );
IMGUI_IMPL_API bool     ImGui_ImplRuby_InitForOther( RubyWindow* pWindow );
IMGUI_IMPL_API void     ImGui_ImplRuby_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplRuby_NewFrame();

// Ruby callbacks
IMGUI_IMPL_API void     ImGui_ImplGlfw_WindowFocusCallback( RubyWindow* window, int focused);
IMGUI_IMPL_API void     ImGui_ImplGlfw_CursorEnterCallback( RubyWindow* window, int entered);
IMGUI_IMPL_API bool     ImGui_ImplRuby_MouseButtonCallback( RubyWindow* window, int button, bool state );
IMGUI_IMPL_API void     ImGui_ImplGlfw_ScrollCallback( RubyWindow* window, double xoffset, double yoffset);
IMGUI_IMPL_API void     ImGui_ImplGlfw_KeyCallback( RubyWindow* window, int key, int scancode, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_CharCallback( RubyWindow* window, unsigned int c);
