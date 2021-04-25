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

#include "EditorLayer.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

// TEMPORARY
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <Saturn/Log.h>
#include <Saturn/Core/Serialisation/Serialiser.h>

#include <Platform/OpenGL/OpenGLFramebuffer.h>
#include <Saturn/Scene/Components.h>

#include <Saturn/ImGui/ImGuizmo.h>

#include <Saturn/Application.h>
#include <Saturn/Scene/Components.h>
#include <Saturn/Renderer/SceneRenderer.h>
#include <Saturn/Renderer/Renderer2D.h>
#include <Saturn/Renderer/Renderer.h>
#include <Saturn/Core/Base.h>
#include <Saturn/MouseButtons.h>
#include <Saturn/Core/Modules/Module.h>
#include <Saturn/Core/Modules/ModuleManager.h>
#include <Saturn/Scene/SceneManager.h>
#include <Saturn/Script/ScriptEngine.h>
#include <Saturn/Input.h>
#include <Saturn/Physics/PhysX/PhysXFnd.h>

#include <Saturn/Scene/ScriptableEntity.h>

#include <Saturn/ImGui/SceneHierarchyPanel.h>
#include <Saturn/Scene/SceneCamera.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Saturn {

	EditorLayer::EditorLayer() : Layer( "EditorLayer" ), m_EditorCamera( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 10000.0f ) )
	{
		SAT_PROFILE_FUNCTION();
	}

	EditorLayer::~EditorLayer()
	{
		SAT_PROFILE_FUNCTION();
	}

	void EditorLayer::OnAttach()
	{
		SAT_PROFILE_FUNCTION();

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); ( void )io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;      
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>( m_EditorScene );
		m_SceneHierarchyPanel->SetSelectionChangedCallback( std::bind( &EditorLayer::SelectEntity, this, std::placeholders::_1 ) );
		m_AssetPanel = Ref<AssetPanel>::Create();
		m_AssetPanel->OnAttach();

		PhysXFnd::Init();

		OpenScene( "" );

		m_CheckerboardTex = Texture2D::Create( "assets/editor/Checkerboard.tga" );
		m_FooBarTexure = Texture2D::Create( "assets/textures/PlayButton.png" );

		ScriptEngine::Init( "assets/assembly/game/exapp.dll" );
		ScriptEngine::SetSceneContext( m_EditorScene );

		// Setup Platform/Renderer bindings
		ImGui_ImplOpenGL3_Init( "#version 410" );
	}

	void EditorLayer::UpdateWindowTitle( std::string name )
	{
		std::string title = name + " - Saturn - " + Application::GetPlatformName() + " (" + Application::GetConfigurationName() + ")";
		Application::Get().GetWindow().SetTitle( title );
	}

	void EditorLayer::NewScene()
	{
		m_EditorScene = Ref<Scene>::Create();
		m_EditorScene->CreatePhysxScene();
		m_SceneHierarchyPanel->SetContext( m_EditorScene );
		UpdateWindowTitle( "Untitled Scene" );

		m_EditorCamera = EditorCamera( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 1000.0f ) );
	}

	void EditorLayer::OpenScene( )
	{
		Ref<Scene> newScene = Ref<Scene>::Create();
		Serialiser serialiser( newScene );
		std::string filepath = Application::Get().OpenFile( "Scene( *.sc )\0 * .sc\0").first;
		serialiser.Deserialise( filepath );
		m_EditorScene = newScene;
		m_EditorScene->CreatePhysxScene();

		std::filesystem::path path = filepath;
		UpdateWindowTitle( path.filename().string() );
		m_SceneHierarchyPanel->SetContext( m_EditorScene );

		m_EditorScene->SetSelectedEntity( {} );
		m_SelectionContext.clear();
	}

	void EditorLayer::OpenScene( const std::string& filepath )
	{
		m_EditorScene = nullptr;
		Ref<Scene> newScene = Ref<Scene>::Create();
		Serialiser serialiser( newScene );
		if( filepath.empty()) 
		{
			serialiser.Deserialise( "assets/untitled.sc" );
		}
		else
		{
			serialiser.Deserialise( filepath );
		}
		m_EditorScene = newScene;

		std::filesystem::path path = filepath;
		UpdateWindowTitle( path.filename().string() );

		if( filepath.empty() )
			UpdateWindowTitle( "Untitled Scene" );

		m_SceneHierarchyPanel->Reset();
		m_SceneHierarchyPanel->SetContext( m_EditorScene );

		m_EditorScene->SetSelectedEntity( {} );
		m_EditorScene->CreatePhysxScene();
		m_SelectionContext.clear();
	}

	void EditorLayer::DeserialiseDebugLvl()
	{
		Serialiser s( m_EditorScene );
		s.Deserialise( "assets\\test.sc" );
	}

	void EditorLayer::OnDetach()
	{
		SAT_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_AssetPanel->OnDetach();
	}

	void EditorLayer::Begin()
	{
		SAT_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void EditorLayer::End()
	{
		SAT_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2( ( float )app.GetWindow().GetWidth(), ( float )app.GetWindow().GetHeight() );

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent( backup_current_context );
		}
	}

	bool EditorLayer::Property( const std::string& name, bool& value )
	{
		ImGui::Text( name.c_str() );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		std::string id = "##" + name;
		bool result = ImGui::Checkbox( id.c_str(), &value );

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return result;
	}

	bool EditorLayer::Property( const std::string& name, float& value, float min, float max, PropertyFlag flags )
	{
		ImGui::Text( name.c_str() );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		std::string id = "##" + name;
		bool changed = false;
		if( flags == PropertyFlag::SliderProperty )
			changed = ImGui::SliderFloat( id.c_str(), &value, min, max );
		else
			changed = ImGui::DragFloat( id.c_str(), &value, 1.0f, min, max );

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
	}

	bool EditorLayer::Property( const std::string& name, glm::vec2& value, EditorLayer::PropertyFlag flags )
	{
		return Property( name, value, -1.0f, 1.0f, flags );
	}

	bool EditorLayer::Property( const std::string& name, glm::vec2& value, float min, float max, PropertyFlag flags )
	{
		ImGui::Text( name.c_str() );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		std::string id = "##" + name;
		bool changed = false;
		if( flags == PropertyFlag::SliderProperty )
			changed = ImGui::SliderFloat2( id.c_str(), glm::value_ptr( value ), min, max );
		else
			changed = ImGui::DragFloat2( id.c_str(), glm::value_ptr( value ), 1.0f, min, max );

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
	}

	bool EditorLayer::Property( const std::string& name, glm::vec3& value, EditorLayer::PropertyFlag flags )
	{
		return Property( name, value, -1.0f, 1.0f, flags );
	}

	bool EditorLayer::Property( const std::string& name, glm::vec3& value, float min, float max, EditorLayer::PropertyFlag flags )
	{
		ImGui::Text( name.c_str() );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		std::string id = "##" + name;
		bool changed = false;
		if( ( int )flags & ( int )PropertyFlag::ColorProperty )
			changed = ImGui::ColorEdit3( id.c_str(), glm::value_ptr( value ), ImGuiColorEditFlags_NoInputs );
		else if( flags == PropertyFlag::SliderProperty )
			changed = ImGui::SliderFloat3( id.c_str(), glm::value_ptr( value ), min, max );
		else
			changed = ImGui::DragFloat3( id.c_str(), glm::value_ptr( value ), 1.0f, min, max );

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
	}

	bool EditorLayer::Property( const std::string& name, glm::vec4& value, EditorLayer::PropertyFlag flags )
	{
		return Property( name, value, -1.0f, 1.0f, flags );
	}

	bool EditorLayer::Property( const std::string& name, glm::vec4& value, float min, float max, EditorLayer::PropertyFlag flags )
	{
		ImGui::Text( name.c_str() );
		ImGui::NextColumn();
		ImGui::PushItemWidth( -1 );

		std::string id = "##" + name;
		bool changed = false;
		if( ( int )flags & ( int )PropertyFlag::ColorProperty )
			changed = ImGui::ColorEdit4( id.c_str(), glm::value_ptr( value ), ImGuiColorEditFlags_NoInputs );
		else if( flags == PropertyFlag::SliderProperty )
			changed = ImGui::SliderFloat4( id.c_str(), glm::value_ptr( value ), min, max );
		else
			changed = ImGui::DragFloat4( id.c_str(), glm::value_ptr( value ), 1.0f, min, max );

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return changed;
	}

	void EditorLayer::SelectEntity( Entity entity )
	{
		SelectedSubmesh selection;
		if( entity.HasComponent<MeshComponent>() )
		{
			selection.Mesh = &entity.GetComponent<MeshComponent>().Mesh->GetSubmeshes()[ 0 ];
		}
		selection.Entity = entity;
		m_SelectionContext.clear();
		m_SelectionContext.push_back( selection );

		m_EditorScene->SetSelectedEntity( entity );

	}

	void EditorLayer::PrepRuntime()
	{
	}

	void EditorLayer::OnUpdate( Timestep ts )
	{
		m_EditorCamera.OnUpdate( ts );

		//only if we aren't in runtime, we can render editor with the editor camera
		if( !m_RuntimeScene )
		{
			m_EditorScene->OnRenderEditor( ts, m_EditorCamera );
		}
		m_DrawOnTopBoundingBoxes = true;

		if( m_RuntimeScene )
		{
			if( m_RuntimeScene->m_RuntimeRunning )
			{
				Ref<SceneCamera>* mainCamera = nullptr;
				glm::mat4 cameraTransform;
				glm::vec3 cameraPosition;
				{
					auto view = m_RuntimeScene->GetRegistry().view<TransformComponent, CameraComponent>();
					for( auto entity : view )
					{
						auto [transform, camera] = view.get<TransformComponent, CameraComponent>( entity );

						if( camera.Primary )
						{
							mainCamera = &camera.Camera;
							cameraTransform = transform.GetTransform();
							cameraPosition = transform.Position;
							//cameraPosition.x += 5.0f;
							break;
						}
					}
				}

				if( mainCamera != nullptr )
				{
					mainCamera->Raw()->SetPosition( cameraPosition );
					mainCamera->Raw()->OnUpdate( ts );
					m_RuntimeScene->OnRenderRuntime( ts, *mainCamera->Raw() );
				}
				else
				{
					//If we don't have a scene camera we can just copy a editor camera and render the runtime...
					if ( !m_NoSceneCamera )
					{
						SAT_CORE_INFO( "No scene camera was found copying editor camera!" );
						m_NoSceneCamera = Ref<EditorCamera>::Create( m_EditorCamera.GetProjectionMatrix() );
						*m_NoSceneCamera = m_EditorCamera;
					}
					else
					{
						*m_NoSceneCamera = m_EditorCamera;
					}
					m_RuntimeScene->OnRenderEditor( ts, *m_NoSceneCamera );
				}
			}

			m_RuntimeScene->OnUpdate( ts );
		}

		if ( !m_RuntimeScene )
		{
			//For physx and others we will have to half the extents... 
			auto view = m_EditorScene->GetRegistry().view<TransformComponent, PhysXBoxColliderComponent>();
			for( auto entity : view )
			{
				auto [transform, boxCollider] = view.get<TransformComponent, PhysXBoxColliderComponent>( entity );
				boxCollider.Extents = transform.Scale / 2.0f;
			}
		}

		if( !m_RuntimeScene )
		{
			auto viewSphere = m_EditorScene->GetRegistry().view<TransformComponent, PhysXSphereColliderComponent>();
			for( auto entity : viewSphere )
			{
				auto [transform, sphereCollider] = viewSphere.get<TransformComponent, PhysXSphereColliderComponent>( entity );
				sphereCollider.Radius = transform.Scale.y / 2.0f;
			}
		}
	}

	std::pair<float, float> EditorLayer::GetMouseViewportSpace()
	{
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[ 0 ].x;
		my -= m_ViewportBounds[ 0 ].y;
		auto viewportWidth = m_ViewportBounds[ 1 ].x - m_ViewportBounds[ 0 ].x;
		auto viewportHeight = m_ViewportBounds[ 1 ].y - m_ViewportBounds[ 0 ].y;

		return { ( mx / viewportWidth ) * 2.0f - 1.0f, ( ( my / viewportHeight ) * 2.0f - 1.0f ) * -1.0f };
	}

	std::pair<glm::vec3, glm::vec3> EditorLayer::CastRay( float mx, float my )
	{
		glm::vec4 mouseClipPos ={ mx, my, -1.0f, 1.0f };

		auto inverseProj = glm::inverse( m_EditorCamera.GetProjectionMatrix() );
		auto inverseView = glm::inverse( glm::mat3( m_EditorCamera.GetViewMatrix() ) );

		glm::vec4 ray = inverseProj * mouseClipPos;
		glm::vec3 rayPos = m_EditorCamera.GetPosition();
		glm::vec3 rayDir = inverseView * glm::vec3( ray );

		return { rayPos, rayDir };
	}

	Ray EditorLayer::CastMouseRay()
	{
		auto [mouseX, mouseY] = GetMouseViewportSpace();
		if( mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f )
		{
			auto [origin, direction] = CastRay( mouseX, mouseY );
			return Ray( origin, direction );
		}
		return Ray::Zero();
	}

	void EditorLayer::OnEvent( Event& e )
	{
		m_EditorCamera.OnEvent( e );

		if( m_RuntimeScene )
		{
			if( m_RuntimeScene->m_RuntimeRunning )
			{
				Ref<SceneCamera>* mainCamera = nullptr;
				{
					auto view = m_RuntimeScene->GetRegistry().view<TransformComponent, CameraComponent>();
					for( auto entity : view )
					{
						auto [transform, camera] = view.get<TransformComponent, CameraComponent>( entity );

						if( camera.Primary )
						{
							mainCamera = &camera.Camera;
							break;
						}
					}
				}

				if( mainCamera != nullptr )
				{
					//mainCamera->Raw()->SetPosition( cameraPosition );
					mainCamera->Raw()->OnEvent( e );
				}
				
			}

		}

		EventDispatcher dispatcher( e );
		dispatcher.Dispatch<MouseButtonPressedEvent>( SAT_BIND_EVENT_FN( EditorLayer::OnMouseButtonPressed ) );
		dispatcher.Dispatch<KeyPressedEvent>( SAT_BIND_EVENT_FN( EditorLayer::OnKeyPressedEvent ) );

	}

	bool EditorLayer::OnKeyPressedEvent( KeyPressedEvent& e )
	{
		switch( e.GetKeyCode() )
		{
			case SAT_KEY_Q:
				m_GizmoType = -1;
				break;
			case SAT_KEY_E:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case SAT_KEY_W:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case SAT_KEY_R:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			case SAT_KEY_DELETE:
				if( m_SelectionContext.size() )
				{
					Entity selectedEntity = m_SelectionContext[ 0 ].Entity;
					m_SceneHierarchyPanel->canDraw = false;
					m_EditorScene->DestroyEntity( selectedEntity );
					m_SelectionContext.clear();
					m_EditorScene->SetSelectedEntity( {} );
					m_SceneHierarchyPanel->SetSelected( {} );
				}
				break;
		}

		if( Input::IsKeyPressed( SAT_KEY_LEFT_SHIFT ) )
		{
			switch( e.GetKeyCode() )
			{
				case SAT_KEY_S:
					SaveSceneAs();
					break;
				case SAT_KEY_O:
					std::string filepath = Application::Get().OpenFile( "Scene (*.sc)\0*.sc\0" ).first;
					OpenScene( filepath );
					break;
			}
		}

	#ifdef SAT_DEBUG
		if( Input::IsKeyPressed( SAT_KEY_LEFT_SHIFT ) )
		{
			switch( e.GetKeyCode() )
			{
				case SAT_KEY_F:
					DeserialiseDebugLvl();
					break;
			}
		}
	#endif

		if( Input::IsKeyPressed( SAT_KEY_LEFT_CONTROL ) )
		{
			switch( e.GetKeyCode() )
			{
				case SAT_KEY_G:
					// Toggle grid
					SceneRenderer::GetOptions().ShowGrid = !SceneRenderer::GetOptions().ShowGrid;
					break;
				case SAT_KEY_J:
					// Toggle grid
					SceneRenderer::GetOptions().ShowSolids = !SceneRenderer::GetOptions().ShowSolids;
					break;

			}
		}

		return false;

	}

	void EditorLayer::SaveSceneAs()
	{
		auto& app = Application::Get();
		std::string filepath = app.SaveFile( "Scene (*.sc)\0*.sc\0" ).first;
		if( !filepath.empty() )
		{
			Serialiser serializer( m_EditorScene );
			serializer.Serialise( filepath );

			std::filesystem::path path = filepath;
			UpdateWindowTitle( path.filename().string() );
			//m_SceneFilePath = filepath;
		}
	}

	bool EditorLayer::OnMouseButtonPressed( MouseButtonEvent& e )
	{
		auto [mx, my] = Input::GetMousePos();
		if( e.GetMouseButton() == SAT_MOUSE_BUTTON_LEFT && !Input::IsKeyPressed( SAT_KEY_LEFT_ALT ) && !ImGuizmo::IsOver() )
		{
			auto [mouseX, mouseY] = GetMouseViewportSpace();
			if( mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f )
			{
				auto [origin, direction] = CastRay( mouseX, mouseY );
				auto meshEntities = m_EditorScene->GetAllEntitiesWith<MeshComponent>();
				for( auto e : meshEntities )
				{
					Entity entity ={ e, m_EditorScene.Raw() };
					auto mesh = entity.GetComponent<MeshComponent>().Mesh;
					if( !mesh )
						continue;

					auto& submeshes = mesh->GetSubmeshes();
					float lastT = std::numeric_limits<float>::max();
					for( uint32_t i = 0; i < submeshes.size(); i++ )
					{
						auto& submesh = submeshes[ i ];
						Ray ray ={
							glm::inverse( entity.GetComponent<TransformComponent>().GetTransform() * submesh.Transform ) * glm::vec4( origin, 1.0f ),
							glm::inverse( glm::mat3( entity.GetComponent<TransformComponent>().GetTransform() ) * glm::mat3( submesh.Transform ) ) * direction
						};

						float t;
						bool intersects = ray.IntersectsAABB( submesh.BoundingBox, t );
						if( intersects )
						{
							const auto& triangleCache = mesh->GetTriangleCache( i );
							for( const auto& triangle : triangleCache )
							{
								if( ray.IntersectsTriangle( triangle.V0.Position, triangle.V1.Position, triangle.V2.Position, t ) )
								{
									SAT_CORE_WARN( "INTERSECTION: {0}, t={1}", submesh.NodeName, t );
									m_SelectionContext.push_back( { entity, &submesh, t } );
									break;
								}
							}
						}
					}
				}
				std::sort( m_SelectionContext.begin(), m_SelectionContext.end(), []( auto& a, auto& b ) { return a.Distance < b.Distance; } );
				if( m_SelectionContext.size() )
					OnSelected( m_SelectionContext[ 0 ] );
			}
		}

		return false;
	}

	void EditorLayer::OnSelected( const SelectedSubmesh& selectionContext )
	{
		m_SceneHierarchyPanel->SetSelected( selectionContext.Entity );
		m_EditorScene->SetSelectedEntity( selectionContext.Entity );
	}

	float EditorLayer::GetSnapValue()
	{
		switch( m_GizmoType )
		{
			case  ImGuizmo::OPERATION::TRANSLATE: return 0.5f;
			case  ImGuizmo::OPERATION::ROTATE: return 45.0f;
			case  ImGuizmo::OPERATION::SCALE: return 0.5f;
		}
		return 0.0f;
	}

	void EditorLayer::StartImGuiConsole()
	{
		static bool p_open = true;
		ImGuiConsole::OnImGuiRender( &p_open );
	}

	static bool DecomposeTransform( const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale )
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix( transform );

		// Normalize the matrix.
		if( epsilonEqual( LocalMatrix[ 3 ][ 3 ], static_cast< float >( 0 ), epsilon<T>() ) )
			return false;

		// First, isolate perspective.  This is the messiest.
		if
			(
				epsilonNotEqual( LocalMatrix[ 0 ][ 3 ], static_cast< T >( 0 ), epsilon<T>() ) ||
				epsilonNotEqual( LocalMatrix[ 1 ][ 3 ], static_cast< T >( 0 ), epsilon<T>() ) ||
				epsilonNotEqual( LocalMatrix[ 2 ][ 3 ], static_cast< T >( 0 ), epsilon<T>() )
			)
		{
			// Clear the perspective partition
			LocalMatrix[ 0 ][ 3 ] = LocalMatrix[ 1 ][ 3 ] = LocalMatrix[ 2 ][ 3 ] = static_cast< T >( 0 );
			LocalMatrix[ 3 ][ 3 ] = static_cast< T >( 1 );
		}

		// Next take care of translation (easy).
		translation = vec3( LocalMatrix[ 3 ] );
		LocalMatrix[ 3 ] = vec4( 0, 0, 0, LocalMatrix[ 3 ].w );

		vec3 Row[ 3 ], Pdum3;

		// Now get scale and shear.
		for( length_t i = 0; i < 3; ++i )
			for( length_t j = 0; j < 3; ++j )
				Row[ i ][ j ] = LocalMatrix[ i ][ j ];

		// Compute X scale factor and normalize first row.
		scale.x = length( Row[ 0 ] );
		Row[ 0 ] = detail::scale( Row[ 0 ], static_cast< T >( 1 ) );
		scale.y = length( Row[ 1 ] );
		Row[ 1 ] = detail::scale( Row[ 1 ], static_cast< T >( 1 ) );
		scale.z = length( Row[ 2 ] );
		Row[ 2 ] = detail::scale( Row[ 2 ], static_cast< T >( 1 ) );

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
	#if 0
		Pdum3 = cross( Row[ 1 ], Row[ 2 ] ); // v3Cross(row[1], row[2], Pdum3);
		if( dot( Row[ 0 ], Pdum3 ) < 0 )
		{
			for( length_t i = 0; i < 3; i++ )
			{
				scale[ i ] *= static_cast< T >( -1 );
				Row[ i ] *= static_cast< T >( -1 );
			}
		}
	#endif

		rotation.y = asin( -Row[ 0 ][ 2 ] );
		if( cos( rotation.y ) != 0 )
		{
			rotation.x = atan2( Row[ 1 ][ 2 ], Row[ 2 ][ 2 ] );
			rotation.z = atan2( Row[ 0 ][ 1 ], Row[ 0 ][ 0 ] );
		}
		else
		{
			rotation.x = atan2( -Row[ 2 ][ 0 ], Row[ 1 ][ 1 ] );
			rotation.z = 0;
		}


		return true;
	}

	void EditorLayer::OnImGuiRender()
	{
		SAT_PROFILE_FUNCTION();

		static bool p_open = true;

		//auto m_ImConsoleThread = std::thread(&ImGuiConsole::OnImGuiRender);

		static bool opt_fullscreen_persistant = true;
		static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
		bool opt_fullscreen = opt_fullscreen_persistant;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if( opt_fullscreen )
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos( viewport->Pos );
			ImGui::SetNextWindowSize( viewport->Size );
			ImGui::SetNextWindowViewport( viewport->ID );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
			ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		//if (opt_flags & ImGuiDockNodeFlags_PassthruDockspace)
		//	window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
		if( ImGui::Begin( "DockSpace Demo", &p_open, window_flags ) )
		{
			ImGui::PopStyleVar();

			if( opt_fullscreen )
				ImGui::PopStyleVar( 2 );

			// Dockspace
			ImGuiIO& io = ImGui::GetIO();
			if( io.ConfigFlags & ImGuiConfigFlags_DockingEnable )
			{
				ImGuiID dockspace_id = ImGui::GetID( "MyDockspace" );
				ImGui::DockSpace( dockspace_id, ImVec2( 0.0f, 0.0f ), opt_flags );
			}

			if(ImGui::BeginMainMenuBar()) 
			{
				if (ImGui::BeginMenu("File"))
				{
					if( ImGui::MenuItem( "New", "Shift+N" ) )
						NewScene();

					if( ImGui::MenuItem( "Open...", "Shift+O" ) )
						OpenScene();

					if( ImGui::MenuItem( "Save As...", "Shift+S" ) )
						SaveSceneAs();
					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}

			m_SceneHierarchyPanel->OnImGuiRender();

			m_ImGuiConsole_Thread = std::thread( &EditorLayer::StartImGuiConsole, this );
			// Editor Panel ------------------------------------------------------------------------------
			m_ImGuiConsole_Thread.join();

			m_AssetPanel->OnImGuiRender();


			ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 12, 0 ) );
			ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 12, 4 ) );
			ImGui::PushStyleVar( ImGuiStyleVar_ItemInnerSpacing, ImVec2( 0, 0 ) );
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.8f, 0.8f, 0.8f, 0.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );
			if( ImGui::Begin( "Toolbar" ) )
			{
			#if 0
				if( !m_RuntimeScene )
				{
					if( ImGui::ImageButton( ( ImTextureID )( m_FooBarTexure->GetRendererID() ), ImVec2( 50, 50 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), -1, ImVec4( 0, 0, 0, 0 ), ImVec4( 0.9f, 0.9f, 0.9f, 1.0f ) ) )
					{
						m_EditorScene->SetSelectedEntity( {} );
						m_SceneHierarchyPanel->SetSelected( {} );
						m_SelectionContext.clear();
						m_SceneHierarchyPanel->Reset();
						m_RuntimeScene = Ref<Scene>::Create();
						m_SceneHierarchyPanel->SetContext( m_RuntimeScene );
						m_EditorScene->CopyScene( m_RuntimeScene );
						m_RuntimeScene->BeginRuntime();
					}
				}

				if( m_RuntimeScene )
				{
					if( m_RuntimeScene->m_RuntimeRunning )
					{
						ImGui::SameLine();

						if( ImGui::ImageButton( ( ImTextureID )( m_FooBarTexure->GetRendererID() ), ImVec2( 50, 50 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), -1, ImVec4( 1.0f, 1.0f, 1.0f, 0.2f ) ) )
						{
							m_RuntimeScene->SetSelectedEntity( {} );
							m_SceneHierarchyPanel->SetSelected( {} );
							m_SelectionContext.clear();
							m_SceneHierarchyPanel->Reset();
							m_RuntimeScene->EndRuntime();
							m_SelectionContext.clear();
							m_SceneHierarchyPanel->SetContext( m_EditorScene );
							//delete m_RuntimeScene.Raw();
						}
					}
				}
			#endif

				if( !m_RuntimeScene )
				{

					if( ImGui::ImageButton( ( ImTextureID )( m_FooBarTexure->GetRendererID() ), ImVec2( 50, 50 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), -1, ImVec4( 0, 0, 0, 0 ), ImVec4( 0.9f, 0.9f, 0.9f, 1.0f ) ) )
					{
						m_SceneHierarchyPanel->Reset();
						m_EditorScene->SetSelectedEntity( {} );
						m_SceneHierarchyPanel->SetSelected( {} );
						m_SelectionContext.clear();
						m_RuntimeScene = Ref<Scene>::Create();
						m_SceneHierarchyPanel->SetContext( m_RuntimeScene );
						m_EditorScene->CopyScene( m_RuntimeScene );
						m_RuntimeScene->BeginRuntime();
					}
				}

				if( m_RuntimeScene )
				{
					if( m_RuntimeScene->m_RuntimeRunning )
					{
						ImGui::SameLine();

						if( ImGui::ImageButton( ( ImTextureID )( m_FooBarTexure->GetRendererID() ), ImVec2( 50, 50 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), -1, ImVec4( 1.0f, 1.0f, 1.0f, 0.2f ) ) )
						{
							m_SceneHierarchyPanel->Reset();
							m_RuntimeScene->SetSelectedEntity( {} );
							m_SceneHierarchyPanel->SetSelected( {} );
							m_NoSceneCamera = nullptr;
							m_RuntimeScene->EndRuntime();
							m_RuntimeScene = nullptr;
							m_SelectionContext.clear();
							m_SceneHierarchyPanel->SetContext( m_EditorScene );
						}
					}
				}

				ImGui::End();
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			if( ImGui::Begin( "Model" ) )
			{
				if( ImGui::Begin( "Environment" ) )
				{

					if( ImGui::Button( "Load Environment Map" ) )
					{
						std::string filename = Application::Get().OpenFile( "HDR (*.hdr)\0 * .hdr\0" ).first;
						if( filename != "" )
							m_EditorScene->SetEnvironment( Environment::Load( filename ) );
					}
					ImGui::SliderFloat( "Skybox LOD", &m_EditorScene->GetSkyboxLod(), 0.0f, 11.0f );

					ImGui::Columns( 2 );
					ImGui::AlignTextToFramePadding();

					auto& light = m_EditorScene->GetLight();
					Property( "Light Direction", light.Direction, PropertyFlag::SliderProperty );
					Property( "Light Radiance", light.Radiance, PropertyFlag::ColorProperty );
					Property( "Light Multiplier", light.Multiplier, 0.0f, 5.0f, PropertyFlag::SliderProperty );

					Property( "Exposure", m_EditorCamera.GetExposure(), 0.0f, 5.0f, PropertyFlag::SliderProperty );

					char* label = "Entity";
					if( ImGui::Button( label ) )
					{
						m_SelectionMode = SelectionMode::Entity;
					}

				}
				ImGui::End();

			}
			ImGui::End();

			ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
			if( ImGui::Begin( "Viewport" ) )
			{
				auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
				auto viewportSize = ImGui::GetContentRegionAvail();
				SceneRenderer::SetViewportSize( ( uint32_t )viewportSize.x, ( uint32_t )viewportSize.y );
				m_EditorScene->SetViewportSize( ( uint32_t )viewportSize.x, ( uint32_t )viewportSize.y );
				if( m_RuntimeScene )
					m_RuntimeScene->SetViewportSize( ( uint32_t )viewportSize.x, ( uint32_t )viewportSize.y );
				m_EditorCamera.SetProjectionMatrix( glm::perspectiveFov( glm::radians( 45.0f ), viewportSize.x, viewportSize.y, 0.1f, 10000.0f ) );
				m_EditorCamera.SetViewportSize( ( uint32_t )viewportSize.x, ( uint32_t )viewportSize.y );
				ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 2, 2 ) );
				ImGui::Image( ( void* )SceneRenderer::GetFinalColorBufferRendererID(), viewportSize, { 0, 1 }, { 1, 0 } );
				ImGui::PopStyleVar();

				static int counter = 0;
				auto windowSize = ImGui::GetWindowSize();
				ImVec2 minBound = ImGui::GetWindowPos();
				minBound.x += viewportOffset.x;
				minBound.y += viewportOffset.y;

				ImVec2 maxBound ={ minBound.x + windowSize.x, minBound.y + windowSize.y };
				m_ViewportBounds[ 0 ] ={ minBound.x, minBound.y };
				m_ViewportBounds[ 1 ] ={ maxBound.x, maxBound.y };
				m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect( minBound, maxBound );

				// Gizmos
				if( m_GizmoType != -1 && m_SelectionContext.size() )
				{
					auto& selection = m_SelectionContext[ 0 ];

					float rw = ( float )ImGui::GetWindowWidth();
					float rh = ( float )ImGui::GetWindowHeight();
					ImGuizmo::SetOrthographic( false );
					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect( ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh );

					bool snap = Input::IsKeyPressed( SAT_KEY_LEFT_CONTROL );


					auto& tc = selection.Entity.GetComponent<TransformComponent>();

					glm::mat4 position = glm::translate( glm::mat4( 1.0f ), tc.Position );
					glm::mat4 rotation = glm::toMat4( tc.Rotation );
					glm::mat4 scale = glm::scale( glm::mat4( 1.0f ), tc.Scale );

					glm::mat4 entityTransform = position * rotation * scale;

					float* et = glm::value_ptr( entityTransform );

					float snapValue = GetSnapValue();
					float snapValues[ 3 ] ={ snapValue, snapValue, snapValue };

					if( m_SelectionMode == SelectionMode::Entity )
					{
						auto viewm = glm::value_ptr( m_EditorCamera.GetViewMatrix() );
						auto projm = glm::value_ptr( m_EditorCamera.GetProjectionMatrix() );

						Entity selectedEntity = m_SceneHierarchyPanel->GetSelectionContext();
						selectedEntity.m_Scene = m_SceneHierarchyPanel->GetSelectionContext().m_Scene;

						auto& tc = selectedEntity.GetComponent<TransformComponent>();
						glm::mat4 transform = tc.GetTransform();
						ImGuizmo::Manipulate( viewm, projm, ( ImGuizmo::OPERATION )m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr( transform ), nullptr, snap ? snapValues : nullptr );


						if( ImGuizmo::IsUsing() )
						{
							glm::vec3 translation, scale;
							glm::quat rotation;
							DecomposeTransform( transform, translation, rotation, scale );

							glm::quat deltaRotation = rotation - tc.Rotation;
							tc.Position = translation;
							tc.Rotation += deltaRotation;
							tc.Scale = scale;
						}

					}
				}
			}
			ImGui::PopStyleVar();

			ImGui::Begin( "Materials" );

			if( m_SelectionContext.size() )
			{
				Entity selectedEntity = m_SelectionContext.front().Entity;
				//m_SceneHierarchyPanel->SetSelected( selectedEntity );
				if( selectedEntity.HasComponent<MeshComponent>() )
				{
					Ref<Mesh> mesh = selectedEntity.GetComponent<MeshComponent>().Mesh;
					if( mesh )
					{
						auto& materials = mesh->GetMaterials();
						static uint32_t selectedMaterialIndex = 0;
						for( uint32_t i = 0; i < materials.size(); i++ )
						{
							auto& materialInstance = materials[ i ];

							ImGuiTreeNodeFlags node_flags = ( selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0 ) | ImGuiTreeNodeFlags_Leaf;
							bool opened = ImGui::TreeNodeEx( ( void* )( &materialInstance ), node_flags, materialInstance->GetName().c_str() );
							if( ImGui::IsItemClicked() )
							{
								selectedMaterialIndex = i;
							}
							if( opened )
								ImGui::TreePop();

						}

						ImGui::Separator();

						/**
						* Selected material -> view and edit the stuff
						*/
						if( selectedMaterialIndex < materials.size() )
						{
							auto& materialInstance = materials[ selectedMaterialIndex ];
							ImGui::Text( "Shader: %s", materialInstance->GetShader()->GetName().c_str() );
							// Textures ------------------------------------------------------------------------------
							{
								// Albedo
								if( ImGui::CollapsingHeader( "Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen ) )
								{
									ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10, 10 ) );

									auto& albedoColor = materialInstance->Get<glm::vec3>( "u_AlbedoColor" );
									bool useAlbedoMap = materialInstance->Get<float>( "u_AlbedoTexToggle" );
									Ref<Texture2D> albedoMap = materialInstance->TryGetResource<Texture2D>( "u_AlbedoTexture" );
									ImGui::Image( albedoMap ? ( void* )albedoMap->GetRendererID() : ( void* )m_CheckerboardTex->GetRendererID(), ImVec2( 64, 64 ) );
									ImGui::PopStyleVar();
									if( ImGui::IsItemHovered() )
									{
										if( albedoMap )
										{
											ImGui::BeginTooltip();
											ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
											ImGui::TextUnformatted( albedoMap->GetPath().c_str() );
											ImGui::PopTextWrapPos();
											ImGui::Image( ( void* )albedoMap->GetRendererID(), ImVec2( 384, 384 ) );
											ImGui::EndTooltip();
										}
										if( ImGui::IsItemClicked() )
										{
											std::string filename = Application::Get().OpenFile( "" ).first;
											if( filename != "" )
											{
												albedoMap = Texture2D::Create( filename, true/*m_AlbedoInput.SRGB*/ );
												materialInstance->Set( "u_AlbedoTexture", albedoMap );
											}
										}
									}
									ImGui::SameLine();
									ImGui::BeginGroup();
									if( ImGui::Checkbox( "Use##AlbedoMap", &useAlbedoMap ) )
										materialInstance->Set<float>( "u_AlbedoTexToggle", useAlbedoMap ? 1.0f : 0.0f );

									/*if (ImGui::Checkbox("sRGB##AlbedoMap", &m_AlbedoInput.SRGB))
									{
									if (m_AlbedoInput.TextureMap)
									m_AlbedoInput.TextureMap = Texture2D::Create(m_AlbedoInput.TextureMap->GetPath(), m_AlbedoInput.SRGB);
									}*/
									ImGui::EndGroup();
									ImGui::SameLine();
									ImGui::ColorEdit3( "Color##Albedo", glm::value_ptr( albedoColor ), ImGuiColorEditFlags_NoInputs );
								}
							}
							{
								// Normals
								if( ImGui::CollapsingHeader( "Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen ) )
								{
									ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10, 10 ) );
									bool useNormalMap = materialInstance->Get<float>( "u_NormalTexToggle" );
									Ref<Texture2D> normalMap = materialInstance->TryGetResource<Texture2D>( "u_NormalTexture" );
									ImGui::Image( normalMap ? ( void* )normalMap->GetRendererID() : ( void* )m_CheckerboardTex->GetRendererID(), ImVec2( 64, 64 ) );
									ImGui::PopStyleVar();
									if( ImGui::IsItemHovered() )
									{
										if( normalMap )
										{
											ImGui::BeginTooltip();
											ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
											ImGui::TextUnformatted( normalMap->GetPath().c_str() );
											ImGui::PopTextWrapPos();
											ImGui::Image( ( void* )normalMap->GetRendererID(), ImVec2( 384, 384 ) );
											ImGui::EndTooltip();
										}
										if( ImGui::IsItemClicked() )
										{
											std::string filename = Application::Get().OpenFile( "" ).first;
											if( filename != "" )
											{
												normalMap = Texture2D::Create( filename );
												materialInstance->Set( "u_NormalTexture", normalMap );
											}
										}
									}
									ImGui::SameLine();
									if( ImGui::Checkbox( "Use##NormalMap", &useNormalMap ) )
										materialInstance->Set<float>( "u_NormalTexToggle", useNormalMap ? 1.0f : 0.0f );
								}
							}
							{
								// Metalness
								if( ImGui::CollapsingHeader( "Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen ) )
								{
									ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10, 10 ) );
									float& metalnessValue = materialInstance->Get<float>( "u_Metalness" );
									bool useMetalnessMap = materialInstance->Get<float>( "u_MetalnessTexToggle" );
									Ref<Texture2D> metalnessMap = materialInstance->TryGetResource<Texture2D>( "u_MetalnessTexture" );
									ImGui::Image( metalnessMap ? ( void* )metalnessMap->GetRendererID() : ( void* )m_CheckerboardTex->GetRendererID(), ImVec2( 64, 64 ) );
									ImGui::PopStyleVar();
									if( ImGui::IsItemHovered() )
									{
										if( metalnessMap )
										{
											ImGui::BeginTooltip();
											ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
											ImGui::TextUnformatted( metalnessMap->GetPath().c_str() );
											ImGui::PopTextWrapPos();
											ImGui::Image( ( void* )metalnessMap->GetRendererID(), ImVec2( 384, 384 ) );
											ImGui::EndTooltip();
										}
										if( ImGui::IsItemClicked() )
										{
											std::string filename = Application::Get().OpenFile( "" ).first;
											if( filename != "" )
											{
												metalnessMap = Texture2D::Create( filename );
												materialInstance->Set( "u_MetalnessTexture", metalnessMap );
											}
										}
									}
									ImGui::SameLine();
									if( ImGui::Checkbox( "Use##MetalnessMap", &useMetalnessMap ) )
										materialInstance->Set<float>( "u_MetalnessTexToggle", useMetalnessMap ? 1.0f : 0.0f );
									ImGui::SameLine();
									ImGui::SliderFloat( "Value##MetalnessInput", &metalnessValue, 0.0f, 1.0f );
								}
							}
							{
								// Roughness
								if( ImGui::CollapsingHeader( "Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen ) )
								{
									ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10, 10 ) );
									float& roughnessValue = materialInstance->Get<float>( "u_Roughness" );
									bool useRoughnessMap = materialInstance->Get<float>( "u_RoughnessTexToggle" );
									Ref<Texture2D> roughnessMap = materialInstance->TryGetResource<Texture2D>( "u_RoughnessTexture" );
									ImGui::Image( roughnessMap ? ( void* )roughnessMap->GetRendererID() : ( void* )m_CheckerboardTex->GetRendererID(), ImVec2( 64, 64 ) );
									ImGui::PopStyleVar();
									if( ImGui::IsItemHovered() )
									{
										if( roughnessMap )
										{
											ImGui::BeginTooltip();
											ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
											ImGui::TextUnformatted( roughnessMap->GetPath().c_str() );
											ImGui::PopTextWrapPos();
											ImGui::Image( ( void* )roughnessMap->GetRendererID(), ImVec2( 384, 384 ) );
											ImGui::EndTooltip();
										}
										if( ImGui::IsItemClicked() )
										{
											std::string filename = Application::Get().OpenFile( "" ).first;
											if( filename != "" )
											{
												roughnessMap = Texture2D::Create( filename );
												materialInstance->Set( "u_RoughnessTexture", roughnessMap );
											}
										}
									}
									ImGui::SameLine();
									if( ImGui::Checkbox( "Use##RoughnessMap", &useRoughnessMap ) )
										materialInstance->Set<float>( "u_RoughnessTexToggle", useRoughnessMap ? 1.0f : 0.0f );
									ImGui::SameLine();
									ImGui::SliderFloat( "Value##RoughnessInput", &roughnessValue, 0.0f, 1.0f );
								}
							}
						}

					}
				}
			}

			ImGui::End();


			ImGui::End();
		}

		ImGui::End();

		ImGuizmo::BeginFrame();

	}
}