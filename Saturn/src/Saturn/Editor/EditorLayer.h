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

#pragma once

#include "Saturn/ImGui/Viewport.h"
#include "Saturn/ImGui/SceneHierarchyPanel.h"

#include "Saturn/Scene/Scene.h"
#include "Saturn/Core/Layer.h"

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
		
	public:
		
		TitleBar* GetTitleBar() { return m_TitleBar; }
		SceneHierarchyPanel* GetSceneHierarchyPanel() { return m_SceneHierarchyPanel; }
		EditorCamera& GetEditorCamera() { return m_EditorCamera; }

	private:
		
		void SelectionChanged( Entity e );

	private:
		TitleBar* m_TitleBar;
		Viewport* m_Viewport;
		Toolbar* m_Toolbar;
		
		SceneHierarchyPanel* m_SceneHierarchyPanel;

		Ref< Texture2D > m_CheckerboardTexture;

		EditorCamera m_EditorCamera;

		Ref< Scene > m_EditorScene;
		Ref< Scene > m_RuntimeScene;
	};
}