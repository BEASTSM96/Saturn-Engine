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
#include "ImGuiAuxiliary.h"

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
						Entity.GetComponent<TransformComponent>().SetRotation( glm::radians( glm::vec3( 80.0f, 10.0f, 0.0f ) ) );

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
						
						// Defaults
						Application::Get().PrimarySceneRenderer().SetDynamicSky( 2.0f, 0.0f, 0.0f );

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
			glm::vec3 rotation = glm::degrees( tc.GetRotationEuler() );
			auto& scale = tc.Scale;

			Auxiliary::DrawVec3Control( "Translation", tc.Position );
			
			if( Auxiliary::DrawVec3Control( "Rotation", rotation ) )
				tc.SetRotation( glm::radians( rotation ) );

			Auxiliary::DrawVec3Control( "Scale", tc.Scale, 1.0f );
		} );

		DrawComponent<StaticMeshComponent>( "Static Mesh", entity, [&]( auto& mc )
		{
			static bool s_Open = false;
			static uint32_t s_CurrentIndex = 0;

			ImGui::Columns( 3 );
			ImGui::SetColumnWidth( 0, 100 );
			ImGui::SetColumnWidth( 1, 300 );
			ImGui::SetColumnWidth( 2, 40 );
			ImGui::Text( "File Path" );
			ImGui::NextColumn();
			ImGui::PushItemWidth( -1 );

			if( ImGui::Button( "...##openmesh", ImVec2( 50, 20 ) ) )
			{
				s_Open = !s_Open;
				m_CurrentFinderType = AssetType::StaticMesh;

				if( mc.Mesh )
					m_CurrentAssetID = mc.Mesh->ID;
			}

			if( mc.Mesh )
				ImGui::InputText( "##meshfilepath", ( char* ) mc.Mesh->Name.c_str(), 256, ImGuiInputTextFlags_ReadOnly );
			else
				ImGui::InputText( "##meshfilepath", ( char* ) "", 256, ImGuiInputTextFlags_ReadOnly );

			if( Auxiliary::DrawAssetFinder( m_CurrentFinderType, &s_Open, m_CurrentAssetID ) )
			{
				if( m_CurrentFinderType == AssetType::StaticMesh )
				{
					mc.Mesh = AssetManager::Get().GetAssetAs<StaticMesh>( m_CurrentAssetID );

					mc.MaterialRegistry = nullptr;

					mc.MaterialRegistry = Ref<MaterialRegistry>::Create( mc.Mesh );
				}
				else if ( m_CurrentFinderType == AssetType::Material )
				{
					mc.MaterialRegistry->SetMaterial( s_CurrentIndex, m_CurrentAssetID );
				}
			}
			
			if( mc.Mesh ) 
			{
				if( Auxiliary::TreeNode( "Materials" ) )
				{
					int i = 0;
					for( auto& rAsset : mc.MaterialRegistry->GetMaterials() )
					{
						if( ImGui::Button( rAsset->Name.c_str() ) )
						{
							m_CurrentFinderType = AssetType::Material;
							s_Open = !s_Open;
							s_CurrentIndex = i;
						}

						if( mc.MaterialRegistry->HasOverrides( i ) )
						{
							ImGui::SameLine();

							if( ImGui::SmallButton( "x" ) )
							{
								mc.MaterialRegistry->ResetMaterial( i );
							}
						}

						i++;
					}

					Auxiliary::EndTreeNode();
				}
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();
		} );

		DrawComponent<CameraComponent>( "Camera", entity, [&]( auto& cc )
		{
			ImGui::Checkbox( "Main Camera", &cc.MainCamera );
		} );

		DrawComponent<PointLightComponent>( "Point Light", entity, []( auto& plc )
		{
			Auxiliary::DrawColorVec3Control( "Light Color", plc.Radiance, 150.0f );


			Auxiliary::DrawFloatControl( "Light Intensity", plc.Multiplier, 0.0f, 500.0f );
			Auxiliary::DrawFloatControl( "Radius", plc.Radius, 0.0f, FLT_MAX );
			Auxiliary::DrawFloatControl( "Falloff", plc.Falloff, 0.0f, 1.0f );
		} );

		DrawComponent<DirectionalLightComponent>( "Directional Light", entity, []( auto& dlc )
		{
			Auxiliary::DrawFloatControl( "Intensity", dlc.Intensity, 110.0f );
			Auxiliary::DrawBoolControl( "Cast shadows", dlc.CastShadows );
			Auxiliary::DrawColorVec3Control( "Radiance", dlc.Radiance, 1.0f );
		} );

		DrawComponent<SkylightComponent>( "Skylight", entity, []( auto& skl )
		{
			Auxiliary::DrawBoolControl( "Dynamic Sky", skl.DynamicSky );

			bool changed = false;

			if( skl.DynamicSky )
			{
				changed = Auxiliary::DrawFloatControl( "Turbidity", skl.Turbidity );
				changed |= Auxiliary::DrawFloatControl( "Azimuth", skl.Azimuth );
				changed |= Auxiliary::DrawFloatControl( "Inclination", skl.Inclination );

				if( changed )
					Application::Get().PrimarySceneRenderer().SetDynamicSky( skl.Turbidity, skl.Azimuth, skl.Inclination );
			}
		} );
		
		DrawComponent<BoxColliderComponent>( "Box Collider", entity, []( auto& bc )
		{
			Auxiliary::DrawVec3Control( "Extent", bc.Extents );
			Auxiliary::DrawVec3Control( "Offset", bc.Offset );
			
			Auxiliary::DrawBoolControl( "IsTrigger", bc.IsTrigger );
		} );

		DrawComponent<SphereColliderComponent>( "Sphere Collider", entity, []( auto& sc )
		{
			Auxiliary::DrawVec3Control( "Offset", sc.Offset );
			Auxiliary::DrawFloatControl( "Radius", sc.Radius );

			Auxiliary::DrawBoolControl( "IsTrigger", sc.IsTrigger );
		} );

		DrawComponent<CapsuleColliderComponent>( "Capsule Collider", entity, []( auto& cc )
		{
			Auxiliary::DrawVec3Control( "Offset", cc.Offset );

			Auxiliary::DrawFloatControl( "Radius", cc.Radius );
			Auxiliary::DrawFloatControl( "Height", cc.Height );

			Auxiliary::DrawBoolControl( "IsTrigger", cc.IsTrigger );
		} );

		DrawComponent<MeshColliderComponent>( "Mesh Collider", entity, [&]( auto& mcc )
		{
			Auxiliary::DrawBoolControl( "IsTrigger", mcc.IsTrigger );
		} );

		DrawComponent<RigidbodyComponent>( "Rigidbody", entity, []( auto& rb )
		{
			Auxiliary::DrawBoolControl( "Is Kinematic", rb.IsKinematic );
			Auxiliary::DrawBoolControl( "Use CCD", rb.UseCCD );

			Auxiliary::DrawFloatControl( "Mass", rb.Mass );
			Auxiliary::DrawFloatControl( "Linear Drag", rb.LinearDrag );

			ImGui::PushID( "rbPos" );

			ImGui::Columns( 2 );
			ImGui::SetColumnWidth( 0, 125.0f );

			ImGui::BeginHorizontal( "rbPos" );

			ImGui::Text( "Position Lock" );

			ImGui::NextColumn();

			bool posX = rb.LockFlags & RigidbodyLockFlags::PositionX;
			bool posY = rb.LockFlags & RigidbodyLockFlags::PositionY;
			bool posZ = rb.LockFlags & RigidbodyLockFlags::PositionZ;

			ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
			ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2{ 1.0f, 0.0f } );

			if( ImGui::Checkbox( "##posX", &posX ) )
			{
				if( posX )
					rb.LockFlags |= RigidbodyLockFlags::PositionX;
				else
					rb.LockFlags &= ~RigidbodyLockFlags::PositionX;
			}

			ImGui::PopItemWidth();

			if( ImGui::Checkbox( "##posY", &posY ) )
			{
				if( posY )
					rb.LockFlags |= RigidbodyLockFlags::PositionY;
				else
					rb.LockFlags &= ~RigidbodyLockFlags::PositionY;
			}

			ImGui::PopItemWidth();

			if( ImGui::Checkbox( "##posZ", &posZ ) ) 
			{
				if( posZ )
					rb.LockFlags |= RigidbodyLockFlags::PositionZ;
				else
					rb.LockFlags &= ~RigidbodyLockFlags::PositionZ;
			}

			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::EndHorizontal();

			ImGui::Columns( 1 );

			ImGui::PopID();

			//////////////////////////////////////////////////////////////////////////

			ImGui::PushID( "rbRot" );

			ImGui::Columns( 2 );
			ImGui::SetColumnWidth( 0, 125.0f );

			ImGui::BeginHorizontal( "rbRot" );

			ImGui::Text( "Rotation Lock" );

			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
			ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2{ 1.0f, 0 } );

			bool rotX = rb.LockFlags & RigidbodyLockFlags::RotationX;
			bool rotY = rb.LockFlags & RigidbodyLockFlags::RotationY;
			bool rotZ = rb.LockFlags & RigidbodyLockFlags::RotationZ;

			if( ImGui::Checkbox( "##rotX", &rotX ) )
			{
				if( rotX )
					rb.LockFlags |= RigidbodyLockFlags::RotationX;
				else
					rb.LockFlags &= ~RigidbodyLockFlags::RotationX;
			}

			ImGui::PopItemWidth();

			if( ImGui::Checkbox( "##rotY", &rotY ) )
			{
				if( rotY )
					rb.LockFlags |= RigidbodyLockFlags::RotationY;
				else
					rb.LockFlags &= ~RigidbodyLockFlags::RotationY;
			}

			ImGui::PopItemWidth();

			if( ImGui::Checkbox( "##rotZ", &rotZ ) )
			{
				if( rotZ )
					rb.LockFlags |= RigidbodyLockFlags::RotationZ;
				else
					rb.LockFlags &= ~RigidbodyLockFlags::RotationZ;
			}


			ImGui::PopStyleVar();

			ImGui::EndHorizontal();

			ImGui::Columns( 1 );

			ImGui::PopID();
		} );
	}

}