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

#pragma once

#include <Saturn/ImGui/Viewport.h>
#include <Saturn/ImGui/SceneHierarchyPanel.h>
#include <Saturn/ImGui/ContentBrowserPanel.h>

#include <Saturn/Scene/Scene.h>
#include <Saturn/Core/Layer.h>

namespace Saturn {
	
	class Toolbar;
	class TitleBar;

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		void OnUpdate( Timestep time ) override;

		void OnImGuiRender() override;

		void OnEvent( Event& rEvent ) override;
		
		void SaveFileAs();
		void OpenFile( const std::filesystem::path& rFilepath );

		void SaveFile();
		void OpenFile();

		void SaveProject();

	public:
		
		Viewport* GetViewport() { return m_Viewport; }
		TitleBar* GetTitleBar() { return m_TitleBar; }
		EditorCamera& GetEditorCamera() { return m_EditorCamera; }

	private:
		
		void SelectionChanged( Entity e );
		void ViewportSizeCallback( uint32_t Width, uint32_t Height );
		bool OnKeyPressed( KeyPressedEvent& rEvent );

		// UI Functions.
		void UI_Titlebar_UserSettings();
		bool m_ShowUserSettings = false;

	private:
		Viewport* m_Viewport;
		TitleBar* m_TitleBar;
		
		Ref< Texture2D > m_CheckerboardTexture;

		EditorCamera m_EditorCamera;
		bool m_AllowCameraEvents = false;
		bool m_StartedRightClickInViewport = false;
		bool m_ViewportFocused = false;
		bool m_MouseOverViewport = false;

		bool m_RequestRuntime = false;

		// Translate as default
		int m_GizmoOperation = 7;

		ImVec2 m_ViewportSize;

		Ref< Scene > m_EditorScene;
		Ref< Scene > m_RuntimeScene;
	};
}