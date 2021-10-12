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

#include "Saturn/Core/App.h"

#include "Saturn/OpenGL/Mesh.h"

#include "Saturn/Scene/Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>

namespace Saturn {

	template<typename T, typename UIFunction>
	static void DrawComponent( const std::string& name, Entity entity, UIFunction uiFunction )
	{
		if( entity.HasComponent<T>() )
		{
			bool removeComponent = false;

			auto& component = entity.GetComponent<T>();
			bool open = ImGui::TreeNodeEx( ( void* )( ( uint32_t )entity | typeid( T ).hash_code() ), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap, name.c_str() );
			ImGui::SameLine();
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );
			if( ImGui::Button( "+" ) )
			{
				ImGui::OpenPopup( "ComponentSettings" );
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			if( ImGui::BeginPopup( "ComponentSettings" ) )
			{
				if( ImGui::MenuItem( "Remove component" ) )
					removeComponent = true;

				ImGui::EndPopup();
			}

			if( open )
			{
				uiFunction( component );
				ImGui::NextColumn();
				ImGui::Columns( 1 );
				ImGui::TreePop();
			}
			ImGui::Separator();

			if( removeComponent )
			{
				entity.RemoveComponent<T>();
			}
		}
	}

	static bool DrawVec3Control( const std::string& label, glm::vec3& values, glm::vec3& currentValues, float resetValue = 0.0f, float columnWidth = 100.0f )
	{
		bool modified = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[ 0 ];

		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2 );
		ImGui::SetColumnWidth( 0, columnWidth );
		ImGui::Text( label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 } );

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize ={ lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "X", buttonSize ) ) )
		{
			values.x = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "Y", buttonSize ) ) )
		{
			values.y = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f } );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f } );
		ImGui::PushFont( boldFont );
		if( ( ImGui::Button( "Z", buttonSize ) ) )
		{
			values.z = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor( 3 );

		ImGui::SameLine();
		modified |= ImGui::DragFloat( "##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f" );
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns( 1 );

		ImGui::PopID();

		if( modified )
		{
			currentValues.z = values.z;
			currentValues.x = values.x;
			currentValues.y = values.y;
		}

		return modified;

	}

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

		if( ImGui::Button( "Add Component" ) )
			ImGui::OpenPopup( "AddComponentPanel" );

		if( ImGui::BeginPopup( "AddComponentPanel" ) )
		{
			if( !m_SelectionContext.HasComponent<MeshComponent>() )
			{
				if( ImGui::Button( "Mesh" ) )
				{
					m_SelectionContext.AddComponent<MeshComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

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


		DrawComponent<TransformComponent>( "Transform", entity, []( auto& tc )
		{
			auto& translation = tc.Position;
			auto& rotation = tc.Rotation;
			auto& scale = tc.Scale;

			bool updateTransform = false;
			updateTransform |= DrawVec3Control( "Translation", tc.Position, tc.Position );

			updateTransform |= DrawVec3Control( "Rotation", tc.Rotation, tc.Rotation );
			updateTransform |= DrawVec3Control( "Scale", tc.Scale, tc.Scale, 1.0f );
		} );

		DrawComponent<MeshComponent>( "Mesh", entity, []( auto& mc )
		{
			ImGui::Columns( 3 );
			ImGui::SetColumnWidth( 0, 100 );
			ImGui::SetColumnWidth( 1, 300 );
			ImGui::SetColumnWidth( 2, 40 );
			ImGui::Text( "File Path" );
			ImGui::NextColumn();
			ImGui::PushItemWidth( -1 );

			if( ImGui::Button( "...##openmesh", ImVec2( 50, 20 ) ) )
			{
				std::string file = Application::Get().OpenFile( "ObjectFile (*.fbx *.obj)\0*.fbx; *.obj\0" ).first;
				if( !file.empty() )
					mc.Mesh = Ref<Mesh>::Create( file );
			}

			if( mc.Mesh )
				ImGui::InputText( "##meshfilepath", ( char* )mc.Mesh->FilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly );
			else
				ImGui::InputText( "##meshfilepath", ( char* )"", 256, ImGuiInputTextFlags_ReadOnly );

			ImGui::PopItemWidth();
			ImGui::NextColumn();

		} );
	}

}