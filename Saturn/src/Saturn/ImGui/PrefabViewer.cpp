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
#include "PrefabViewer.h"

#include "Saturn/Asset/AssetRegistry.h"
#include "Saturn/Vulkan/SceneRenderer.h"

#include "Saturn/ImGui/UITools.h"

#include <imgui.h>

namespace Saturn {

	PrefabViewer::PrefabViewer( AssetID id )
		: AssetViewer(), m_Camera( 45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f )
	{
		m_AssetID = id;

		m_SceneHierarchyPanel = Ref<SceneHierarchyPanel>::Create();
		m_SceneHierarchyPanel->SetIsPrefabScene( true );

		AddPrefab();

		m_SceneRenderer = new SceneRenderer();

		m_Camera.SetActive( true );
		m_SceneRenderer->SetCamera( { m_Camera, m_Camera.ViewMatrix() } );
	}

	PrefabViewer::~PrefabViewer()
	{
	}

	void PrefabViewer::Draw()
	{
		ImGui::PushID( static_cast< int >( m_AssetID ) );

		ImGui::Begin( m_Prefab->Name.c_str(), &m_Open );

		m_SceneHierarchyPanel->Draw();

		// Update for scene rendering.
		m_Prefab->GetScene()->OnRenderEditor( m_Camera, Application::Get().Time(), *m_SceneRenderer );
		m_SceneRenderer->RenderScene();

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
		ImGui::Begin( "Viewport", 0, flags );

		Image( m_SceneRenderer->CompositeImage(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );

		ImGui::End();

		ImGui::End();

		ImGui::PopID();

		if( m_Open == false )
		{
			PrefabSerialiser ps;
			ps.Serialise( m_Prefab );
		}
	}

	void PrefabViewer::AddPrefab()
	{
		Ref<Prefab> prefab = AssetRegistry::Get().GetAssetAs<Prefab>( m_AssetID );

		m_SceneHierarchyPanel->SetContext( prefab->GetScene() );

		m_Prefab = prefab;

		m_Open = true;
	}
}