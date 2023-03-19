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

#include <imgui.h>

namespace Saturn {

	PrefabViewer::PrefabViewer()
	{
		m_SceneHierarchyPanel = Ref<SceneHierarchyPanel>::Create();
		m_SceneHierarchyPanel->SetIsPrefabScene( true );
	}

	PrefabViewer::~PrefabViewer()
	{

	}

	void PrefabViewer::Draw()
	{
		for( auto& prefab : m_Prefabs )
			DrawInternal( prefab );
	}

	void PrefabViewer::AddPrefab( Ref<Asset>& rAsset )
	{
		Ref<Prefab> prefab = AssetRegistry::Get().GetAssetAs<Prefab>( rAsset->GetAssetID() );

		m_SceneHierarchyPanel->SetContext( prefab->GetScene() );

		m_Prefabs.push_back( prefab );

		m_OpenPrefabs[ prefab->ID ] = true;
	}

	void PrefabViewer::DrawInternal( Ref<Prefab>& rPrefab )
	{
		ImGui::PushID( static_cast<int>( rPrefab->ID ) );

		ImGui::Begin( rPrefab->Name.c_str(), &m_OpenPrefabs.at( rPrefab->ID ) );

		m_SceneHierarchyPanel->Draw();

		ImGui::End();

		ImGui::PopID();

		if( m_OpenPrefabs.at( rPrefab->ID ) == false )
		{
			m_OpenPrefabs.erase( rPrefab->ID );

			m_Prefabs.erase( std::remove( m_Prefabs.begin(), m_Prefabs.end(), rPrefab ), m_Prefabs.end() );

			PrefabSerialiser ps;
			ps.Serialise( rPrefab );
		}
	}

}