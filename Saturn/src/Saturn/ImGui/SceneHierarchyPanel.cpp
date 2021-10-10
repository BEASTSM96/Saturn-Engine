/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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
#include "SceneHierarchyPanel.h"

#include "Saturn/Scene/Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>

namespace Saturn {

	SceneHierarchyPanel::SceneHierarchyPanel()
	{
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::SetContext( const Ref<Scene>& scene )
	{
		m_Context = scene;
		m_SelectionContext  ={};
	}

	void SceneHierarchyPanel::SetSelected( Entity entity )
	{
		m_SelectionContext = entity;
		m_Context->SetSelectedEntity( entity );
	}

	void SceneHierarchyPanel::Draw()
	{
		ImGui::Begin( "Scene Hierarchy" );

		if( m_Context )
		{
			m_Context->m_Registry.each( [&]( auto entity ) 
			{
				Entity e{ entity, m_Context.Pointer() };
				if( !m_Context->m_Registry.has<SceneComponent>( entity ) || !e )
					DrawEntityNode( e );
			} );

			if( ImGui::IsMouseDown( 0 ) && ImGui::IsWindowHovered() )
			{
				SetSelected( {} );
			}

			if( ImGui::BeginPopupContextWindow( 0, 1, false ) )
			{
				if( ImGui::MenuItem( "Create Empty Entity" ) )
				{
					auto Entity = m_Context->CreateEntity( "Empty Entity" );
					SetSelected( Entity );
				}
				ImGui::EndPopup();
			}

			ImGui::Begin( "Inspector" );
			if( m_SelectionContext )
			{
				DrawComponents( m_SelectionContext );
			}
			ImGui::End();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawComponents( Entity entity )
	{
		DrawEntityComponents( m_SelectionContext );
	}

	void SceneHierarchyPanel::DrawEntityNode( Entity entity )
	{
		if( entity.HasComponent<TagComponent>() )
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			ImGuiTreeNodeFlags flags = ( ( m_SelectionContext == entity ) ? ImGuiTreeNodeFlags_Selected : 0 ) | ImGuiTreeNodeFlags_OpenOnArrow;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

			bool opened = ImGui::TreeNodeEx( ( void* )( uint64_t )( uint32_t )entity, flags, tag.c_str() );

			if( ImGui::IsItemClicked() )
			{
				m_SelectionContext = entity;
				if( m_SelectionChangedCallback )
				{
					m_SelectionChangedCallback( m_SelectionContext );
				}
			}

			if( opened )
			{
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
				bool opened = ImGui::TreeNodeEx( ( void* )9817239, flags, tag.c_str() );
				if( opened )
					ImGui::TreePop();
				ImGui::TreePop();
			}
		}
		else
		{
			entity.AddComponent<TagComponent>();
			DrawEntityNode( entity );
		}
	}

	void SceneHierarchyPanel::DrawEntityComponents( Entity entity )
	{
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		auto id = entity.GetComponent<IdComponent>().ID;

		if( entity.HasComponent<TagComponent>() )
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[ 256 ];
			memset( buffer, 0, 256 );
			memcpy( buffer, tag.c_str(), tag.length() );
			ImGui::PushItemWidth( contentRegionAvailable.x * 0.5f );
			if( ImGui::InputText( "##Tag", buffer, 256 ) )
			{
				tag = std::string( buffer );
			}
			ImGui::PopItemWidth();
		}

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

		// ID
		ImGui::SameLine();
		ImGui::TextDisabled( "%llx", id );
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	}

}