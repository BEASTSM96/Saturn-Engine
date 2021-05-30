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

#pragma once

#include <Saturn/Layer.h>
#include <Saturn/Renderer/Mesh.h>
#include <Saturn/Core/Ray.h>
#include <Saturn/ImGui/ImGuiConsole.h>

#include "AssetPanel.h"
#include "AssetGUI/ScriptViewer.h"
#include "AssetGUI/TextureViewer.h"

#include <Saturn/Core/Assets/FileType.h>

namespace Saturn { 

	class EditorLayer : public Layer
	{
	public:
		EditorLayer( void );
		~EditorLayer( void );

		virtual void OnAttach( void ) override;
		void UpdateWindowTitle( std::string name );
		void NewScene();
		void OpenScene();
		void OpenScene( const std::string& filepath );
		virtual void OnDetach( void ) override;
		virtual void OnImGuiRender() override;
		virtual void OnUpdate( Timestep ts ) override;
		virtual void OnEvent( Event& e ) override;
		bool OnMouseButtonPressed( MouseButtonEvent& e );
		bool OnKeyPressedEvent( KeyPressedEvent& e );
		std::pair<float, float> GetMouseViewportSpace( void );
		std::pair<glm::vec3, glm::vec3> CastRay( float mx, float my );
		Ray CastMouseRay( void );
		void SelectEntity( Entity entity );
		float GetSnapValue( void );

		Ref<Scene>& GetEditorScene()
		{
			return m_EditorScene;
		}

		Ref<Scene>& GetRuntimeScene()
		{
			return m_RuntimeScene;
		}

		void DeserialiseDebugLvl();

		void Begin( void );
		void End( void );


		enum class PropertyFlag
		{
			None = 0, ColorProperty = 1, DragProperty = 2, SliderProperty = 4
		};

		// ImGui UI helpers
		bool Property( const std::string& name, bool& value );
		bool Property( const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None );
		bool Property( const std::string& name, glm::vec2& value, PropertyFlag flags );
		bool Property( const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None );
		bool Property( const std::string& name, glm::vec3& value, PropertyFlag flags );
		bool Property( const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None );
		bool Property( const std::string& name, glm::vec4& value, PropertyFlag flags );
		bool Property( const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None );


	private:
		void SaveSceneAs( void );
		void StartImGuiConsole( void );

		template<typename T>
		void MakeFileFrom( std::string filename, std::string filepath, FileExtensionType type )
		{
			static_assert( std::is_base_of<File, T>::value, "T is not type of File" );

			std::string path = filepath;

			if( !FileCollection::DoesFileExistInCollection( filename ) )
			{
				Ref<T>& file = Ref<T>::Create();

				file->Init( filename, path, type );

				FileCollection::AddFileToCollection( (Ref<File>&)file );
			}
		}

		void StartAssetLayer();
		void PrepRuntime();

		int times = 0;

		Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;
		Ref<TextureViewer> m_TextureViewerPanel;
		Ref<AssetPanel> m_AssetPanel;
		Ref<ScriptViewerStandalone> m_ScriptViewerStandalone;

		EditorCamera m_EditorCamera;
		Ref<EditorCamera> m_NoSceneCamera;

		struct SelectedSubmesh
		{
			Saturn::Entity Entity;
			Submesh* Mesh = nullptr;
			float Distance = 0.0f;
		};


		enum class SelectionMode
		{
			None = 0, Entity = 1, SubMesh = 2
		};

		glm::vec2 m_ViewportBounds[ 2 ];
		int m_GizmoType = -1; // -1 = no gizmo
		float m_SnapValue = 0.5f;
		float m_RotationSnapValue = 45.0f;
		bool m_AllowViewportCameraEvents = false;
		bool m_DrawOnTopBoundingBoxes = false;

		SelectionMode m_SelectionMode = SelectionMode::Entity;
		Ref<Texture2D> m_CheckerboardTex;
		Ref<Texture2D> m_FooBarTexure;
		Ref<Texture2D> m_FileSceneTexture;
		std::vector<SelectedSubmesh> m_SelectionContext;
		Ref<Scene> m_RuntimeScene;
		Ref<Scene> m_EditorScene;
		glm::vec2 m_ViewportSize ={ 0.0f, 0.0f };

		std::vector<TransformComponent> m_EditorTransformComponents;
		std::vector<TransformComponent> m_RuntimeTransformComponents;

		std::thread m_Serialiser_Thread;
		std::thread m_ImGuiConsole_Thread;

		void OnSelected( const SelectedSubmesh& selectionContext );

	private:
		//TODO: Add back to AssetPanel.h
		std::vector<std::string> m_Assets;
		std::vector<std::string> m_AssetsFolderContents;
		std::string m_FolderPath = "assets";
		std::string m_Folder;
		std::string m_CurrentFolder;

	private:
		friend class SceneHierarchyPanel;
	};
}