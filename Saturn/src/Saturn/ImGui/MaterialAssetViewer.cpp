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

#include "sppch.h"
#include "MaterialAssetViewer.h"
#include "NodeEditor/NodeEditor.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

	static ed::EditorContext* s_Context = nullptr;
	static NodeEditor* s_NodeEditor = nullptr;

	MaterialAssetViewer::MaterialAssetViewer()
	{
		m_AssetType = AssetType::Material;

		ed::Config config = {};
		s_Context = ed::CreateEditor( &config );

		s_NodeEditor = new NodeEditor();
	}

	void MaterialAssetViewer::Draw()
	{
		for( auto& asset : m_MaterialAssets )
			DrawInternal( asset );
	}

	void MaterialAssetViewer::AddMaterialAsset( Ref<MaterialAsset>& rMaterialAsset )
	{
		m_MaterialAssets.push_back( rMaterialAsset );
	}

	void MaterialAssetViewer::DrawInternal( Ref<MaterialAsset>& rMaterialAsset )
	{
		rMaterialAsset->BeginViewingSession();

		s_NodeEditor->Draw();

		/*
		ImGui::PushID( rMaterialAsset->GetAssetID() );

		std::string name = "Material##";
		name += std::to_string( rMaterialAsset->GetAssetID() );

		ImGui::Begin( name.c_str() );

		ImGui::Separator();

		ed::SetCurrentEditor( s_Context );

		ed::Begin( "Material editor", ImVec2(0.0f, 0.0f) );

		int uniqueId = 1;
		// Start drawing nodes.
		ed::BeginNode( uniqueId++ );
		ImGui::Text( "Node A" );
		ed::BeginPin( uniqueId++, ed::PinKind::Input );
		ImGui::Text( "-> In" );
		ed::EndPin();
		ImGui::SameLine();
		ed::BeginPin( uniqueId++, ed::PinKind::Output );
		ImGui::Text( "Out ->" );
		ed::EndPin();
		ed::EndNode();

		ed::BeginNode( uniqueId++ );
		ImGui::Text( "Material Output" );
		ed::BeginPin( uniqueId++, ed::PinKind::Input );
		ImGui::Text( "-> Albedo Texture" );
		ed::EndPin();
		ed::BeginPin( uniqueId++, ed::PinKind::Input );
		ImGui::Text( "-> Base Color" );
		ed::EndPin();
		ed::BeginPin( uniqueId++, ed::PinKind::Input );
		ImGui::Text( "-> Normal Texture" );
		ed::EndPin();
		ed::BeginPin( uniqueId++, ed::PinKind::Input );
		ImGui::Text( "-> Roughness Texture" );
		ed::EndPin();
		ed::BeginPin( uniqueId++, ed::PinKind::Input );
		ImGui::Text( "-> Metalness Texture" );
		ed::EndPin();
		ed::EndNode();

		ed::End();
		ed::SetCurrentEditor( nullptr );

		ImGui::End();

		ImGui::PopID();
		*/



		rMaterialAsset->EndViewingSession();
	}

}