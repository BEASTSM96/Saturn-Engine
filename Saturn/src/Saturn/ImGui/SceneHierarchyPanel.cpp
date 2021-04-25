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
#include "Saturn/Application.h"
#include "Saturn/Script/ScriptEngine.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_internal.h>

namespace Saturn {

	static int s_UIContextID = 0;
	static uint32_t s_Counter = 0;
	static char s_IDBuffer[ 16 ];

	static void PushID()
	{
		ImGui::PushID( s_UIContextID++ );
		s_Counter = 0;
	}

	static void PopID()
	{
		ImGui::PopID();
		s_UIContextID--;
	}

	static void BeginPropertyGrid()
	{
		PushID();
		ImGui::Columns( 2 );
	}

	static bool Property( const char* label, std::string& value )
	{
		bool modified = false;

		ImGui::Text( label );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		char buffer[ 256 ];
		strcpy( buffer, value.c_str() );

		s_IDBuffer[ 0 ] = '#';
		s_IDBuffer[ 1 ] = '#';
		memset( s_IDBuffer + 2, 0, 14 );
		itoa( s_Counter++, s_IDBuffer + 2, 16 );
		if( ImGui::InputText( s_IDBuffer, buffer, 256 ) )
		{
			value = buffer;
			modified = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static void Property( const char* label, const char* value )
	{
		ImGui::Text( label );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		s_IDBuffer[ 0 ] = '#';
		s_IDBuffer[ 1 ] = '#';
		memset( s_IDBuffer + 2, 0, 14 );
		itoa( s_Counter++, s_IDBuffer + 2, 16 );
		ImGui::InputText( s_IDBuffer, ( char* )value, 256, ImGuiInputTextFlags_ReadOnly );

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}

	static bool Property( const char* label, int& value )
	{
		bool modified = false;

		ImGui::Text( label );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		s_IDBuffer[ 0 ] = '#';
		s_IDBuffer[ 1 ] = '#';
		memset( s_IDBuffer + 2, 0, 14 );
		itoa( s_Counter++, s_IDBuffer + 2, 16 );
		if( ImGui::DragInt( s_IDBuffer, &value ) )
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property( const char* label, float& value, float delta = 0.1f )
	{
		bool modified = false;

		ImGui::Text( label );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		s_IDBuffer[ 0 ] = '#';
		s_IDBuffer[ 1 ] = '#';
		memset( s_IDBuffer + 2, 0, 14 );
		itoa( s_Counter++, s_IDBuffer + 2, 16 );
		if( ImGui::DragFloat( s_IDBuffer, &value, delta ) )
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static bool Property( const char* label, glm::vec2& value, float delta = 0.1f )
	{
		bool modified = false;

		ImGui::Text( label );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		s_IDBuffer[ 0 ] = '#';
		s_IDBuffer[ 1 ] = '#';
		memset( s_IDBuffer + 2, 0, 14 );
		itoa( s_Counter++, s_IDBuffer + 2, 16 );
		if( ImGui::DragFloat2( s_IDBuffer, glm::value_ptr( value ), delta ) )
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	static void EndPropertyGrid()
	{
		ImGui::Columns( 1 );
		PopID();
	}


	SceneHierarchyPanel::SceneHierarchyPanel( const Ref<Scene>& context ) : m_Context( context )
	{
	}

	void SceneHierarchyPanel::SetContext( const Ref<Scene>& scene )
	{
		m_Context = scene;
		m_SelectionContext = {};

		if( m_SelectionContext && false )
		{
			// Try and find same entity in new scene
			auto& entityMap = m_Context->GetEntityMap();
			UUID selectedEntityID = m_SelectionContext.GetUUID();
			if( entityMap.find( selectedEntityID ) != entityMap.end() )
				m_SelectionContext = entityMap.at( selectedEntityID );
		}
	}

	void SceneHierarchyPanel::SetSelected( Entity entity )
	{
		m_SelectionContext = entity;
	}

	void SceneHierarchyPanel::OnUpdate( Timestep ts )
	{
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		SAT_PROFILE_FUNCTION();

		ImGui::Begin( "Scene Hierarchy" );
		if( m_Context )
		{
			uint32_t entityCount = 0, meshCount = 0;
			m_Context->m_Registry.each( [&]( auto entity )
				{
					Entity e( entity, m_Context.Raw() );
					if(!m_Context->m_Registry.has<SceneComponent>( entity ))
						DrawEntityNode( e );
				} );
		}

		if( ImGui::IsMouseDown( 0 ) && ImGui::IsWindowHovered() )
		{
			SetSelected({});
			Reset();
		}

		if( ImGui::BeginPopupContextWindow( 0, 1, false ) )
		{
			if( ImGui::MenuItem( "Create Empty Entity" ) )
			{
				auto Entity =  m_Context->CreateEntity( "Empty Entity" );
				SetSelected( Entity );
			}

			if( ImGui::MenuItem( "Create Mesh Entity" ) )
			{
				Entity e =  m_Context->CreateEntity( "Mesh Entity" );
				if( e.HasComponent<MeshComponent>() )
				{
					SAT_CORE_ASSERT( false, "Entity somehow has a mesh component" );
				}

				if( !e.HasComponent<MeshComponent>() )
				{
					auto& mc = e.AddComponent<MeshComponent>();
					std::string filepath = Application::Get().OpenFile( "ObjectFile (*.fbx *.obj)\0*.fbx; *.obj\0" ).first;
					mc.Mesh = Ref<Mesh>::Create( filepath );
					SetSelected( e );
				}

			}

			if( ImGui::MenuItem( "Create Empty Mesh Entity" ) )
			{
				Entity e =  m_Context->CreateEntity( "Empty Mesh Entity" );
				if( e.HasComponent<MeshComponent>() )
				{
					SAT_CORE_ASSERT( false, "Entity somehow has a mesh component" );
				}

				if( !e.HasComponent<MeshComponent>() )
				{
					auto& mc = e.AddComponent<MeshComponent>();
					SetSelected( e );
				}

			}

			ImGui::EndPopup();

		}

		ImGui::End();

		if (canDraw)
		{
			if( ImGui::Begin( "Inspector" ) )
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
					if( !m_SelectionContext.HasComponent<CameraComponent>() )
					{
						if( ImGui::Button( "Camera" ) )
						{
							m_SelectionContext.AddComponent<CameraComponent>();
							ImGui::CloseCurrentPopup();
						}
					}
					if( !m_SelectionContext.HasComponent<ScriptComponent>() )
					{
						if( ImGui::Button( "Script" ) )
						{
							m_SelectionContext.AddComponent<ScriptComponent>();
							ImGui::CloseCurrentPopup();
						}
					}
					if( ImGui::BeginMenu( "Physics" ) )
					{
						if( !m_SelectionContext.HasComponent<PhysicsComponent>() )
						{

							if( ImGui::MenuItem( "Physics" ) )
							{
								m_SelectionContext.AddComponent<PhysicsComponent>();
							}
						}

						if( !m_SelectionContext.HasComponent<RigidbodyComponent>() )
						{
							if( ImGui::MenuItem( "Rigidbody" ) )
							{
								m_SelectionContext.AddComponent<RigidbodyComponent>();
							}
						}

						if( !m_SelectionContext.HasComponent<BoxColliderComponent>() )
						{
							if( ImGui::MenuItem( "Box Collider" ) )
							{
								m_SelectionContext.AddComponent<BoxColliderComponent>( glm::vec3( 1 ) );
							}
						}

						if( !m_SelectionContext.HasComponent<SphereColliderComponent>() )
						{
							if( ImGui::MenuItem( "Sphere Collider" ) )
							{
								m_SelectionContext.AddComponent<SphereColliderComponent>( 1.0f );
							}
						}

						if( !m_SelectionContext.HasComponent<PhysXRigidbodyComponent>() )
						{
							if( ImGui::MenuItem( "PhysXRigidbody" ) )
							{
								m_SelectionContext.AddComponent<PhysXRigidbodyComponent>();
							}
						}

						if
							( !m_SelectionContext.HasComponent<PhysXBoxColliderComponent>() 
								&& m_SelectionContext.HasComponent<PhysXRigidbodyComponent>() 
								&& !m_SelectionContext.HasComponent<PhysXSphereColliderComponent>()
								&& !m_SelectionContext.HasComponent<PhysXCapsuleColliderComponent>()
							)
						{
							if( ImGui::MenuItem( "PhysXBoxCollider" ) )
							{
								m_SelectionContext.AddComponent<PhysXBoxColliderComponent>();
							}
						}

						if
							( !m_SelectionContext.HasComponent<PhysXSphereColliderComponent>()
								&& m_SelectionContext.HasComponent<PhysXRigidbodyComponent>() 
								&& !m_SelectionContext.HasComponent<PhysXBoxColliderComponent>()
								&& !m_SelectionContext.HasComponent<PhysXCapsuleColliderComponent>()
							)
						{
							if( ImGui::MenuItem( "PhysXSphereCollider" ) )
							{
								m_SelectionContext.AddComponent<PhysXSphereColliderComponent>();
							}
						}

						if
							( !m_SelectionContext.HasComponent<PhysXCapsuleColliderComponent>()
								&& m_SelectionContext.HasComponent<PhysXRigidbodyComponent>()
								&& !m_SelectionContext.HasComponent<PhysXBoxColliderComponent>()
								&& !m_SelectionContext.HasComponent<PhysXSphereColliderComponent>()
							)
						{
							if( ImGui::MenuItem( "PhysXCapsuleCollider" ) )
							{
								m_SelectionContext.AddComponent<PhysXCapsuleColliderComponent>();
							}
						}

						ImGui::EndMenu();
					}

					ImGui::EndPopup();
				}


				ImGui::End();
			}
		}
		else
		{
			if( ImGui::Begin( "Inspector" ) )
			{
				ImGui::End();
			}
		}
	}

	void SceneHierarchyPanel::DrawEntityNode( Entity entity )
	{
		SAT_PROFILE_FUNCTION();

		if( entity.HasComponent<TagComponent>() )
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			ImGuiTreeNodeFlags flags = ( ( m_SelectionContext == entity ) ? ImGuiTreeNodeFlags_Selected : 0 ) | ImGuiTreeNodeFlags_OpenOnArrow;
			flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx( ( void* )( uint64_t )( uint32_t )entity, flags, tag.c_str() );

			if( ImGui::IsItemClicked() )
			{
				m_SelectionContext = entity;
				canDraw = true;
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

	template<typename T, typename UIFunction>
	static void DrawComponent( const std::string& name, Entity entity, UIFunction uiFunction )
	{
		if( entity.HasComponent<T>() )
		{
			bool removeComponent = false;
			bool removeAllComponent = false;

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

				if( ImGui::MenuItem( "Remove All components" ) )
					removeAllComponent = true;

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
				entity.RemoveComponent<T>();
		}
	}

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition( const glm::mat4& transform )
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose( transform, scale, orientation, translation, skew, perspective );

		return { translation, orientation, scale };
	}

	void DrawBoolControl( const std::string& label, bool* val, float colWidth = 100.0f )
	{
		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2, NULL, false );


		ImGui::SetColumnWidth( 0, colWidth );
		ImGui::Text( "%s", label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 1, ImGui::CalcItemWidth() );
		ImGui::Checkbox( "##value", val );

		ImGui::PopItemWidth();

		ImGui::Columns( 1, NULL, false );

		ImGui::PopID();
	}

	void DrawFloatControl( const std::string& label, float* val, float min = 0.0, float max = 0.0, float step = 1.0f, float colWidth = 100.0f )
	{
		ImGui::PushID( label.c_str() );

		ImGui::Columns( 2, NULL, false );


		ImGui::SetColumnWidth( 0, colWidth );
		ImGui::Text( "%s", label.c_str() );
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths( 1, ImGui::CalcItemWidth() );
		ImGui::DragFloat( "##FLIN", val, step, min, max );

		ImGui::PopItemWidth();

		ImGui::Columns( 1, NULL, false );

		ImGui::PopID();
	}

	void SceneHierarchyPanel::DrawEntityComponents( Entity entity )
	{
		SAT_PROFILE_FUNCTION();

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
		ImGui::TextDisabled( "%f", id );
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		DrawComponent<TransformComponent>( "Transform", entity, []( auto& tc )
			{
				auto& translation = tc.Position;
				auto& rotation = tc.Rotation;
				auto& scale = tc.Scale;

				bool updateTransform = false;
				updateTransform |= DrawVec3Control( "Translation", tc.Position, tc.Position );
				glm::vec3 newRotation = glm::degrees( glm::eulerAngles( tc.Rotation ) );
				updateTransform |= DrawVec3Control( "Rotation", newRotation, newRotation );
				updateTransform |= DrawVec3Control( "Scale", tc.Scale, tc.Scale, 1.0f );

				tc.Rotation = glm::quat( glm::radians( newRotation ) );

				if( updateTransform )
				{
					//TODO: Add
				}
			} );


		DrawComponent<CameraComponent>( "Camera", entity, []( auto& cc )
	{

		ImGui::Columns( 3 );
		ImGui::SetColumnWidth( 0, 100 );
		ImGui::SetColumnWidth( 1, 300 );
		ImGui::SetColumnWidth( 2, 40 );
		ImGui::Text( "Camera" );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );
		ImGui::PopItemWidth();
		ImGui::NextColumn();
			//if( !cc.Camera )
				//cc.Camera = Ref<SceneCamera>::Create( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 10000.0f ) );
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
				if( mc.Mesh )
					ImGui::InputText( "##meshfilepath", ( char* )mc.Mesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly );
				else
					ImGui::InputText( "##meshfilepath", ( char* )"", 256, ImGuiInputTextFlags_ReadOnly );
				ImGui::PopItemWidth();
				ImGui::NextColumn();
				if( ImGui::Button( "...##openmesh", ImVec2( 50, 20 ) ) )
				{
					std::string file = Application::Get().OpenFile( "ObjectFile (*.fbx *.obj)\0*.fbx; *.obj\0" ).first;
					if( !file.empty() )
						mc.Mesh = Ref<Mesh>::Create( file );

				}
			} );

		DrawComponent<BoxColliderComponent>( "Box Collider", entity, []( auto& component )
			{
				DrawVec3Control( "Extents", component.Extents, component.Extents );
			} );

		DrawComponent<SphereColliderComponent>( "Sphere Collider", entity, []( auto& component )
		{
			DrawFloatControl( "Radius", &component.Radius, component.Radius );
		});

		DrawComponent<PhysicsComponent>( "Physics", entity, []( auto& pc )
		{


		});

		DrawComponent<PhysXRigidbodyComponent>( "PhysXRigidbody", entity, []( auto& rb )
			{

				bool Kinematic = rb.isKinematic;
				DrawBoolControl( "Kinematic", &Kinematic );

				rb.isKinematic = Kinematic;

			} );


		DrawComponent<PhysXBoxColliderComponent>( "PhysXBoxCollider", entity, []( auto& bc )
			{

				DrawVec3Control( "Extents", bc.Extents, bc.Extents );

			} );

		DrawComponent<PhysXSphereColliderComponent>( "PhysXSphereCollider", entity, []( auto& sc )
		{

				DrawFloatControl( "Radius", &sc.Radius, sc.Radius );

		} );

		DrawComponent<PhysXCapsuleColliderComponent>( "PhysXCapsuleCollider", entity, []( auto& cc )
		{

			DrawFloatControl( "Radius", &cc.Radius, cc.Radius );
			DrawFloatControl( "Height", &cc.Height, cc.Height );

		} );

		DrawComponent<ScriptComponent>("Script", entity, []( auto& csc )
		{
			std::string name = /*TEMP*/"ExampleApp.Test";

			ImGui::Text( "Module Name:" );
			ImGui::SameLine();
			ImGui::InputText( "##name", (char*)csc.ModuleName.c_str(), 256 );

			auto& fieldMap = ScriptEngine::GetFieldMap();
			if( fieldMap.find( csc.ModuleName ) != fieldMap.end() )
			{
				auto& publicFields = fieldMap.at( csc.ModuleName );
				for( auto& field : publicFields )
				{
					switch( field.Type )
					{
						case FieldType::Int:
						{
							int value = field.GetValue<int>();
							if( Property( field.Name.c_str(), value ) )
							{
								field.SetValue( value );
							}
							break;
						}
						case FieldType::Float:
						{
							float value = field.GetValue<float>();
							if( Property( field.Name.c_str(), value, 0.2f ) )
							{
								field.SetValue( value );
							}
							break;
						}
						case FieldType::Vec2:
						{
							glm::vec2 value = field.GetValue<glm::vec2>();
							if( Property( field.Name.c_str(), value, 0.2f ) )
							{
								field.SetValue( value );
							}
							break;
						}
					}
				}
			}

			

		});

		if( ImGui::Button( "Run Script" ) )
		{
			ScriptEngine::OnCreateEntity( entity );
		}
	}

}