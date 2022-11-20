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
#include "Saturn/Serialisation/AssetSerialisers.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

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

		return pNodeEditor->AddNode( node );
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

	Node* SpawnNewColorNode( NodeEditor* pNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "RGBA";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification node;
		node.Color = ImColor( 252, 186, 3 );
		node.Name = "Color Picker";

		node.Outputs.push_back( pin );

		return pNodeEditor->AddNode( node );
	}

	Node* FindOtherNodeByPin( ed::PinId id, NodeEditor* pNodeEditor ) 
	{
		// The other end of the link.
		ed::PinId OtherPinID;

		Link* pLink = pNodeEditor->FindLinkByPin( id );

		if( pLink->EndPinID == id )
			OtherPinID = pLink->StartPinID;
		else
			OtherPinID = pLink->EndPinID;

		return pNodeEditor->FindPin( OtherPinID )->Node;
	}

	MaterialAssetViewer::MaterialAssetViewer()
	{
		m_AssetType = AssetType::Material;
	}

	MaterialAssetViewer::~MaterialAssetViewer()
	{
	}

	void MaterialAssetViewer::Draw()
	{
		for( auto& asset : m_MaterialAssets )
			DrawInternal( asset );
	}

	void MaterialAssetViewer::AddMaterialAsset( Ref<Asset>& rAsset )
	{
		Ref<MaterialAsset> materialAsset = rAsset.As<MaterialAsset>();

		m_MaterialAssets.push_back( materialAsset );

		s_NodeEditors[ rAsset->GetAssetID() ] = new NodeEditor( rAsset->GetAssetID() );

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

		auto* OutputNode = s_NodeEditors[ rAsset->GetAssetID() ]->AddNode( node );

		m_OutputNodeID = (size_t)OutputNode->ID;

		// Read the material data, and create some nodes based of the info.
		
		auto Fn = [&]( const Ref<Texture2D>& rTexture, int Index ) -> void
		{
			const auto& rPath = rTexture->GetPath();

			bool InternalTexture = ( rPath == "Renderer Pink Texture" );
			// TODO: Create color node.

			if( !InternalTexture )
			{
				// Find the texture asset.
				Ref<Asset> TextureAsset = AssetRegistry::Get().FindAsset( rPath );
				auto TextureAssetID = TextureAsset->GetAssetID();

				Node* Sampler2DNode = nullptr;
				Node* AssetNode = nullptr;

				// Create a "Get asset node".

				PinSpecification outPin;
				outPin.Name = "Asset ID";
				outPin.Type = PinType::AssetHandle;

				NodeSpecification nodeSpecification;
				nodeSpecification.Color = ImColor( 30, 117, 217 );
				nodeSpecification.Name = "Get Asset";

				nodeSpecification.Outputs.push_back( outPin );

				AssetNode = s_NodeEditors[ rAsset->GetAssetID() ]->AddNode( nodeSpecification );

				AssetNode->ExtraData.Allocate( 1024 );
				AssetNode->ExtraData.Zero_Memory();
				AssetNode->ExtraData.Write( ( uint8_t* ) &TextureAssetID, sizeof( UUID ), 0 );
				AssetNode->ExtraData.Write( ( uint8_t* ) &TextureAsset->GetPath(), sizeof( std::filesystem::path ), sizeof( UUID ) );

				// Create the corresponding texture sampler node.
				Sampler2DNode = SpawnNewSampler2D( s_NodeEditors[ rAsset->GetAssetID() ] );

				// TODO: FIX THE LINKING!!!

				auto* pOutNode = s_NodeEditors[ rAsset->GetAssetID() ]->FindNode( m_OutputNodeID );

				// Link the get asset node with the texture node.
				s_NodeEditors[ rAsset->GetAssetID() ]->LinkPin( AssetNode->Outputs[ 0 ].ID, Sampler2DNode->Inputs[ 0 ].ID );

				// Link the texture node with the corresponding input in the material output.
				s_NodeEditors[ rAsset->GetAssetID() ]->LinkPin( Sampler2DNode->Outputs[ 0 ].ID, pOutNode->Inputs[ Index ].ID );
			}
			else if( Index == 0 )
			{
				Node* node = SpawnNewColorNode( s_NodeEditors[ rAsset->GetAssetID() ] );

				auto albedoColor = materialAsset->Get<glm::vec3>( "u_Materials.AlbedoColor" );
				ImVec4 color = ImVec4( albedoColor.x, albedoColor.y, albedoColor.z, 1.0f );

				node->ExtraData.Write( ( uint8_t* ) &color, sizeof( ImVec4 ), 0 );

				auto* pNode = s_NodeEditors[ rAsset->GetAssetID() ]->FindNode( m_OutputNodeID );

				s_NodeEditors[ rAsset->GetAssetID() ]->LinkPin( node->Outputs[ Index ].ID, pNode->Inputs[ Index ].ID );
			}

		};

		// Albedo
		{
			Fn( materialAsset->GetAlbeoMap(), 0 );

		}

		// Normal
		{
			Fn( materialAsset->GetNormalMap(), 1 );
		}

		// Metallic
		{
			Fn( materialAsset->GetMetallicMap(), 2 );
		}

		// Roughness
		{
			Fn( materialAsset->GetRoughnessMap(), 3 );
		}

		s_NodeEditors[ rAsset->GetAssetID() ]->SetCreateNewNodeFunction(
			[&, rAsset ]() -> Node*
			{
				Node* node = nullptr;

				if( ImGui::MenuItem( "Texture Sampler2D" ) )
					node = SpawnNewSampler2D( s_NodeEditors[ rAsset->GetAssetID() ] );
				
				if( ImGui::MenuItem( "Get Asset" ) )
					node = SpawnNewGetAssetNode( s_NodeEditors[ rAsset->GetAssetID() ] );

				if( ImGui::MenuItem( "Color Picker" ) )
					node = SpawnNewColorNode( s_NodeEditors[ rAsset->GetAssetID() ] );

				return node;
			} );

		s_NodeEditors[ rAsset->GetAssetID() ]->SetDetailsFunction(
			[]( Node node ) -> void
			{
			}
		);

		auto& rNodeEditor = s_NodeEditors[ rAsset->GetAssetID() ];

		s_NodeEditors[ rAsset->GetAssetID() ]->SetCompileFunction(
			[ &, rAsset, rNodeEditor ]() -> NodeEditorCompilationStatus
			{
				Ref<MaterialAsset> materialAsset = rAsset.As<MaterialAsset>();
				Node* OutputNode = rNodeEditor->FindNode( m_OutputNodeID );

				// Check if we have a link in the first input (albedo) as that must be set.
				if( rNodeEditor->IsPinLinked( OutputNode->Inputs[ 0 ].ID ) )
				{
					auto PinID = OutputNode->Inputs[ 0 ].ID;
					Node* pOtherNode = FindOtherNodeByPin( PinID, rNodeEditor );

					// Now, we can check what type of node we have.
					if( pOtherNode->Name == "Color Picker" ) 
					{
						auto& rColor = pOtherNode->ExtraData.Read<ImVec4>( 0 );

						materialAsset->SetAlbeoColor( glm::vec4( rColor.x, rColor.y, rColor.z, rColor.w ) );
					}
					else if( pOtherNode->Name == "Sampler2D" ) 
					{
						Node* pAssetNode = nullptr;
						Ref<Asset> TextureAsset = nullptr;
						UUID AssetID;
						
						if( rNodeEditor->IsPinLinked( pOtherNode->Inputs[ 0 ].ID ) ) 
							pAssetNode = FindOtherNodeByPin( pOtherNode->Inputs[ 0 ].ID, rNodeEditor );
						else
							return rNodeEditor->ThrowError( "A texture sampler requires an asset to be linked!" );

						AssetID = pAssetNode->ExtraData.Read<UUID>( 0 );
						TextureAsset = AssetRegistry::Get().FindAsset( AssetID );

						// This wont load the texture until we apply it.
						materialAsset->SetAlbeoMap( TextureAsset->Path );
					}
				}
				else
				{
					return rNodeEditor->ThrowError( "Albedo must be linked to texture sampler or a color node!" );
				}

				// Next, Normals
				if( CheckOutputNodeInput( rNodeEditor, ( size_t ) OutputNode->Inputs[ 1 ].ID, false, "", 1, false, materialAsset ) == NodeEditorCompilationStatus::Failed )
					return NodeEditorCompilationStatus::Failed;

				// Then, Metallic
				if( CheckOutputNodeInput( rNodeEditor, ( size_t )OutputNode->Inputs[ 2 ].ID, false, "", 2, false, materialAsset ) == NodeEditorCompilationStatus::Failed )
					return NodeEditorCompilationStatus::Failed;

				// Then, Roughness
				if( CheckOutputNodeInput( rNodeEditor, ( size_t ) OutputNode->Inputs[ 3 ].ID, false, "", 3, false, materialAsset ) == NodeEditorCompilationStatus::Failed )
					return NodeEditorCompilationStatus::Failed;

				materialAsset->ApplyChanges();

				MaterialAssetSerialiser mas;
				mas.Serialise( rAsset, rNodeEditor );

				return NodeEditorCompilationStatus::Success;
			} );

		s_NodeEditors[ rAsset->GetAssetID() ]->Open( true );
	}

	void MaterialAssetViewer::DrawInternal( Ref<MaterialAsset>& rMaterialAsset )
	{
		rMaterialAsset->BeginViewingSession();

		if( s_NodeEditors[ rMaterialAsset->GetAssetID() ]->IsOpen() )
			s_NodeEditors[ rMaterialAsset->GetAssetID() ]->Draw();

		// Disable viewing mode, when we close the editor it will end the session.
		rMaterialAsset->SaveViewingSession();
	}

	NodeEditorCompilationStatus MaterialAssetViewer::CheckOutputNodeInput( NodeEditor* pNodeEditor, int PinID, bool ThrowIfNotLinked, const std::string& rErrorMessage, int Index, bool AllowColorPicker, Ref<MaterialAsset>& rMaterialAsset )
	{
		if( pNodeEditor->IsPinLinked( PinID ) )
		{
			Node* pOtherNode = FindOtherNodeByPin( PinID, pNodeEditor );

			// Color picker is only for albedo.
			if( AllowColorPicker )
			{
				if( pOtherNode->Name == "Color Picker" )
				{
					auto& rColor = pOtherNode->ExtraData.Read<ImVec4>( 0 );

					rMaterialAsset->SetAlbeoColor( glm::vec4( rColor.x, rColor.y, rColor.z, rColor.w ) );
				}
			}

			if( pOtherNode->Name == "Sampler2D" )
			{
				Node* pAssetNode = nullptr;
				Ref<Asset> TextureAsset = nullptr;
				UUID AssetID;

				if( pNodeEditor->IsPinLinked( pOtherNode->Inputs[ 0 ].ID ) )
					pAssetNode = FindOtherNodeByPin( pOtherNode->Inputs[ 0 ].ID, pNodeEditor );
				else 
				{
					return pNodeEditor->ThrowError( "A texture sampler requires an asset to be linked!" );
				}

				AssetID = pAssetNode->ExtraData.Read<UUID>( 0 );
				TextureAsset = AssetRegistry::Get().FindAsset( AssetID );

				// This wont load the texture until we apply it.
				if( Index == 0 )
					rMaterialAsset->SetAlbeoMap( TextureAsset->Path );
				else if( Index == 1 )
					rMaterialAsset->SetNormalMap( TextureAsset->Path );
				else if( Index == 2 )
					rMaterialAsset->SetMetallicMap( TextureAsset->Path );
				else if( Index == 3 )
					rMaterialAsset->SetRoughnessMap( TextureAsset->Path );
			}
		}
		else
		{
			if( ThrowIfNotLinked )
			{
				// TODO: Make a better message.
				return pNodeEditor->ThrowError( rErrorMessage );
			}

			pNodeEditor->ThrowWarning( "Pin was not linked, this may result in errors in the material." );
		}

		return NodeEditorCompilationStatus::Success;
	}
}