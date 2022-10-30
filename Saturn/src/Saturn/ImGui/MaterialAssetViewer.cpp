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

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->AddNode( node );

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

		node.Inputs.clear();
		node.Color = ImColor( 0, 255, 0 );
		node.Name = "Sampler2D";

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->AddNode( node );

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->SetCreateNewNodeFunction( 
			[&, rMaterialAsset ]() -> Node*
			{
				Node* node = nullptr;

				if( ImGui::MenuItem( "Texture Sampler2D" ) )
					node = SpawnNewSampler2D( s_NodeEditors[ rMaterialAsset->GetAssetID() ] );

				return node;
			} );

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->SetDetailsFunction(
			[&]( Node node ) -> void
			{
				// TEMP
				// For now lets just check if we have a Material output, if we do then assume it's a texture sampler

				bool HasTextureOutput = false;

				for( auto& outs : node.Outputs )
				{
					if( outs.Type == PinType::Material_Sampler2D )
					{
						HasTextureOutput = true;
						break;
					}
				}

				// Texture sampler node
				if( HasTextureOutput )
				{
					const char* items[] = { "Albedo", "Normal", "Metalness" };
					static const char* current_item = NULL;

					if( ImGui::BeginCombo( "##combo", current_item ) )
					{
						for( size_t i = 0; i < IM_ARRAYSIZE( items ); i++ )
						{
							bool Selected = ( current_item == items[ i ] );

							if( ImGui::Selectable( items[ i ], Selected ) )
								current_item = items[ i ];

							if( Selected )
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					auto opened = TreeNode( "Texture", true );

					if( opened )
					{
						ImGui::Text( "Select texture" );
						ImGui::SameLine();

						static Ref<Texture2D> PreviewTexture = Renderer::Get().GetPinkTexture();

						static bool ShowTextureAssets = false;

						if( ImageButton( PreviewTexture, ImVec2( 100, 100 ) ) )
						{
							ShowTextureAssets = !ShowTextureAssets;
						}

						if( ShowTextureAssets )
						{
							std::vector<Ref<Asset>> result;

							for( const auto& [id, asset] : AssetRegistry::Get().GetAssetMap() )
							{
								if( asset->GetAssetType() == AssetType::Texture )
									result.push_back( asset );
							}

							static AssetID pCurrentItemAssetName = NULL;

							if( ImGui::BeginCombo( "##combo", std::to_string( pCurrentItemAssetName ).c_str() ) )
							{
								for( size_t i = 0; i < result.size(); i++ )
								{
									bool Selected = ( pCurrentItemAssetName == result[ i ] );

									if( ImGui::Selectable( std::to_string( result[ i ]->GetAssetID() ).c_str(), Selected ) )
										pCurrentItemAssetName = result[ i ]->GetAssetID();

									if( Selected )
									{
										ImGui::SetItemDefaultFocus();
									}
								}

								ImGui::EndCombo();
							}
						}
					}

					EndTreeNode();
				}
			}
		);
	}

	void MaterialAssetViewer::DrawInternal( Ref<MaterialAsset>& rMaterialAsset )
	{
		rMaterialAsset->BeginViewingSession();

		s_NodeEditors[ rMaterialAsset->GetAssetID() ]->Draw();

		rMaterialAsset->EndViewingSession();
	}

}