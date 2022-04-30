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

	std::string::value_type CharUpFunc( std::string::value_type val ) 
	{
		return std::use_facet< std::ctype< std::string::value_type > >( std::locale() ).toupper( val );
	}

	std::string ToUpper( const std::string& src ) 
	{
		std::string result;
		std::transform( src.begin(), src.end(), std::back_inserter( result ), CharUpFunc );
		return result;
	}

	std::string::value_type CharDownFunc( std::string::value_type val )
	{
		return std::use_facet< std::ctype< std::string::value_type > >( std::locale() ).tolower( val );
	}

	std::string ToLower( const std::string& src )
	{
		std::string result;
		std::transform( src.begin(), src.end(), std::back_inserter( result ), CharDownFunc );
		return result;
	}

	template<typename T, typename UIFunction>
	static void DrawComponent( const std::string& name, Entity entity, UIFunction uiFunction )
	{
		if( entity.HasComponent<T>() )
		{
			bool removeComponent = false;

			auto& component = entity.GetComponent<T>();

			bool open = ImGui::TreeNodeEx( ( void* )( ( uint32_t )entity | typeid( T ).hash_code() ), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap, ToUpper( name ).c_str() );

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

		/*
		if( m_Context->m_Registry.valid( entity ) ) 
		{
			if( entity.HasComponent<MeshComponent>() )
			{
				if( entity.GetComponent<MeshComponent>().Mesh ) 
				{
					VulkanContext::Get().RenderDebugUUID( entity.GetComponent< IdComponent >().ID );
				}
			}	
		} 
		else
		{
			VulkanContext::Get().ShowDebugUUID( false );
		}
		*/
	}

	void SceneHierarchyPanel::DrawEntities()
	{
		
		ImGui::Columns( 2, "Bar" );

		ImGui::Columns( 2, "Key" );
		{
			ImGui::Text( "Name" );

			ImGui::NextColumn();

			ImGui::Text( "Visibility" );

			ImGui::NextColumn();
		}

		ImGui::Separator();

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

				if( ImGui::MenuItem( "Skylight" ) )
				{
					auto Entity = m_Context->CreateEntity( "Skylight" );
					Entity.AddComponent<SkylightComponent>();
					SetSelected( Entity );
				}

				ImGui::EndPopup();
			}

			if( m_SelectionContext && m_SelectionContext.HasComponent<MeshComponent>() )
			{
				auto& mesh = m_SelectionContext.GetComponent<MeshComponent>().Mesh;

				if( mesh )
				{
				}
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

			if( !m_SelectionContext.HasComponent<LightComponent>() )
			{
				if( ImGui::Button( "Light" ) )
				{
					m_SelectionContext.AddComponent<LightComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if( !m_SelectionContext.HasComponent<SkylightComponent>() )
			{
				if( ImGui::Button( "Skylight" ) )
				{
					m_SelectionContext.AddComponent<SkylightComponent>();
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
				
				if( m_SelectionChangedCallback )
					m_SelectionChangedCallback( m_SelectionContext );
			}

			ImGui::NextColumn();
			
			{
				auto& visibility = entity.GetComponent<VisibilityComponent>().visibility;

				const char* visibilityStr = visibility == Visibility::Visible ? "Visible" : "Hidden";

				ImGui::Selectable( visibilityStr );

				if( ImGui::IsItemClicked() )
				{
					Visibility newVis = visibility == Visibility::Visible ? Visibility::Hidden : Visibility::Visible;

					entity.GetComponent<VisibilityComponent>().visibility = newVis;
				}
			}

			ImGui::NextColumn();
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
			updateTransform |= DrawVec3Control( "Translation", tc.Position );

			updateTransform |= DrawVec3Control( "Rotation", tc.Rotation );
			updateTransform |= DrawVec3Control( "Scale", tc.Scale, 1.0f );
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

				std::string file = Application::Get().OpenFile( "ObjectFile (*.fbx *.obj)\0*.fbx; *.obj\0" ).first;
				if( !file.empty() )
					mc.Mesh = Ref<Mesh>::Create( file, entity.GetComponent<IdComponent>().ID );
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

		DrawComponent<SkylightComponent>( "Skylight", entity, []( auto& skl )
		{
			if( !skl.DynamicSky )
			{
				if( ImGui::Button( "...##openenvmap", ImVec2( 50, 20 ) ) ) 
				{
					std::string file = Application::Get().OpenFile( "Environment map file (*.hdr)\0*.hdr;\0" ).first;
					
					//skl.Map = { .Name = file, .Path = file, .Texture = nullptr };
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