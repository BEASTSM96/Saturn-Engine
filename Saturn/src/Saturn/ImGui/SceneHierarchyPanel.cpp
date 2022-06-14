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
#include "SceneHierarchyPanel.h"

#include "Saturn/Core/App.h"
#include "Saturn/Vulkan/Mesh.h"
#include "Saturn/Scene/Entity.h"
#include "UITools.h"

#include "Saturn/Vulkan/VulkanContext.h"

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
	
	SceneHierarchyPanel::SceneHierarchyPanel() : Panel( "Scene Hierarchy Panel" )
	{
		m_EditIcon = Ref<Texture2D>::Create( "assets/textures/editor/EditIcon.png", AddressingMode::Repeat );
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
		m_EditIcon = nullptr;
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

	void SceneHierarchyPanel::DrawEntities()
	{
		m_Context->m_Registry.each( [&]( auto entity )
		{
			Entity e{ entity, m_Context.Pointer() };
			if( !m_Context->m_Registry.has<SceneComponent>( entity ) || !e ) 
			{
				DrawEntityNode( e );
			}
		} );
	}

	void SceneHierarchyPanel::Draw()
	{
		ImGui::Begin( "Scene Hierarchy" );

		if( m_Context )
		{	
			DrawEntities();

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

				if( ImGui::MenuItem( "Directional Light" ) )
				{
					auto Entity = m_Context->CreateEntity( "Directional Light" );
					Entity.AddComponent<DirectionalLightComponent>();
					Entity.GetComponent<TransformComponent>().Rotation = glm::radians( glm::vec3( 80.0f, 10.0f, 0.0f ) );

					SetSelected( Entity );
				}

				auto components = m_Context->m_Registry.view<SkylightComponent>();
				
				if( components.empty() )
				{
					if( ImGui::MenuItem( "Skylight" ) )
					{
						auto Entity = m_Context->CreateEntity( "Skylight" );
						Entity.AddComponent<SkylightComponent>();
						
						SetSelected( Entity );
					}
				}

				ImGui::EndPopup();
			}

			ImGui::Begin( "Inspector", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse );
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

			if( !m_SelectionContext.HasComponent<LightComponent>() )
			{
				if( ImGui::Button( "Light" ) )
				{
					m_SelectionContext.AddComponent<LightComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if( !m_SelectionContext.HasComponent<DirectionalLightComponent>() ) 
			{
				if( ImGui::Button( "Directional Light" ) )
				{
					m_SelectionContext.AddComponent<DirectionalLightComponent>();
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

			ImGui::Selectable( tag.c_str(), m_SelectionContext && m_SelectionContext == entity ? true : false );

			if( ImGui::IsItemClicked() )
			{
				m_SelectionContext = entity;
				SetSelected( entity );

				if( m_SelectionChangedCallback )
					m_SelectionChangedCallback( m_SelectionContext );
			}
		}
	}

	void SceneHierarchyPanel::DrawEntityComponents( Entity entity )
	{
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		auto id = entity.GetComponent<IdComponent>().ID;
		
		ImGui::Image( m_EditIcon->GetDescriptorSet(), ImVec2( 30, 30 ) );

		ImGui::SameLine();

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

			DrawVec3Control( "Translation", tc.Position );
			DrawVec3Control( "Rotation", tc.Rotation );
			DrawVec3Control( "Scale", tc.Scale, 1.0f );
		} );

		DrawComponent<MeshComponent>( "Mesh", entity, [&]( auto& mc )
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
				if( mc.Mesh )
					mc.Mesh.Delete();

				std::string file = Application::Get().OpenFile( "ObjectFile (*.fbx *.obj *.glb *.glft)\0*.fbx; *.obj; *.gltf\0" );
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

		DrawComponent<LightComponent>( "Light", entity, []( auto& lc )
		{
			DrawColorVec3Control( "Light Color", lc.Color, 150.0f );
		
			DrawFloatControl( "Light Intensity", lc.Intensity, 110.0f );
		} );

		DrawComponent<DirectionalLightComponent>( "Directional Light", entity, []( auto& dlc )
		{
			DrawFloatControl( "Intensity", dlc.Intensity, 110.0f );
			DrawBoolControl( "Cast shadows", dlc.CastShadows );
		} );

		DrawComponent<SkylightComponent>( "Skylight", entity, []( auto& skl )
		{
			if( !skl.DynamicSky )
			{
				if( ImGui::Button( "...##openenvmap", ImVec2( 50, 20 ) ) ) 
				{
					std::string file = Application::Get().OpenFile( "Environment map file (*.hdr)\0*.hdr;\0" );	
				}
			}

			DrawBoolControl( "Dynamic Sky", skl.DynamicSky );

			if ( skl.DynamicSky )
			{
				DrawFloatControl( "Turbidity", skl.Turbidity );
				DrawFloatControl( "Azimuth", skl.Azimuth );
				DrawFloatControl( "Inclination", skl.Inclination );
			}
		} );

	}

}