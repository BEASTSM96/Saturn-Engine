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
		
		colors[ ImGuiCol_WindowBg ]               = ImGui::ColorConvertU32ToFloat4( IM_COL32( 21, 21, 21, 255 ) );
		colors[ ImGuiCol_WindowBg ]               = ImGui::ColorConvertU32ToFloat4( IM_COL32( 26, 26, 26, 255 ) );
		colors[ ImGuiCol_ChildBg ]                = ImGui::ColorConvertU32ToFloat4( IM_COL32( 34, 34, 34, 255 ) );
		colors[ ImGuiCol_PopupBg ]				  = colors[ ImGuiCol_ChildBg ];
		colors[ ImGuiCol_Border ]                 = ImGui::ColorConvertU32ToFloat4( IM_COL32( 31, 31, 31, 255 ) );
		colors[ ImGuiCol_BorderShadow ]           = ImVec4( 0.000f, 0.000f, 0.000f, 0.000f );
		
		colors[ ImGuiCol_FrameBg ]                = ImGui::ColorConvertU32ToFloat4( IM_COL32( 20, 20, 20, 255 ) );
		colors[ ImGuiCol_FrameBgHovered ]         = ImVec4( 0.200f, 0.200f, 0.200f, 1.000f );
		colors[ ImGuiCol_FrameBgActive ]		  = colors[ ImGuiCol_FrameBg ];

		colors[ ImGuiCol_TitleBg ]                = colors[ ImGuiCol_WindowBg ];
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
		
		colors[ ImGuiCol_Button ]                 = ImGui::ColorConvertU32ToFloat4( IM_COL32( 58, 58, 58, 255 ) );
		colors[ ImGuiCol_ButtonHovered ]          = ImVec4( 1.000f, 1.000f, 1.000f, 0.156f );
		colors[ ImGuiCol_ButtonActive ]           = ImVec4( 1.000f, 1.000f, 1.000f, 0.391f );
		
		colors[ ImGuiCol_Header ]                 = ImGui::ColorConvertU32ToFloat4( IM_COL32( 47, 47, 47, 255 ) );
		colors[ ImGuiCol_HeaderHovered ]          = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
		colors[ ImGuiCol_HeaderActive ]           = colors[ ImGuiCol_Header ];
		
		colors[ ImGuiCol_Separator ]			  = colors[ ImGuiCol_WindowBg ];
		colors[ ImGuiCol_SeparatorHovered ]       = ImVec4( 0.391f, 0.391f, 0.391f, 1.000f );
		colors[ ImGuiCol_SeparatorActive ]        = ImGui::ColorConvertU32ToFloat4( IM_COL32( 76, 121, 141, 255 ) );

		colors[ ImGuiCol_ResizeGrip ]             = ImVec4( 1.000f, 1.000f, 1.000f, 0.250f );
		colors[ ImGuiCol_ResizeGripHovered ]      = ImVec4( 1.000f, 1.000f, 1.000f, 0.670f );
		colors[ ImGuiCol_ResizeGripActive ]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
		
		colors[ ImGuiCol_Tab ]                    = colors[ ImGuiCol_ChildBg ];
		colors[ ImGuiCol_TabHovered ]             = ImVec4( 0.352f, 0.352f, 0.352f, 1.000f );
		colors[ ImGuiCol_TabActive ]              = ImGui::ColorConvertU32ToFloat4( IM_COL32( 76, 69, 47, 255 ) );
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