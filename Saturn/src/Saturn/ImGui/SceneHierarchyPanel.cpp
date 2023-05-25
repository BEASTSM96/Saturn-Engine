/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "Saturn/Vulkan/SceneRenderer.h"
#include "UITools.h"

#include "Saturn/Vulkan/VulkanContext.h"

#include "Saturn/GameFramework/EntityScriptManager.h"

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
		m_EditIcon = Ref<Texture2D>::Create( "content/textures/editor/EditIcon.png", AddressingMode::Repeat );
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

			if( !e )
				return;

			if( !m_Context->m_Registry.any_of<SceneComponent>( entity ) && !e.HasParent() )
			{
				DrawEntityNode( e );
			}
		} );
	}

	template<typename Ty>
	void SceneHierarchyPanel::DrawAddComponents( const char* pName, Entity entity )
	{
		if( !m_SelectionContext.HasComponent<Ty>() )
		{
			if( ImGui::Button( pName ) )
			{
				m_SelectionContext.AddComponent<Ty>();

				ImGui::CloseCurrentPopup();
			}
		}
	}

	void SceneHierarchyPanel::Draw()
	{
		ImGui::PushID( static_cast<int>( m_CustomID == 0 ? m_Context->GetId() : m_CustomID ) );

		if( !m_IsPrefabScene )
			ImGui::Begin( m_WindowName.c_str() );

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

				auto components = m_Context->m_Registry.view<DirectionalLightComponent>();

				if( components.empty() )
				{
					if( ImGui::MenuItem( "Directional Light" ) )
					{
						auto Entity = m_Context->CreateEntity( "Directional Light" );
						Entity.AddComponent<DirectionalLightComponent>();
						Entity.GetComponent<TransformComponent>().Rotation = glm::radians( glm::vec3( 80.0f, 10.0f, 0.0f ) );

						SetSelected( Entity );
					}
				}

				auto SkylightComponents = m_Context->m_Registry.view<SkylightComponent>();
				
				if( SkylightComponents.empty() )
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

			std::string name = "Inspector##" + m_WindowName;
			ImGui::Begin( name.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse );

			if( m_SelectionContext )
			{
				DrawComponents( m_SelectionContext );
			}

			ImGui::End();
		}

		if( !m_IsPrefabScene )
			ImGui::End();

		ImGui::PopID();
	}
	
	void SceneHierarchyPanel::DrawComponents( Entity entity )
	{
		DrawEntityComponents( m_SelectionContext );

		if( ImGui::Button( "Add Component" ) )
			ImGui::OpenPopup( "AddComponentPanel" );

		if( ImGui::BeginPopup( "AddComponentPanel" ) )
		{
			DrawAddComponents<StaticMeshComponent>( "Static Mesh", m_SelectionContext );

			DrawAddComponents<ScriptComponent>( "Script", m_SelectionContext );

			DrawAddComponents<CameraComponent>( "Camera", m_SelectionContext );

			DrawAddComponents<PointLightComponent>( "Point Light", m_SelectionContext );

			DrawAddComponents<DirectionalLightComponent>( "Directional Light", m_SelectionContext );

			DrawAddComponents<BoxColliderComponent>( "Box Collider", m_SelectionContext );

			DrawAddComponents<SphereColliderComponent>( "Sphere Collider", m_SelectionContext );

			DrawAddComponents<CapsuleColliderComponent>( "Capsule Collider", m_SelectionContext );

			DrawAddComponents<MeshColliderComponent>( "Mesh Collider", m_SelectionContext );

			DrawAddComponents<RigidbodyComponent>( "Rigidbody", m_SelectionContext );

			DrawAddComponents<PhysicsMaterialComponent>( "Physics material", m_SelectionContext );

			ImGui::EndPopup();
		}
	}

	void SceneHierarchyPanel::DrawEntityNode( Entity entity )
	{
		if( entity.HasComponent<TagComponent>() )
		{
			auto& rTag = entity.GetComponent<TagComponent>().Tag;

			ImGuiTreeNodeFlags Flags = ( ( m_SelectionContext == entity ) ? ImGuiTreeNodeFlags_Selected : 0 ) | ImGuiTreeNodeFlags_OpenOnArrow;
			Flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

			bool Clicked;

			Clicked = ImGui::TreeNodeEx( (void*)(uint32_t)entity, Flags, rTag.c_str() );

			if( ImGui::IsItemClicked() )
			{
				m_SelectionContext = entity;
				SetSelected( entity );

				if( m_SelectionChangedCallback )
					m_SelectionChangedCallback( m_SelectionContext );
			}

			if( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceAllowNullID ) ) 
			{
				ImGui::Text( rTag.c_str() );

				ImGui::SetDragDropPayload( "ENTITY_PARENT_SCHPANEL", &entity, sizeof( Entity ), ImGuiCond_Once );

				ImGui::EndDragDropSource();
			}

			if( ImGui::BeginDragDropTarget() )
			{
				const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload( "ENTITY_PARENT_SCHPANEL" );

				if( pPayload )
				{
					Entity& e = *( Entity* ) pPayload->Data;
					Entity previousParent = m_Context->FindEntityByID( e.GetParent() );

					// If a child is trying to parent it's parent.
					bool ParentToParent = false;
					for( auto& child : e.GetChildren() )
					{
						if( child == entity.GetUUID() )
						{
							ParentToParent = true;
							break;
						}
					}

					if( !ParentToParent )
					{
						if( previousParent )
						{
							auto& children = previousParent.GetChildren();
							children.erase( std::remove( children.begin(), children.end(), e.GetComponent<IdComponent>().ID ), children.end() );
						}

						e.SetParent( entity.GetComponent<IdComponent>().ID );

						auto& children = entity.GetChildren();
						children.push_back( e.GetComponent<IdComponent>().ID );
					}
				}

				ImGui::EndDragDropTarget();
			}

			if( Clicked ) 
			{
				for ( auto& child : entity.GetChildren() )
				{
					Entity e = m_Context->FindEntityByID( child );
					if( e )
						DrawEntityNode( e );
				}

				ImGui::TreePop();
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

		DrawComponent<TransformComponent>( "Transform", entity, [&]( auto& tc )
		{
			auto& translation = tc.Position;
			glm::vec3 rotation = glm::degrees( tc.Rotation );
			auto& scale = tc.Scale;

			DrawVec3Control( "Translation", tc.Position );
			
			if( DrawVec3Control( "Rotation", rotation ) )
				tc.Rotation = glm::radians( rotation );

			DrawVec3Control( "Scale", tc.Scale, 1.0f );
		} );

		DrawComponent<StaticMeshComponent>( "Static Mesh", entity, [&]( auto& mc )
		{
			static AssetID id;

			ImGui::Columns( 3 );
			ImGui::SetColumnWidth( 0, 100 );
			ImGui::SetColumnWidth( 1, 300 );
			ImGui::SetColumnWidth( 2, 40 );
			ImGui::Text( "File Path" );
			ImGui::NextColumn();
			ImGui::PushItemWidth( -1 );

			if( ImGui::Button( "...##openmesh", ImVec2( 50, 20 ) ) )
			{
				m_OpenAssetFinderPopup = !m_OpenAssetFinderPopup;
				m_CurrentFinderType = AssetType::StaticMesh;
			}
			
			ImGui::SameLine();

			if( m_OpenAssetFinderPopup )
				ImGui::OpenPopup( "AssetFinderPopup" );

			ImGui::SetNextWindowSize( { 250.0f, 0.0f } );
			if( ImGui::BeginPopup( "AssetFinderPopup", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize ) )
			{
				bool PopupModified = false;

				if( ImGui::BeginListBox( "##ASSETLIST", ImVec2( -FLT_MIN, 0.0f ) ) )
				{
					for( const auto& [assetID, rAsset] : AssetRegistry::Get().GetAssetMap() )
					{
						bool Selected = ( id == assetID );

						ImGui::PushID( assetID );

						if( rAsset->GetAssetType() == m_CurrentFinderType || m_CurrentFinderType == AssetType::Unknown )
						{
							if( ImGui::Selectable( rAsset->GetName().c_str() ) )
							{
								mc.Mesh = AssetRegistry::Get().GetAssetAs<StaticMesh>( assetID );

								PopupModified = true;
							}
						}

						ImGui::PopID();

						if( Selected )
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndListBox();
				}

				if( PopupModified )
				{
					m_OpenAssetFinderPopup = false;

					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if( mc.Mesh )
				ImGui::InputText( "##meshfilepath", ( char* ) mc.Mesh->Name.c_str(), 256, ImGuiInputTextFlags_ReadOnly );
			else
				ImGui::InputText( "##meshfilepath", ( char* ) "", 256, ImGuiInputTextFlags_ReadOnly );

			ImGui::PopItemWidth();
			ImGui::NextColumn();

		} );

		DrawComponent<CameraComponent>( "Camera", entity, [&]( auto& cc )
		{
			ImGui::Checkbox( "Main Camera", &cc.MainCamera );
		} );

		DrawComponent<PointLightComponent>( "Point Light", entity, []( auto& plc )
		{
			DrawColorVec3Control( "Light Color", plc.Radiance, 150.0f );


			DrawFloatControl( "Light Intensity", plc.Multiplier, 0.0f, 500.0f );
			DrawFloatControl( "Radius", plc.Radius, 0.0f, FLT_MAX );
			DrawFloatControl( "Falloff", plc.Falloff, 0.0f, 1.0f );
		} );

		DrawComponent<DirectionalLightComponent>( "Directional Light", entity, []( auto& dlc )
		{
			DrawFloatControl( "Intensity", dlc.Intensity, 110.0f );
			DrawBoolControl( "Cast shadows", dlc.CastShadows );
			DrawColorVec3Control( "Radiance", dlc.Radiance, 1.0f );
		} );

		DrawComponent<SkylightComponent>( "Skylight", entity, []( auto& skl )
		{
			DrawBoolControl( "Dynamic Sky", skl.DynamicSky );

			bool changed = false;

			if ( skl.DynamicSky )
			{
				changed = DrawFloatControl( "Turbidity", skl.Turbidity );
				changed |= DrawFloatControl( "Azimuth", skl.Azimuth );
				changed |= DrawFloatControl( "Inclination", skl.Inclination );

				if( changed )
					Application::Get().PrimarySceneRenderer().SetDynamicSky( skl.Turbidity, skl.Azimuth, skl.Inclination );
			}
		} );
		
		DrawComponent<BoxColliderComponent>( "Box Collider", entity, []( auto& bc )
		{
			DrawVec3Control( "Extent", bc.Extents );
			DrawVec3Control( "Offset", bc.Offset );
			
			DrawBoolControl( "IsTrigger", bc.IsTrigger );
		} );

		DrawComponent<SphereColliderComponent>( "Sphere Collider", entity, []( auto& sc )
		{
			DrawVec3Control( "Offset", sc.Offset );
			DrawFloatControl( "Radius", sc.Radius );

			DrawBoolControl( "IsTrigger", sc.IsTrigger );
		} );

		DrawComponent<CapsuleColliderComponent>( "Capsule Collider", entity, []( auto& cc )
		{
			DrawVec3Control( "Offset", cc.Offset );

			DrawFloatControl( "Radius", cc.Radius );
			DrawFloatControl( "Height", cc.Height );

			DrawBoolControl( "IsTrigger", cc.IsTrigger );
		} );

		DrawComponent<MeshColliderComponent>( "Mesh Collider", entity, []( auto& mcc )
		{
		} );

		DrawComponent<RigidbodyComponent>( "Rigidbody", entity, []( auto& rb )
		{
			DrawBoolControl( "Is Kinematic", rb.IsKinematic );
			DrawBoolControl( "Use CCD", rb.UseCCD );

			int value = rb.Mass;
			bool modified = false;
			modified |= ImGui::DragInt( "Mass##intx", &value, 0.0f, 5000, 0, "%.2f" );

			if ( modified )
			{
				rb.Mass = value;
			}

			float drag = rb.LinearDrag;
			modified = false;
			modified |= ImGui::DragFloat( "LinearDrag##inty", &drag, 0.0f, 5000, 0, "%.2f" );

			if( modified )
			{
				rb.LinearDrag = drag;
			}
		} );

	}

}