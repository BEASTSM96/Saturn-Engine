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

#include "UITools.h"
#include "Saturn/Vulkan/Renderer.h"

#include "Saturn/Asset/AssetRegistry.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

	static ed::EditorContext* s_Context = nullptr;

	static std::unordered_map<AssetID, NodeEditor*> s_NodeEditors;

	// TOOD: Create default nodes class
	Node* SpawnNewGetAssetNode( NodeEditor* pNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Asset ID";
		pin.Type = PinType::AssetHandle;

		NodeSpecification node;
		node.Color = ImColor( 30, 117, 217 );
		node.Name = "Get Asset";

		node.Outputs.push_back( pin );

		Node* pNode = pNodeEditor->AddNode( node );

		pNode->ExtraData.Allocate( 1024 );
		pNode->ExtraData.Zero_Memory();

		return pNode;
	}

	Node* SpawnNewSampler2D( NodeEditor* pNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Albedo";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification node;
		node.Color = ImColor( 0, 255, 0 );
		node.Name = "Sampler2D";

		pin.Name = "RGBA";
		node.Outputs.push_back( pin );
		pin.Name = "R";
		node.Outputs.push_back( pin );
		pin.Name = "G";
		node.Outputs.push_back( pin );
		pin.Name = "B";
		node.Outputs.push_back( pin );
		pin.Name = "A";
		node.Outputs.push_back( pin );

		pin.Name = "Asset";
		pin.Type = PinType::AssetHandle;
		node.Inputs.push_back( pin );

		return pNodeEditor->AddNode( node );
	}

	MaterialAssetViewer::MaterialAssetViewer()
	{
		m_AssetType = AssetType::Material;

		ed::Config config = {};
		s_Context = ed::CreateEditor( &config );
	}

	MaterialAssetViewer::~MaterialAssetViewer()
	{
	}

	void MaterialAssetViewer::Draw()
	{
		for( auto& asset : m_MaterialAssets )
			DrawInternal( asset );
	}

	void MaterialAssetViewer::AddMaterialAsset( Ref<MaterialAsset>& rMaterialAsset )
	{
		m_MaterialAssets.push_back( rMaterialAsset );
	
		s_NodeEditors[ rMaterialAsset->GetAssetID() ] = new NodeEditor();

		// Add material output node.
		PinSpecification pin;
		pin.Name = "Albedo";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification node;
		node.Color = ImColor( 255, 128, 128 );
		node.Name = "Material Output";
		node.Inputs.push_back( pin );

		pin.Name = "Normal";
		node.Inputs.push_back( pin );
		pin.Name = "Metallic";
		node.Inputs.push_back( pin );
		pin.Name = "Roughness";
		node.Inputs.push_back( pin );

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->SetCreateNewNodeFunction( 
			[&, rMaterialAsset ]() -> Node*
			{
				Node* node = nullptr;

				if( ImGui::MenuItem( "Texture Sampler2D" ) )
					node = SpawnNewSampler2D( s_NodeEditors[ rMaterialAsset->GetAssetID() ] );
				
				if( ImGui::MenuItem( "Get Asset" ) )
					node = SpawnNewGetAssetNode( s_NodeEditors[ rMaterialAsset->GetAssetID() ] );

				return node;
			} );

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->SetDetailsFunction(
			[&, rMaterialAsset]( Node node ) -> void
			{
			}
		);

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->SetCompileFunction( 
			[ rMaterialAsset ]() 
			{
				auto editor = s_NodeEditors[ rMaterialAsset->GetAssetID() ];

				Node* MaterialOutputNode = nullptr;

				for( auto& rNode : editor->GetNodes() )
				{
					if( MaterialOutputNode )
						continue;

					 for ( auto& rOutput : rNode.Outputs )
					 {
						 if( MaterialOutputNode )
							 continue;

						 bool HasAlbedoOut = false;

						 if( rOutput.Name == "Albedo" && rOutput.Type == PinType::Material_Sampler2D )
							 HasAlbedoOut = true;

						 if( HasAlbedoOut )
							 MaterialOutputNode = &rNode;
					 }
				}

				Ref<MaterialAsset> materialAsset = rMaterialAsset;
				//materialAsset->SetAlbeoMap();
			} );

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->Open( true );
	}

	void MaterialAssetViewer::DrawInternal( Ref<MaterialAsset>& rMaterialAsset )
	{
		rMaterialAsset->BeginViewingSession();

		if( s_NodeEditors[ rMaterialAsset->GetAssetID() ]->IsOpen() )
			s_NodeEditors[ rMaterialAsset->GetAssetID() ]->Draw();

		rMaterialAsset->EndViewingSession();
	}

}