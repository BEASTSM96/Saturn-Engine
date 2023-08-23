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
#include "MaterialAssetViewer.h"
#include "NodeEditor/NodeEditor.h"

#include "ImGuiAuxiliary.h"
#include "Saturn/Vulkan/Renderer.h"

#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

	// TOOD: Create default nodes class
	// TODO: We might want to change this asset type of this node but right now we only use GetAsset to get texture assets.
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

		Pin* p = pNodeEditor->FindPin( OtherPinID );

		return p->Node;
	}

	ed::NodeId FindOtherNodeIDByPin( ed::PinId id, NodeEditor* pNodeEditor ) 
	{
		// The other end of the link.
		ed::PinId OtherPinID;

		Link* pLink = pNodeEditor->FindLinkByPin( id );

		if( pLink->EndPinID == id )
			OtherPinID = pLink->StartPinID;
		else
			OtherPinID = pLink->EndPinID;

		Pin* p = pNodeEditor->FindPin( OtherPinID );

		return p->NodeID;
	}

	MaterialAssetViewer::MaterialAssetViewer( AssetID id )
		: AssetViewer( id )
	{
		m_AssetType = AssetType::Material;

		AddMaterialAsset();
	}

	MaterialAssetViewer::~MaterialAssetViewer()
	{
		delete m_NodeEditor;
	}

	void MaterialAssetViewer::OnImGuiRender()
	{
		DrawInternal();
	}

	void MaterialAssetViewer::AddMaterialAsset()
	{
		Ref<MaterialAsset> materialAsset = AssetManager::Get().GetAssetAs<MaterialAsset>( m_AssetID );

		m_NodeEditor = new NodeEditor( m_AssetID );
		m_NodeEditor->SetWindowName( materialAsset->GetName() );

		m_HostMaterialAsset = materialAsset;

		m_EditingMaterial = Ref<Material>( m_HostMaterialAsset->GetMaterial() );

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

		auto* OutputNode = m_NodeEditor->AddNode( node );

		m_OutputNodeID = (int)(size_t)OutputNode->ID;

		// Read the material data, and create some nodes based of the info.
		
		auto Fn = [&]( const Ref<Texture2D>& rTexture, int Index ) -> void
		{
			const auto& rPath = rTexture->GetPath();
			bool InternalTexture = ( rPath == "Renderer Pink Texture" );

			if( !InternalTexture )
			{
				// Find the texture asset.
				auto relativePath = std::filesystem::relative( rPath, Project::GetActiveProject()->GetRootDir() );
				
				Ref<Asset> TextureAsset = AssetManager::Get().FindAsset( relativePath );
				AssetID TextureAssetID = TextureAsset->GetAssetID();

				Ref<Node> Sampler2DNode;
				Ref<Node> AssetNode;
				Sampler2DNode = SpawnNewSampler2D( m_NodeEditor );
				AssetNode = SpawnNewGetAssetNode( m_NodeEditor );

				AssetNode->ExtraData.Allocate( 1024 );
				AssetNode->ExtraData.Zero_Memory();
				AssetNode->ExtraData.Write( ( uint8_t* ) &TextureAssetID, sizeof( UUID ), 0 );
				AssetNode->ExtraData.Write( ( uint8_t* ) &TextureAsset->GetPath(), sizeof( std::filesystem::path ), sizeof( UUID ) );

				Ref<Node> OutputNode = m_NodeEditor->FindNode( m_OutputNodeID );

				// Link the get asset node with the texture node.
				//s_NodeEditors[ rAsset->GetAssetID() ]->LinkPin( AssetNode->Outputs[ 0 ].ID, Sampler2DNode->Inputs[ 0 ].ID );

				m_NodeEditor->LinkPin( AssetNode->Outputs[ 0 ].ID, Sampler2DNode->Inputs[ 0 ].ID );

				// Link the texture node with the corresponding input in the material output.
				m_NodeEditor->LinkPin( Sampler2DNode->Outputs[ 0 ].ID, OutputNode->Inputs[ Index ].ID );
			}
			else if( Index == 0 )
			{
				Node* node = SpawnNewColorNode( m_NodeEditor );

				auto albedoColor = materialAsset->Get<glm::vec3>( "u_Materials.AlbedoColor" );
				ImVec4 color = ImVec4( albedoColor.x, albedoColor.y, albedoColor.z, 1.0f );

				node->ExtraData.Write( ( uint8_t* ) &color, sizeof( ImVec4 ), 0 );

				auto* pNode = m_NodeEditor->FindNode( m_OutputNodeID );

				m_NodeEditor->LinkPin( node->Outputs[ Index ].ID, pNode->Inputs[ Index ].ID );
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

		m_NodeEditor->SetCreateNewNodeFunction(
			[&]() -> Node*
			{
				Node* node = nullptr;

				if( ImGui::MenuItem( "Texture Sampler2D" ) )
					node = SpawnNewSampler2D( m_NodeEditor );
				
				if( ImGui::MenuItem( "Get Asset" ) )
					node = SpawnNewGetAssetNode( m_NodeEditor );

				if( ImGui::MenuItem( "Color Picker" ) )
					node = SpawnNewColorNode( m_NodeEditor );

				return node;
			} );

		m_NodeEditor->SetDetailsFunction(
			[]( const Node& rNode ) -> void
			{
			}
		);

		m_NodeEditor->SetCompileFunction(
			[ & ]() -> NodeEditorCompilationStatus
			{
				return Compile();
			} );

		// Maybe in the future we would want to do some stuff here.
		m_NodeEditor->SetCloseFunction( []() -> void {} );
		m_NodeEditor->Open( true );
		m_Open = true;
	}

	void MaterialAssetViewer::DrawInternal()
	{
		if( m_NodeEditor->IsOpen() )
			m_NodeEditor->OnImGuiRender(); 
	}

	NodeEditorCompilationStatus MaterialAssetViewer::CheckOutputNodeInput( NodeEditor* pNodeEditor, int PinID, bool ThrowIfNotLinked, const std::string& rErrorMessage, int Index, bool AllowColorPicker, Ref<MaterialAsset>& rMaterialAsset )
	{
		if( pNodeEditor->IsPinLinked( PinID ) )
		{
			ed::NodeId nodeId = FindOtherNodeIDByPin( PinID, pNodeEditor );
			Node* pOtherNode = pNodeEditor->FindNode( nodeId );

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
				ed::NodeId id;
				Node* pAssetNode = nullptr;
				Ref<Asset> TextureAsset = nullptr;
				UUID AssetID;

				if( pNodeEditor->IsPinLinked( pOtherNode->Inputs[ 0 ].ID ) )
					id = FindOtherNodeIDByPin( pOtherNode->Inputs[ 0 ].ID, pNodeEditor );
				else 
				{
					return pNodeEditor->ThrowError( "A texture sampler requires an asset to be linked!" );
				}

				pAssetNode = pNodeEditor->FindNode( id );

				AssetID = pAssetNode->ExtraData.Read<UUID>( 0 );
				TextureAsset = AssetManager::Get().FindAsset( AssetID );

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

	NodeEditorCompilationStatus MaterialAssetViewer::Compile()
	{
		// Find the main output node.
		Node* OutputNode = m_NodeEditor->FindNode( m_OutputNodeID );

		// Clear cache.
		m_HostMaterialAsset->Reset();

		// Check if we have a link in the first input (albedo) as that must be set.
		if( m_NodeEditor->IsPinLinked( OutputNode->Inputs[ 0 ].ID ) )
		{
			auto PinID = OutputNode->Inputs[ 0 ].ID;
			ed::NodeId OtherNodeID;

			OtherNodeID = FindOtherNodeIDByPin( PinID, m_NodeEditor );

			Node* pOtherNode = m_NodeEditor->FindNode( OtherNodeID );

			// Now, we can check what type of node we have.
			if( pOtherNode->Name == "Color Picker" )
			{
				auto& rColor = pOtherNode->ExtraData.Read<ImVec4>( 0 );

				m_HostMaterialAsset->SetAlbeoColor( glm::vec4( rColor.x, rColor.y, rColor.z, rColor.w ) );
			}
			else if( pOtherNode->Name == "Sampler2D" )
			{
				ed::NodeId NodeId;
				Node* pAssetNode = nullptr;

				Ref<Asset> TextureAsset = nullptr;
				UUID AssetID;

				if( m_NodeEditor->IsPinLinked( pOtherNode->Inputs[ 0 ].ID ) )
					NodeId = FindOtherNodeIDByPin( pOtherNode->Inputs[ 0 ].ID, m_NodeEditor );
				else
					return m_NodeEditor->ThrowError( "A texture sampler requires an asset to be linked!" );

				pAssetNode = m_NodeEditor->FindNode( NodeId );

				AssetID = pAssetNode->ExtraData.Read<UUID>( 0 );
				TextureAsset = AssetManager::Get().FindAsset( AssetID );

				// This wont load the texture until we apply it.
				m_HostMaterialAsset->SetAlbeoMap( TextureAsset->Path );
			}
		}
		else
		{
			return m_NodeEditor->ThrowError( "Albedo must be linked to texture sampler or a color node!" );
		}

		// Next, Normals
		if( CheckOutputNodeInput( m_NodeEditor, ( size_t ) OutputNode->Inputs[ 1 ].ID, false, "", 1, false, m_HostMaterialAsset ) == NodeEditorCompilationStatus::Failed )
			return NodeEditorCompilationStatus::Failed;

		// Then, Metallic
		if( CheckOutputNodeInput( m_NodeEditor, ( size_t ) OutputNode->Inputs[ 2 ].ID, false, "", 2, false, m_HostMaterialAsset ) == NodeEditorCompilationStatus::Failed )
			return NodeEditorCompilationStatus::Failed;

		// Then, Roughness
		if( CheckOutputNodeInput( m_NodeEditor, ( size_t ) OutputNode->Inputs[ 3 ].ID, false, "", 3, false, m_HostMaterialAsset ) == NodeEditorCompilationStatus::Failed )
			return NodeEditorCompilationStatus::Failed;

		m_HostMaterialAsset->ApplyChanges();

		// We need to find the asset not the material asset.
		Ref<Asset> asset = AssetManager::Get().FindAsset( m_HostMaterialAsset->ID );

		MaterialAssetSerialiser mas;
		mas.Serialise( asset, m_NodeEditor );

		return NodeEditorCompilationStatus::Success;
	}

}