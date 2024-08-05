/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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

#include <Saturn/ImGui/SceneHierarchyPanel.h>
#include <Saturn/ImGui/ContentBrowserPanel/ContentBrowserPanel.h>

#include <Saturn/Scene/Scene.h>
#include <Saturn/Core/Layer.h>

#include <Saturn/ImGui/JobProgress.h>

#include <queue>

namespace Saturn {
	
	class TitleBar;
	class GameModule;

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		void OnUpdate( Timestep time ) override;
		void OnImGuiRender() override;
		void OnEvent( RubyEvent& rEvent ) override;
		void OnAttach() override;
		void OnDetach() override;
		
	private:
		void SaveFileAs();
		void OpenFile( AssetID id );

		void SaveFile();

		void SaveProject();

		void SelectionChanged( Ref<Entity> e );
		void ViewportSizeCallback( uint32_t Width, uint32_t Height );
		bool OnKeyPressed( RubyKeyEvent& rEvent );

		// UI Functions.
		void DrawProjectSettingsWindow();
		bool m_ShowUserSettings = false;

		void HotReloadGame();

		void DrawAssetRegistryDebug();
		void DrawLoadedAssetsDebug();
		void DrawEditorSettings();
		void DrawMaterials();
		void DrawMaterialHeader( Ref<MaterialAsset>& rMaterial );
		void DrawAttributions();
		void DrawVFSDebug();
		void DrawTitlebarOptions();
		void DrawAboutWindow();
		void DrawSceneRendererWindow();
		void DrawRendererWindow();

		// Viewport
		void DrawViewport();
		void Viewport_Gizmo();
		void Viewport_RTControls();
		
		// Close editor and open the project browser.
		void CloseEditorAndOpenPB();

		void CheckMissingEnv();
		bool BuildShaderBundle();

		enum MessageBoxButtons_
		{
			MessageBoxButtons_Ok = BIT( 0 ),
			MessageBoxButtons_Cancel = BIT( 1 ),
			MessageBoxButtons_Retry = BIT( 2 ),
			MessageBoxButtons_Exit = BIT( 3 )
		};

		struct MessageBoxInfo
		{
			std::string Title = "Error";
			std::string Text;

			// enum MessageBoxButtons_
			uint32_t Buttons = MessageBoxButtons_Ok;
		};

		void PushMessageBox( MessageBoxInfo& rInfo );
		void PopMessageBox();
		void HandleMessageBoxes();
		void DrawMessageBox( const MessageBoxInfo& rInfo );

	private:
		TitleBar* m_TitleBar = nullptr;
		
		Ref< Texture2D > m_CheckerboardTexture = nullptr;
		Ref< Texture2D > m_StartRuntimeTexture = nullptr;
		Ref< Texture2D > m_EndRuntimeTexture = nullptr;

		Ref< Texture2D > m_TranslationTexture = nullptr;
		Ref< Texture2D > m_RotationTexture = nullptr;
		Ref< Texture2D > m_ScaleTexture = nullptr;
		Ref< Texture2D > m_SyncTexture = nullptr;
		Ref< Texture2D > m_PointLightTexture = nullptr;
		Ref< Texture2D > m_ExclamationTexture = nullptr;

		Ref< PanelManager > m_PanelManager = nullptr;

		GameModule* m_GameModule = nullptr;

		// Used to be called BlockingOperation hence the name.
		Ref<JobProgress> m_BlockingOperation = nullptr;

		EditorCamera m_EditorCamera;
		bool m_AllowCameraEvents = false;
		bool m_StartedRightClickInViewport = false;
		bool m_ViewportFocused = false;
		bool m_MouseOverViewport = false;
		bool m_OpenEditorSettings = false;
		bool m_ShowImGuiDemoWindow = false;
		bool m_ShowVFSDebug = false;
		bool m_HasPremakePath = false;
		bool m_OpenAssetRegistryDebug = false;
		bool m_OpenLoadedAssetDebug = false;
		bool m_OpenAttributions = false;
		bool m_JobModalOpen = false;
		bool m_OpenAboutWindow = false;
		bool m_ShowRendererWindow = true;
		bool m_ShowSceneRendererWindow = true;

		bool m_RequestRuntime = false;

		// Translate as default
		int m_GizmoOperation = 7 /* ImGuizmo::OPERATION::TRANSLATE */;

		ImVec2 m_ViewportSize;

		float m_OperationPercent = 0.0f;
		bool m_ShowOperation = false;

		std::queue<MessageBoxInfo> m_MessageBoxes;

		Ref< Scene > m_EditorScene = nullptr;
		Ref< Scene > m_RuntimeScene = nullptr;
	};
}