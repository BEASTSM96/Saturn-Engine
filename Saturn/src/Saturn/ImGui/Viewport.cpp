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
#include "Viewport.h"
#include "Saturn/Core/App.h"

#include "Saturn/Vulkan/SceneRenderer.h"

#include "UITools.h"

#include "imgui.h"
#include "ImGuizmo/ImGuizmo.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Saturn/Vulkan/VulkanContext.h"

namespace Saturn {

	Viewport::Viewport()
	{
		m_CursorTexture = Ref< Texture2D >::Create( "assets/textures/editor/Cursor.png", AddressingMode::Repeat );
		m_MoveTexture = Ref< Texture2D >::Create( "assets/textures/editor/Move.png", AddressingMode::Repeat );
		m_RotateTexture = Ref< Texture2D >::Create( "assets/textures/editor/Rotate.png", AddressingMode::Repeat );
		m_ScaleTexture = Ref< Texture2D >::Create( "assets/textures/editor/Scale.png", AddressingMode::Repeat );
	}

	Viewport::~Viewport()
	{
		m_CursorTexture = nullptr;
		m_MoveTexture = nullptr;
		m_RotateTexture = nullptr;
		m_ScaleTexture = nullptr;
	}

	void Viewport::Draw()
	{
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
		
		ImGui::Begin( "Viewport", 0, flags );

		auto viewportOffset = ImGui::GetWindowPos(); // includes tab bar
		auto viewportSize = ImGui::GetContentRegionAvail();
		
		auto windowSize = ImGui::GetWindowSize();

		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };

		m_SendCameraEvents = ImGui::IsMouseHoveringRect( minBound, maxBound );

		ImGui::Image( SceneRenderer::Get().CompositeImage(), viewportSize );
		
		// Draw gizmo controls.

		{			
			ImVec2 Pos;
			Pos.x = viewportOffset.x + 5;
			Pos.y = viewportOffset.y + 5;

			DrawOverlay( "##GizmoOverlayControl", Pos );

			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );

			if( ImGui::ImageButton( m_CursorTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
				m_GizmoOperation = -1;

			ImGui::SameLine();

			if( ImGui::ImageButton( m_MoveTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
				m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

			ImGui::SameLine();

			if( ImGui::ImageButton( m_RotateTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
				m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;

			ImGui::SameLine();

			if( ImGui::ImageButton( m_ScaleTexture->GetDescriptorSet(), ImVec2( 30, 30 ) ) )
				m_GizmoOperation = ImGuizmo::OPERATION::SCALE;

			ImGui::PopStyleColor( 2 );

			EndOverlay();
		}
		
		// Draw Gizmo
		Entity selectedEntity = Application::Get().GetEditorLayer()->GetSceneHierarchyPanel()->GetSelectionContext();

		if( selectedEntity && m_GizmoOperation != -1 )
		{
			if( !selectedEntity.HasComponent<SkylightComponent>() )
			{
				ImGuizmo::SetOrthographic( false );
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect( viewportOffset.x, viewportOffset.y, viewportSize.x, viewportSize.y );

				auto& tc = selectedEntity.GetComponent<TransformComponent>();

				glm::mat4 transform = tc.GetTransform();

				EditorCamera camera = Application::Get().GetEditorLayer()->GetEditorCamera();

				const glm::mat4 Projection = camera.ProjectionMatrix();
				const glm::mat4 View = camera.ViewMatrix();

				ImGuizmo::Manipulate( glm::value_ptr( View ), glm::value_ptr( Projection ), (ImGuizmo::OPERATION)m_GizmoOperation, ImGuizmo::LOCAL, glm::value_ptr( transform ) );

				if( ImGuizmo::IsUsing() )
				{
					glm::vec3 translation;
					glm::quat Qrotation;
					glm::vec3 scale;
					glm::vec3 skew;
					glm::vec4 perspective;

					glm::decompose( transform, scale, Qrotation, translation, skew, perspective );

					glm::vec3 rotation = glm::eulerAngles( Qrotation );
					
					glm::vec3 DeltaRotation = rotation - tc.Rotation;

					tc.Position = translation;
					tc.Rotation += DeltaRotation;
					tc.Scale = scale;
				}
			}
		}

		ImGui::End();

		ImGui::PopStyleVar();
	}

}