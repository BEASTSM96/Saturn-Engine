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
#include "ViewportBar.h"

#include "Saturn/Editor/EditorLayer.h"
#include "Saturn/Core/App.h"

#include "UITools.h"

#include <imgui.h>
#include "ImGuizmo/ImGuizmo.h"

namespace Saturn {

	ViewportBar::ViewportBar() : Panel( "Viewport Bar" )
	{
		m_PlayImage = Ref< Texture2D >::Create( "assets/textures/PlayButton.png", AddressingMode::Repeat );
		m_PauseImage = Ref< Texture2D >::Create( "assets/textures/PauseButton.png", AddressingMode::Repeat );
		m_StopImage = Ref< Texture2D >::Create( "assets/textures/StopButton.png", AddressingMode::Repeat );

		m_CursorTexture = Ref< Texture2D >::Create( "assets/textures/editor/Cursor.png", AddressingMode::Repeat );
		m_MoveTexture = Ref< Texture2D >::Create( "assets/textures/editor/Move.png", AddressingMode::Repeat );
		m_RotateTexture = Ref< Texture2D >::Create( "assets/textures/editor/Rotate.png", AddressingMode::Repeat );
		m_ScaleTexture = Ref< Texture2D >::Create( "assets/textures/editor/Scale.png", AddressingMode::Repeat );
		m_SettingsTexture = Ref< Texture2D >::Create( "assets/textures/editor/Settings.png", AddressingMode::Repeat );
	}

	ViewportBar::~ViewportBar()
	{
		m_CursorTexture = nullptr;
		m_MoveTexture = nullptr;
		m_RotateTexture = nullptr;
		m_ScaleTexture = nullptr;
		m_PauseImage = nullptr;
		m_PlayImage = nullptr;
		m_StopImage = nullptr;
		m_SettingsTexture = nullptr;
	}

	void ViewportBar::Draw()
	{
		auto Viewport = Application::Get().GetEditorLayer()->GetViewport();
		auto ViewportPos = Viewport->m_WindowPos;
		auto ViewportSize = Viewport->m_WindowSize;
		
		ImVec2 Pos;
		Pos.x = ViewportPos.x + 5;
		Pos.y = ViewportPos.y + 5;

		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );

		DrawOverlay( "##GizmoOverlayControl", Pos );

		if( ImGui::ImageButton( m_CursorTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
			Viewport->SetOperation( -1 );

		ImGui::SameLine();

		if( ImGui::ImageButton( m_MoveTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
			Viewport->SetOperation( ImGuizmo::OPERATION::TRANSLATE );

		ImGui::SameLine();

		if( ImGui::ImageButton( m_RotateTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
			Viewport->SetOperation( ImGuizmo::OPERATION::ROTATE );

		ImGui::SameLine();

		if( ImGui::ImageButton( m_ScaleTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
			Viewport->SetOperation( ImGuizmo::OPERATION::SCALE );

		ImGui::SameLine();

		if( ImGui::ImageButton( m_PlayImage->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
			SAT_CORE_INFO( "Runtime..." );

		ImGui::SameLine();

		if( ImGui::ImageButton( m_SettingsTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
			SAT_CORE_INFO( "Settings" );

		EndOverlay();

		ImGui::PopStyleColor( 2 );
	}

}