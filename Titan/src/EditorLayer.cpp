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
#include <Saturn/Scene/SceneManager.h>
#include <Saturn/Script/ScriptEngine.h>
#include <Saturn/Input.h>
#include <Saturn/Physics/PhysX/PhysXFnd.h>

#include <Saturn/Scene/ScriptableEntity.h>

#include <Saturn/ImGui/SceneHierarchyPanel.h>
#include <Saturn/Scene/SceneCamera.h>

#include <Saturn/Core/FileSystemHelpers.h>
#include <Saturn/Core/Assets/FileCollection.h>
#include <Saturn/Core/Assets/PNGFile.h>

#include <Saturn/Core/Math.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <FontAwseome/IconsForkAwesome.h>

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

		m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>( m_EditorScene );
		m_SceneHierarchyPanel->SetSelectionChangedCallback( std::bind( &EditorLayer::SelectEntity, this, std::placeholders::_1 ) );

		m_AssetPanel = Ref<AssetPanel>::Create();
		m_TextureViewerPanel = Ref<TextureViewer>::Create();
		m_ScriptViewerStandalone = Ref<ScriptViewerStandalone>::Create();

		m_AssetPanel->OnAttach();
		m_ScriptViewerStandalone->OnAttach();
		m_TextureViewerPanel->OnAttach();

		PhysXFnd::Init();

		ProjectSettings::Load();
		OpenScene( ProjectSettings::GetStartupSceneName() );

		m_CheckerboardTex = Texture2D::Create( "assets/editor/Checkerboard.tga" );
		m_PlayButtonTexture = Texture2D::Create( "assets/textures/PlayButton.png" );
		m_PauseButtonTexture = Texture2D::Create( "assets/textures/PauseButton.png" );
		m_FileSceneTexture = Texture2D::Create( "assets/.github/i/sat/SaturnLogov2.png" );
		m_StopButtonTexture = Texture2D::Create( "assets/textures/StopButton.png" );

		m_UnkownFile = Texture2D::Create( "assets/textures/assetpanel/unkown_file.png" );
		m_TextFile = Texture2D::Create( "assets/textures/assetpanel/text_file.png" );

		ScriptEngine::Init( "assets/assembly/ExampleApp.dll" );
		ScriptEngine::SetSceneContext( m_EditorScene );

	}

	void EditorLayer::UpdateWindowTitle( std::string name )
	{
		uint64_t uuid  = Application::Get().GetFixedVersionUUID();
		std::string branch  = Application::Get().GetVersionCtrl().Branch;
		std::string uuidstr = std::to_string( uuid );

		if( name.empty() )
			name = "Unnamed Scene";

		std::string title = name + " - Saturn - " + Application::GetPlatformName() + " (" + Application::GetConfigurationName() + ")" + "," + " (" + " " + uuidstr + " " + "/" + " " + branch + " )";
		Application::Get().GetWindow().SetTitle( title );
	}

	void EditorLayer::NewScene()
	{
		m_EditorScene->SetEnvironment( {} );
		m_EditorScene = Ref<Scene>::Create();
		m_EditorScene->CreatePhysxScene();
		m_SceneHierarchyPanel->SetContext( m_EditorScene );
		UpdateWindowTitle( "Unnamed Scene" );

		m_EditorCamera = EditorCamera( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 1000.0f ) );
	}

	void EditorLayer::OpenScene()
	{
		Ref<Scene> newScene = Ref<Scene>::Create();
		Serialiser serialiser( newScene );
		std::string filepath = Application::Get().OpenFile( "Scene( *.sc )\0 * .sc\0" ).first;
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
		if( filepath.empty() )
		{
			serialiser.Deserialise( "assets/untitled.sc" );
		}
		else
		{
			serialiser.Deserialise( filepath );
		}
		m_EditorScene = newScene;

		if( FileSystem::DoesFileExist( "", "version-control.vcinfo" ) )
		{
			Serialiser s( m_EditorScene );
			s.DeserialiseVC( "version-control.vcinfo" );
		}
		else
		{
			Serialiser s( m_EditorScene );
			s.SerialiseVC( "version-control.vcinfo" );
		}


		std::filesystem::path path = filepath;
		UpdateWindowTitle( path.filename().string() );

		m_SceneHierarchyPanel->Reset();
		m_SceneHierarchyPanel->SetContext( m_EditorScene );

		m_EditorScene->SetSelectedEntity( {} );
		m_SelectionContext.clear();
		m_AssetPanel->m_CurrentScene = m_EditorScene;
	}

	void EditorLayer::DeserialiseDebugLvl()
	{
		Serialiser s( m_EditorScene );
		s.Deserialise( "assets\\test.sc" );
	}

	void EditorLayer::OnDetach()
	{
		SAT_PROFILE_FUNCTION();

		m_AssetPanel->OnDetach();
		m_TextureViewerPanel->OnDetach();
		m_ScriptViewerStandalone->OnDetach();
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
		m_SceneHierarchyPanel->SetSelected( {} );
		m_SceneHierarchyPanel->SetSelected( entity );
	}

	void EditorLayer::PrepRuntime()
	{
	}

	void EditorLayer::OnUpdate( Timestep ts )
	{
		m_DrawOnTopBoundingBoxes = true;

		//Only if we aren't in runtime, we can render editor with the editor camera
		if( !m_RuntimeScene )
		{
			m_EditorCamera.SetActive( m_AllowViewportCameraEvents || glfwGetInputMode( static_cast< GLFWwindow* >( Application::Get().GetWindow().GetNativeWindow() ), GLFW_CURSOR ) == GLFW_CURSOR_DISABLED );

			if( m_AllowViewportCameraEvents )
				m_EditorCamera.OnUpdate( ts );

			m_EditorScene->OnRenderEditor( ts, m_EditorCamera );
		}

		if( m_RuntimeScene )
		{
			if( m_RuntimeScene->m_RuntimeRunning )
			{
				m_RuntimeScene->OnUpdate( ts );
				m_RuntimeScene->OnRenderRuntime( ts );
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
		if( m_AllowViewportCameraEvents )
			m_EditorCamera.OnEvent( e );

		EventDispatcher dispatcher( e );
		dispatcher.Dispatch<MouseButtonPressedEvent>( SAT_BIND_EVENT_FN( EditorLayer::OnMouseButtonPressed ) );
		dispatcher.Dispatch<KeyPressedEvent>( SAT_BIND_EVENT_FN( EditorLayer::OnKeyPressedEvent ) );
	}

	bool EditorLayer::OnKeyPressedEvent( KeyPressedEvent& e )
	{
		switch( e.GetKeyCode() )
		{
			case Key::Q:
				m_GizmoType = -1;
				break;
			case Key::E:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Key::W:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case Key::R:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			case Key::Delete:
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

		if( Input::IsKeyPressed( Key::LeftControl ) )
		{
			switch( e.GetKeyCode() )
			{
				case Key::G:
					// Toggle grid
					SceneRenderer::GetOptions().ShowGrid = !SceneRenderer::GetOptions().ShowGrid;
					break;
				case Key::J:
					// Toggle solids
					SceneRenderer::GetOptions().ShowSolids = !SceneRenderer::GetOptions().ShowSolids;
					break;
				case Key::D:
					if( m_SelectionContext.size() )
						m_EditorScene->DuplicateEntity( m_SceneHierarchyPanel->GetSelectionContext() );
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
		auto [mx, my] = Input::GetMousePosition();
		if( e.GetMouseButton() == Mouse::Left && !Input::IsKeyPressed( Key::LeftAlt ) && !ImGuizmo::IsOver() )
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
				//m_SelectionContext.clear();
			}
		}

		//m_SelectionContext.clear();

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

	static void DrawAssetText( const char* text )
	{
		ImVec2 rectMin = ImGui::GetItemRectMin();
		ImVec2 rectSize = ImGui::GetItemRectSize();
		ImVec2 textSize = ImGui::CalcTextSize( text );

		static ImVec2 padding( 5, 5 );
		static int marginTop = 15;

		ImDrawList& windowDrawList = *ImGui::GetWindowDrawList();
		ImGuiStyle& style = ImGui::GetStyle();

		if( textSize.x + padding.x * 2 <= rectSize.x )
		{
			float rectMin_x = rectMin.x - padding.x + ( rectSize.x - textSize.x ) / 2;
			float rectMin_y = rectMin.y + rectSize.y + marginTop;

			float rectMax_x = rectMin_x + textSize.x + padding.x * 2;
			float rectMax_y = rectMin_y + textSize.y + padding.y * 2;

			windowDrawList.AddRectFilled( { rectMin_x, rectMin_y }, { rectMax_x, rectMax_y }, ImColor( ImVec4( 0.18f, 0.18f, 0.18f, 1.0f ) ), 0 );

			windowDrawList.AddText( { rectMin_x + padding.x, rectMin_y + padding.y }, ImColor( 1.0f, 1.0f, 1.0f ), text );
		}
		else
		{
			float rectMin_y = rectMin.y + rectSize.y + marginTop;

			float rectMax_x = rectMin.x + rectSize.x;
			float rectMax_y = rectMin_y + textSize.y + padding.y * 2;

			windowDrawList.AddRectFilled( { rectMin.x, rectMin_y }, { rectMax_x, rectMax_y }, ImColor( ImVec4( 0.18f, 0.18f, 0.18f, 1.0f ) ), 0 );

			rectMax_x -= padding.x;
			rectMax_y -= padding.y;

			ImGui::RenderTextEllipsis( &windowDrawList, { rectMin.x + padding.x, rectMin_y + padding.y }, { rectMax_x, rectMax_y }, rectMax_x, rectMax_x + 5, text, nullptr, &textSize );
		}
	}

	//TODO: Add back into the AssetLayer.cpp file
	void EditorLayer::StartAssetLayer()
	{
		bool p_open = true;
		namespace fs = std::filesystem;

		std::string asset_path = "assets";

		int selected = ( 1 << 2 );

		unsigned int idx;

		if( ImGui::Begin( "AssetPanel", &p_open ) )
		{
			std::string m_FolderPath = "abc..";

			for( fs::recursive_directory_iterator it( asset_path ); it != fs::recursive_directory_iterator(); ++it )
			{
				if( fs::is_directory( it->path() ) )
				{
					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Indent();
					}

					if( ImGui::TreeNode( it->path().filename().string().c_str() ) )
					{
						ImGui::TreePop();
					}

					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Unindent();
					}

					if( !it->path().has_extension() )
					{
						m_FolderPath = it->path().string();
					}

				}
				else
				{
					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Indent();
					}

					for( int i = 0; i < it.depth(); ++i )
					{
						ImGui::Unindent();
					}
				}
			}
		}
		ImGui::End();

		if( ImGui::Begin( "Assets", &p_open ) )
		{
			auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();

			auto viewportOffset = ImGui::GetWindowPos(); // includes tab bar
			auto viewportSize = ImGui::GetContentRegionAvail();

			m_FolderPath = ProjectSettings::GetCurrentProject()->GetAssetsFolderPath();

			ImGui::Text( "File Path: %s", m_FolderPath.c_str() );

			if( m_FolderPath == "assets" )
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
				ImGui::Button( "Back" );
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
			else
			{
				if( ImGui::Button( "Back" ) )
				{
					fs::path currentPath( m_FolderPath );
					fs::path root_path = currentPath.parent_path();

					m_FolderPath = root_path.string();

				}

			}

			if( ImGui::Button( "Scan All Folders for Assets" ) )
			{
				for( fs::recursive_directory_iterator it( m_FolderPath ); it != fs::recursive_directory_iterator(); ++it )
				{
					if( it->path().extension().string() == ".sc" )
					{
						MakeFileFrom<File>( it->path().filename().string(), it->path().string(), FileExtensionType::SCENE );
					}

					if( it->path().extension().string() == ".txt" )
					{
						MakeFileFrom<File>( it->path().filename().string(), it->path().string(), FileExtensionType::TEXT );
					}

					if( it->path().extension().string() == ".png" )
					{
						MakeFileFrom<PNGFile>( it->path().filename().string(), it->path().string(), FileExtensionType::PNG );
					}

					if( it->path().extension().string() == ".tga" )
					{
						MakeFileFrom<PNGFile>( it->path().filename().string(), it->path().string(), FileExtensionType::TGA );
					}

					if( it->path().extension().string() == ".obj" )
					{
						MakeFileFrom<File>( it->path().filename().string(), it->path().string(), FileExtensionType::OBJ );
					}

					if( it->path().extension().string() == ".fbx" )
					{
						MakeFileFrom<File>( it->path().filename().string(), it->path().string(), FileExtensionType::FBX );
					}

					if( it->path().extension().string() == ".cs" )
					{
						MakeFileFrom<File>( it->path().filename().string(), it->path().string(), FileExtensionType::SCRIPT );
					}

					if( it->path().extension().string() == ".glsl" )
					{
						MakeFileFrom<File>( it->path().filename().string(), it->path().string(), FileExtensionType::SHADER );
					}

					if( it->path().extension().string() == ".hdr" )
					{
						MakeFileFrom<PNGFile>( it->path().filename().string(), it->path().string(), FileExtensionType::HDR );
					}
				}
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			for( fs::directory_iterator it( m_FolderPath ); it != fs::directory_iterator(); ++it )
			{
				if( !it->path().has_extension() )
				{
					ImVec2 imageSize( 64, 64 );

					ImGui::ManualWrapBegin( imageSize );
					if( ImGui::Button( it->path().filename().string().c_str(), imageSize ) )
					{
						m_CurrentFolder = it->path().filename().string().c_str();
						m_FolderPath = m_FolderPath + "\\" + it->path().filename().string();
					}

					DrawAssetText( it->path().filename().string().c_str() );

					ImGui::ManualWrapEnd( imageSize );
				}
			}

			for( fs::directory_iterator it( m_FolderPath ); it != fs::directory_iterator(); ++it )
			{
				if( it->path().has_extension() )
				{
					if( it->path().extension().string() == ".sc" )
					{
						std::string path = m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<File>( it->path().filename().string(), path, FileExtensionType::SCENE );

						ImVec2 imageSize( 64, 64 );

						if( m_RuntimeScene )
						{
							ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
							ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );
							ImGui::ManualWrapBegin( imageSize );
							ImGui::Button( it->path().filename().string().c_str(), ImVec2( 64, 64 ) );
							ImGui::ManualWrapEnd( imageSize );
							ImGui::PopItemFlag();
							ImGui::PopStyleVar();
						}
						else
						{
							ImGui::ManualWrapBegin( imageSize );
							if( ImGui::Button( it->path().filename().string().c_str(), ImVec2( 64, 64 ) ) )
							{
								OpenScene( path );
							}

							DrawAssetText( it->path().filename().string().c_str() );

							ImGui::ManualWrapEnd( imageSize );
						}

					}

					if( it->path().extension().string() == ".png" )
					{
						std::string path = m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<PNGFile>( it->path().filename().string(), path, FileExtensionType::PNG );

						Ref<PNGFile> file = FileCollection::GetFile( it->path().filename().string() );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::ImageButton( ( ImTextureID )file->GetData()->GetRendererID(), imageSize ) )
						{
							m_TextureViewerPanel->ShowWindowAgain();
							m_TextureViewerPanel->Reset();
							TextureViewer::SetRenderImageTarget( file->GetData() );
						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );
					}

					if( it->path().extension().string() == ".tga" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<PNGFile>( it->path().filename().string(), path, FileExtensionType::TGA );

						Ref<PNGFile> file = FileCollection::GetFile( it->path().filename().string() );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::ImageButton( ( ImTextureID )file->GetData()->GetRendererID(), imageSize ) )
						{
							m_TextureViewerPanel->ShowWindowAgain();
							m_TextureViewerPanel->Reset();
							TextureViewer::SetRenderImageTarget( file->GetData() );
						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );
					}

					if( it->path().extension().string() == ".hdr" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<PNGFile>( it->path().filename().string(), path, FileExtensionType::HDR );

						Ref<PNGFile> file = FileCollection::GetFile( it->path().filename().string() );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::ImageButton( ( ImTextureID )file->GetData()->GetRendererID(), imageSize ) )
						{
							m_TextureViewerPanel->ShowWindowAgain();
							m_TextureViewerPanel->Reset();
							TextureViewer::SetRenderImageTarget( file->GetData() );
						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );

					}

					if( it->path().extension().string() == ".cs" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<File>( it->path().filename().string(), path, FileExtensionType::SCRIPT );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::Button( it->path().filename().string().c_str(), imageSize ) )
						{
							m_ScriptViewerStandalone->ShowWindowAgain();
							m_ScriptViewerStandalone->SetFile( it->path().string() );
						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );
					}

					if( it->path().extension().string() == ".obj" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<File>( it->path().filename().string(), path, FileExtensionType::OBJ );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::Button( it->path().filename().string().c_str() ) )
						{

						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );
					}

					if( it->path().extension().string() == ".fbx" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<File>( it->path().filename().string(), path, FileExtensionType::FBX );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::Button( it->path().filename().string().c_str() ) )
						{

						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );

					}

					if( it->path().extension().string() == ".txt" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<File>( it->path().filename().string(), path, FileExtensionType::TEXT );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::ImageButton( ( ImTextureID )m_TextFile->GetRendererID(), imageSize ) )
						{
						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );

					}

					if( it->path().extension().string() == ".glsl" )
					{
						std::string path =  m_FolderPath + "\\" + it->path().filename().string();

						MakeFileFrom<File>( it->path().filename().string(), path, FileExtensionType::SHADER );

						ImVec2 imageSize( 64, 64 );

						ImGui::ManualWrapBegin( imageSize );
						if( ImGui::Button( it->path().filename().string().c_str() ) )
						{

						}

						DrawAssetText( it->path().filename().string().c_str() );

						ImGui::ManualWrapEnd( imageSize );
					}

				}
			}

			/*
			for( fs::directory_iterator it( m_FolderPath ); it != fs::directory_iterator(); ++it )
			{
				if( it->path().has_extension() )
				{
					if( it->path().extension().string() == ".sc" )
					{
						ImVec2 imageSize( 64, 64 );
						ImGui::ManualWrapBegin( imageSize );
						ImGui::Text( it->path().filename().string().c_str() );
						ImGui::ManualWrapEnd( imageSize );
					}
				}
			}
			*/
			static bool openModal = false;

			if( ImGui::BeginPopupContextWindow( 0, 1, false ) )
			{
				if( ImGui::MenuItem( "Create new File" ) )
				{
					openModal = true;
				}
				ImGui::EndPopup();
			}

			if( openModal )
			{
				ImGui::OpenPopup( "Create new File" );
			}

			if( ImGui::BeginPopupModal( "Create new File", &openModal ) )
			{
				static char filename[ 256 ] = "";
				static char folder[ 256 ] = "";
				static bool fileAlreadyExists = false;
				static char* fullpath = "";

				ImGui::SetWindowSize( ImVec2( 235, 235 ) );

				ImGui::AlignTextToFramePadding();
				if( ImGui::Button( "...##showdropdown" ) )
					ImGui::OpenPopup( "AddFileExtension" );

				if( ImGui::BeginPopup( "AddFileExtension" ) )
				{
					if( ImGui::MenuItem( "Scene File" ) )
					{
						strcat( filename, ".sc" );
					}

					if( ImGui::MenuItem( "cs File" ) )
					{
						strcat( filename, ".cs" );
					}
					ImGui::EndPopup();
				}
				ImGui::Text( "File Name and extension : " );
				ImGui::SameLine();
				ImGui::InputText( "##filename", filename, IM_ARRAYSIZE( filename ) );

				ImGui::Text( "Folder : " );
				ImGui::SameLine();
				if( ImGui::InputText( "##folder", folder, IM_ARRAYSIZE( folder ) ) )
				{
					char* foldercopy = folder;
					char* filenamecopy = filename;
					fullpath = strcat( strcat( foldercopy, "\\" ), filenamecopy );
				}

				if( std::filesystem::exists( fullpath ) )
				{
					ImGui::Text( "File already exists" );
					fileAlreadyExists = true;
				}
				else
				{
					fileAlreadyExists = false;
				}

				if( ImGui::Button( "Cancel" ) )
				{
					ImGui::CloseCurrentPopup();
					openModal = false;
				}
				ImGui::SameLine();
				if( ImGui::Button( "Done" ) && !fileAlreadyExists )
				{
					std::string fullFileName;

					fullFileName = filename;

					std::ofstream file( fullpath );

					std::string currPath = std::filesystem::current_path().string();

					ImGui::CloseCurrentPopup();
					openModal = false;

				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		if( ImGui::Begin( "AssetDebuger" ) )
		{
			ImGui::Text( "File Collection Size: %i", FileCollection::GetCollectionSize() );
		}
		ImGui::End();
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
		ImGui::Begin( "DockSpace Demo", &p_open, window_flags );
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

		if( ImGui::BeginMainMenuBar() )
		{
			if( ImGui::BeginMenu( "File" ) )
			{
				if( ImGui::MenuItem( "New", "Shift+N" ) )
					NewScene();

				if( ImGui::MenuItem( "Open...", "Shift+O" ) )
					OpenScene();

				if( ImGui::MenuItem( "Save As...", "Shift+S" ) )
					SaveSceneAs();
				ImGui::EndMenu();

			}

			const char* text = ProjectSettings::GetCurrentProject()->GetName().c_str();

			ImVec2 textSize = ImGui::CalcTextSize( text );
			ImGui::SameLine( ImGui::GetWindowWidth() - 30 - textSize.x - textSize.y );

			ImGui::Text( text );

			ImGui::EndMainMenuBar();
		}

		m_SceneHierarchyPanel->OnImGuiRender();

		m_ImGuiConsole_Thread = std::thread( &EditorLayer::StartImGuiConsole, this );
		// Editor Panel ------------------------------------------------------------------------------
		m_ImGuiConsole_Thread.join();

		//m_AssetPanel->OnImGuiRender();
		m_TextureViewerPanel->OnImGuiRender();
		m_ScriptViewerStandalone->OnImGuiRender();

		StartAssetLayer();

		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 12, 0 ) );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 12, 4 ) );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemInnerSpacing, ImVec2( 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.8f, 0.8f, 0.8f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );

		ImGui::Begin( "Toolbar", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
		float size = ImGui::GetWindowHeight() - 4.0f;
		// Make buttons in the center of the window
		ImGui::SameLine( ( ImGui::GetWindowContentRegionMax().x / 2.0f ) - ( 1.5f * ( ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x ) ) - ( size / 2.0f ) );

		Ref<Texture2D> button = m_RuntimeScene == !nullptr ? m_StopButtonTexture : m_PlayButtonTexture;

		if( ImGui::ImageButton( ( ( ImTextureID )( button->GetRendererID() ) ), ImVec2( size, size ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), 0 ) )
		{
			if( m_RuntimeScene && m_RuntimeScene->m_RuntimeRunning )
			{
				m_SceneHierarchyPanel->Reset();
				m_RuntimeScene->SetSelectedEntity( {} );
				m_SceneHierarchyPanel->SetSelected( {} );
				m_NoSceneCamera = nullptr;
				m_RuntimeScene->EndRuntime();
				m_RuntimeScene = nullptr;
				m_SelectionContext.clear();
				m_SceneHierarchyPanel->SetContext( m_EditorScene );
				ScriptEngine::SetSceneContext( m_EditorScene );
			}
			else
			{
				m_SceneHierarchyPanel->Reset();
				m_EditorScene->SetSelectedEntity( {} );
				m_SceneHierarchyPanel->SetSelected( {} );
				m_SelectionContext.clear();
				m_RuntimeScene = Ref<Scene>::Create();
				m_SceneHierarchyPanel->SetContext( m_RuntimeScene );
				m_EditorScene->CopyScene( m_RuntimeScene );
				m_RuntimeScene->BeginRuntime();
				ScriptEngine::SetSceneContext( m_RuntimeScene );
			}
		}

		ImGui::End();

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();


		ImGui::Begin( "Project Settings" );
		ImGui::Text( "Startup Scene :" );
		//ImGui::SameLine();

		char buffer[ 256 ];
		memset( buffer, 0, 256 );
		memcpy( buffer, ProjectSettings::GetStartupSceneName().c_str(), ProjectSettings::GetStartupSceneName().length() );
		if( ImGui::InputText( "##directory", buffer, 256 ) )
		{
			ProjectSettings::SetStartupSceneName( std::string( buffer ) );
		}

		if( ImGui::Button( "Open Startup Project" ) )
		{
			OpenScene( ProjectSettings::GetStartupSceneName() );
		}

		ImGui::Text( "Startup project name :" );

		char buff[ 256 ];
		memset( buff, 0, 256 );
		memcpy( buff, ProjectSettings::GetStartupProjectName().c_str(), ProjectSettings::GetStartupProjectName().length() );
		if( ImGui::InputText( "##name", buff, 256 ) )
		{
			ProjectSettings::SetStartupName( std::string( buff ) );
		}

		ImGui::Text( "Startup project folder :" );

		char bufr[ 256 ];
		memset( bufr, 0, 256 );
		memcpy( bufr, ProjectSettings::GetStartupProjectFolder().c_str(), ProjectSettings::GetStartupProjectFolder().length() );
		if( ImGui::InputText( "##yourmum", bufr, 256 ) )
		{
			ProjectSettings::SetStartupFolder( std::string( bufr ) );
		}

		//SAT_CORE_INFO( "{0}, {1}", ProjectSettings::GetStartupNameFolder().first, ProjectSettings::GetStartupNameFolder().second );

		if( ImGui::Button( "Save" ) )
		{
			ProjectSettings::Save();
		}
		ImGui::End();

		ImGui::Begin( "Environment" );
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
		ImGui::End();

		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );

		ImGui::Begin( "Viewport" );
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();

		auto viewportOffset = ImGui::GetWindowPos(); // includes tab bar
		auto viewportSize = ImGui::GetContentRegionAvail();

		m_ViewportBounds[ 0 ] ={ viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[ 1 ] ={ viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportSelected = ImGui::IsWindowFocused();

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
		Entity selectedEntity = m_SceneHierarchyPanel->GetSelectionContext();

		if( selectedEntity && m_GizmoType != -1 )
		{
			float rw = ( float )ImGui::GetWindowWidth();
			float rh = ( float )ImGui::GetWindowHeight();

			ImGuizmo::SetOrthographic( false );
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect( ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh );

			const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
			glm::mat4 viewMatrix = m_EditorCamera.GetViewMatrix();

			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			bool snap = Input::IsKeyPressed( Key::LeftControl );
			float snapValue = 0.5f;
			if( m_GizmoType == ImGuizmo::OPERATION::ROTATE )
				snapValue = 45.0f;

			float snapValues[ 3 ] ={ snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate( glm::value_ptr( viewMatrix ), glm::value_ptr( cameraProjection ), ( ImGuizmo::OPERATION )m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr( transform ), nullptr, snap ? snapValues : nullptr );

			if( ImGuizmo::IsUsing() )
			{
				glm::vec3 translation, scale;
				glm::quat rotation;

				DecomposeTransform( transform, translation, rotation, scale );

				glm::quat delta = rotation - tc.Rotation;
				tc.Position = translation;
				tc.Rotation += delta;
				tc.Scale = scale;
			}
		}
		ImGui::End();

		ImGui::PopStyleVar();

		/*
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
											albedoMap = Texture2D::Create( filename, true/*m_AlbedoInput.SRGB );
											materialInstance->Set( "u_AlbedoTexture", albedoMap );
										}
									}
								}
								ImGui::SameLine();
								ImGui::BeginGroup();
								if( ImGui::Checkbox( "Use##AlbedoMap", &useAlbedoMap ) )
									materialInstance->Set<float>( "u_AlbedoTexToggle", useAlbedoMap ? 1.0f : 0.0f );

								if (ImGui::Checkbox("sRGB##AlbedoMap", &m_AlbedoInput.SRGB))
								{
								if (m_AlbedoInput.TextureMap)
								m_AlbedoInput.TextureMap = Texture2D::Create(m_AlbedoInput.TextureMap->GetPath(), m_AlbedoInput.SRGB);
								}
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
		*/

		ImGui::End();
	}
}