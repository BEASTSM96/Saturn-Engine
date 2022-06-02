/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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

#include "sppch.h"
#include "Styles.h"

#include <imgui.h>

namespace Saturn::Styles {

	void Dark()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = ImGui::GetStyle().Colors;

		// Inspired by My Own custom theme, Geno IDE's theme, and ImGui's own dark theme, and a mixture with some of Hazel's theme.
		colors[ ImGuiCol_Text ]                   = ImVec4( 1.000f, 1.000f, 1.000f, 1.000f );
		colors[ ImGuiCol_TextDisabled ]           = ImVec4( 0.500f, 0.500f, 0.500f, 1.000f );
		colors[ ImGuiCol_WindowBg ]               = ImVec4( ImColor( 36, 36, 36 ) );
		colors[ ImGuiCol_ChildBg ]                = colors[ ImGuiCol_WindowBg ];
		colors[ ImGuiCol_PopupBg ]				  = colors[ ImGuiCol_WindowBg ];
		colors[ ImGuiCol_Border ]                 = ImVec4( ImColor( 31, 31, 31 ) );
		colors[ ImGuiCol_BorderShadow ]           = ImVec4( 0.000f, 0.000f, 0.000f, 0.000f );
		colors[ ImGuiCol_FrameBg ]                = ImVec4( ImColor( 21, 21, 21 ) );
		colors[ ImGuiCol_FrameBgHovered ]         = ImVec4( 0.200f, 0.200f, 0.200f, 1.000f );
		colors[ ImGuiCol_FrameBgActive ]		  = colors[ ImGuiCol_FrameBg ];
		colors[ ImGuiCol_TitleBg ]                = ImVec4( ImColor( 21, 21, 21 ) );
		colors[ ImGuiCol_TitleBgActive ]          = colors[ ImGuiCol_FrameBg ];
		colors[ ImGuiCol_TitleBgCollapsed ]		  = colors[ ImGuiCol_FrameBg ];
		colors[ ImGuiCol_MenuBarBg ]			  = colors[ ImGuiCol_FrameBg ];
		colors[ ImGuiCol_ScrollbarBg ]            = ImVec4( 0.160f, 0.160f, 0.160f, 1.000f );
		colors[ ImGuiCol_ScrollbarGrab ]          = ImVec4( 0.277f, 0.277f, 0.277f, 1.000f );
		colors[ ImGuiCol_ScrollbarGrabHovered ]   = ImVec4( 0.300f, 0.300f, 0.300f, 1.000f );
		colors[ ImGuiCol_ScrollbarGrabActive ]    = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_CheckMark ]              = ImVec4( 1.000f, 1.000f, 1.000f, 1.000f );
		colors[ ImGuiCol_SliderGrab ]             = ImVec4( 0.391f, 0.391f, 0.391f, 1.000f );
		colors[ ImGuiCol_SliderGrabActive ]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_Button ]                 = ImVec4( ImColor( 58, 58, 58 ) );
		colors[ ImGuiCol_ButtonHovered ]          = ImVec4( 1.000f, 1.000f, 1.000f, 0.156f );
		colors[ ImGuiCol_ButtonActive ]           = ImVec4( 1.000f, 1.000f, 1.000f, 0.391f );
		colors[ ImGuiCol_Header ]                 = ImVec4( ImColor( 47, 47, 46 ) );
		colors[ ImGuiCol_HeaderHovered ]          = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
		colors[ ImGuiCol_HeaderActive ]           = colors[ ImGuiCol_Header ];
		colors[ ImGuiCol_Separator ]			  = ImVec4( ImColor( 27, 27, 27 ) );
		colors[ ImGuiCol_SeparatorHovered ]       = ImVec4( 0.391f, 0.391f, 0.391f, 1.000f );
		colors[ ImGuiCol_SeparatorActive ]        = ImVec4( ImColor( 79, 121, 141 ) );
		colors[ ImGuiCol_ResizeGrip ]             = ImVec4( 1.000f, 1.000f, 1.000f, 0.250f );
		colors[ ImGuiCol_ResizeGripHovered ]      = ImVec4( 1.000f, 1.000f, 1.000f, 0.670f );
		colors[ ImGuiCol_ResizeGripActive ]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_Tab ]                    = ImVec4( ImColor( 49, 45, 35 ) );
		colors[ ImGuiCol_TabHovered ]             = ImVec4( 0.352f, 0.352f, 0.352f, 1.000f );
		colors[ ImGuiCol_TabActive ]              = ImVec4( ImColor( 76, 69, 47 ) );
		colors[ ImGuiCol_TabUnfocused ]           = ImVec4( 0.098f, 0.098f, 0.098f, 1.000f );
		colors[ ImGuiCol_TabUnfocusedActive ]     = ImVec4( 0.195f, 0.195f, 0.195f, 1.000f );
		colors[ ImGuiCol_DockingPreview ]         = ImVec4( 1.000f, 0.391f, 0.000f, 0.781f );
		colors[ ImGuiCol_DockingEmptyBg ]         = ImVec4( 0.180f, 0.180f, 0.180f, 1.000f );
		colors[ ImGuiCol_PlotLines ]              = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
		colors[ ImGuiCol_PlotLinesHovered ]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_PlotHistogram ]          = ImVec4( 0.586f, 0.586f, 0.586f, 1.000f );
		colors[ ImGuiCol_PlotHistogramHovered ]   = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_TextSelectedBg ]         = ImVec4( 1.000f, 1.000f, 1.000f, 0.156f );
		colors[ ImGuiCol_DragDropTarget ]         = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_NavHighlight ]           = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_NavWindowingHighlight ]  = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		colors[ ImGuiCol_NavWindowingDimBg ]      = ImVec4( 0.000f, 0.000f, 0.000f, 0.586f );
		colors[ ImGuiCol_ModalWindowDimBg ]       = ImVec4( 0.000f, 0.000f, 0.000f, 0.586f );

		style.ChildRounding = 4.0f;
		style.FrameBorderSize = 1.0f;
		style.FrameRounding = 2.0f;
		style.GrabMinSize = 7.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 12.0f;
		style.ScrollbarSize = 13.0f;
		style.TabBorderSize = 1.0f;
		style.TabRounding = 0.0f;
		style.WindowRounding = 4.0f;
		style.TabRounding = 4.0f;
	}

}