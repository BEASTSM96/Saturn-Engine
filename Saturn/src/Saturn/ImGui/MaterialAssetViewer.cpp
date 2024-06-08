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

#include "sppch.h"
#include "MaterialAssetViewer.h"

#include "Saturn/NodeEditor/NodeEditorBase.h"
#include "Saturn/NodeEditor/UI/NodeEditor.h"
#include "Saturn/NodeEditor/Serialisation/NodeEditorCache.h"

#include "ImGuiAuxiliary.h"
#include "Saturn/Vulkan/Renderer.h"

#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL NODE LIBRARY

	Ref<Node> MaterialNodeLibrary::SpawnGetAsset( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Asset ID";
		pin.Type = PinType::AssetHandle;

		NodeSpecification node;
		node.Color = ImColor( 30, 117, 217 );
		node.Name = "Get Asset";

		node.Outputs.push_back( pin );

		return rNodeEditor->AddNode( node );
	}

	Ref<Node> MaterialNodeLibrary::SpawnColorPicker( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "RGBA";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification node;
		node.Color = ImColor( 252, 186, 3 );
		node.Name = "Color Picker";

		node.Outputs.push_back( pin );

		return rNodeEditor->AddNode( node );
	}

	Ref<Node> MaterialNodeLibrary::SpawnSampler2D( Ref<NodeEditorBase> rNodeEditor )
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

		return rNodeEditor->AddNode( node );
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL NODE EDITOR RUNTIME

	static Ref<Node> FindOtherNodeByPin( ed::PinId id, Ref<NodeEditorBase> nodeEditor )
	{
		// The other end of the link.
		ed::PinId OtherPinID;

		Ref<Link> pLink = nodeEditor->FindLinkByPin( id );

		if( pLink->EndPinID == id )
			OtherPinID = pLink->StartPinID;
		else
			OtherPinID = pLink->EndPinID;

		Ref<Pin> p = nodeEditor->FindPin( OtherPinID );

		return p->Node;
	}

	static ed::NodeId FindOtherNodeIDByPin( ed::PinId id, Ref<NodeEditorBase> nodeEditor )
	{
		// The other end of the link.
		ed::PinId OtherPinID;

		Ref<Link> pLink = nodeEditor->FindLinkByPin( id );

		if( pLink->EndPinID == id )
			OtherPinID = pLink->StartPinID;
		else
			OtherPinID = pLink->EndPinID;

		Ref<Pin> p = nodeEditor->FindPin( OtherPinID );

		return p->NodeID;
	}

	MaterialNodeEditorRuntime::MaterialNodeEditorRuntime( const MaterialNodeEdInfo& rInfo )
		: m_Info( rInfo )
	{
	}

	NodeEditorCompilationStatus MaterialNodeEditorRuntime::Execute()
	{
		if( !m_NodeEditor )
			return NodeEditorCompilationStatus::Failed;

		Ref<Node> OutputNode = m_NodeEditor->FindNode( m_Info.OutputNode );

		// TODO: Maybe we should not clear it what if the compile fails?
		m_Info.HostMaterial->Reset();

		// Check if we have a link in the first input (albedo) as that must be set.
		if( m_NodeEditor->IsPinLinked( OutputNode->Inputs[ 0 ]->ID ) )
		{
			auto PinID = OutputNode->Inputs[ 0 ]->ID;
			ed::NodeId OtherNodeID;

			OtherNodeID = FindOtherNodeIDByPin( PinID, m_NodeEditor );

			Ref<Node> pOtherNode = m_NodeEditor->FindNode( OtherNodeID );

			// Now, we can check what type of node we have.
			if( pOtherNode->Name == "Color Picker" )
			{
				auto& rColor = pOtherNode->ExtraData.Read<ImVec4>( 0 );

				m_Info.HostMaterial->SetAlbeoColor( glm::vec4( rColor.x, rColor.y, rColor.z, rColor.w ) );
			}
			else if( pOtherNode->Name == "Sampler2D" )
			{
				ed::NodeId NodeId;
				Ref<Node> pAssetNode = nullptr;

				Ref<Asset> TextureAsset = nullptr;
				UUID AssetID;

				if( m_NodeEditor->IsPinLinked( pOtherNode->Inputs[ 0 ]->ID ) )
					NodeId = FindOtherNodeIDByPin( pOtherNode->Inputs[ 0 ]->ID, m_NodeEditor );
				else
					return NodeEditorCompilationStatus::Failed; // TODO: Throw error

				pAssetNode = m_NodeEditor->FindNode( NodeId );

				AssetID = pAssetNode->ExtraData.Read<UUID>( 0 );
				TextureAsset = AssetManager::Get().FindAsset( AssetID );

				// This wont load the texture until we apply it.
				m_Info.HostMaterial->SetAlbeoMap( TextureAsset->Path );
			}
		}
		else
		{
			return NodeEditorCompilationStatus::Failed; // TODO: Throw error
		}

		// Next, Normals
		if( CheckOutputNodeInput( ( size_t ) OutputNode->Inputs[ 1 ]->ID, false, "", 1, false ) == NodeEditorCompilationStatus::Failed )
			return NodeEditorCompilationStatus::Failed;

		// Then, Metallic
		if( CheckOutputNodeInput( ( size_t ) OutputNode->Inputs[ 2 ]->ID, false, "", 2, false ) == NodeEditorCompilationStatus::Failed )
			return NodeEditorCompilationStatus::Failed;

		// Then, Roughness
		if( CheckOutputNodeInput( ( size_t ) OutputNode->Inputs[ 3 ]->ID, false, "", 3, false ) == NodeEditorCompilationStatus::Failed )
			return NodeEditorCompilationStatus::Failed;

		// Then, Emissive
		auto emissive = OutputNode->Inputs[ 4 ]->ExtraData.Read<float>( 4 );
		if( emissive > 0.0f )
			m_Info.HostMaterial->SetEmissive( emissive );

		m_Info.HostMaterial->ApplyChanges();

		// We need to find the asset not the material asset.
		Ref<Asset> asset = AssetManager::Get().FindAsset( m_Info.HostMaterial->ID );

		// Save the material
		MaterialAssetSerialiser mas;
		//mas.Serialise( asset );

		return NodeEditorCompilationStatus::Success;
	}

	NodeEditorCompilationStatus MaterialNodeEditorRuntime::CheckOutputNodeInput(
		int PinID, bool ThrowIfNotLinked,
		const std::string& rErrorMessage, int Index,
		bool AllowColorPicker )
	{
		if( m_NodeEditor->IsPinLinked( PinID ) )
		{
			ed::NodeId nodeId = FindOtherNodeIDByPin( PinID, m_NodeEditor );
			Ref<Node> pOtherNode = m_NodeEditor->FindNode( nodeId );

			// Color picker is only for albedo.
			if( AllowColorPicker )
			{
				if( pOtherNode->Name == "Color Picker" )
				{
					auto& rColor = pOtherNode->ExtraData.Read<ImVec4>( 0 );

					m_Info.HostMaterial->SetAlbeoColor( glm::vec4( rColor.x, rColor.y, rColor.z, rColor.w ) );
				}
			}

			if( pOtherNode->Name == "Sampler2D" )
			{
				ed::NodeId id;
				Ref<Node> pAssetNode = nullptr;
				Ref<Asset> TextureAsset = nullptr;
				UUID AssetID;

				if( m_NodeEditor->IsPinLinked( pOtherNode->Inputs[ 0 ]->ID ) )
					id = FindOtherNodeIDByPin( pOtherNode->Inputs[ 0 ]->ID, m_NodeEditor );
				else
				{
					return NodeEditorCompilationStatus::Failed;
				}

				pAssetNode = m_NodeEditor->FindNode( id );

				AssetID = pAssetNode->ExtraData.Read<UUID>( 0 );
				TextureAsset = AssetManager::Get().FindAsset( AssetID );

				// This wont load the texture until we apply it.
				if( Index == 0 )
					m_Info.HostMaterial->SetAlbeoMap( TextureAsset->Path );
				else if( Index == 1 )
					m_Info.HostMaterial->SetNormalMap( TextureAsset->Path );
				else if( Index == 2 )
					m_Info.HostMaterial->SetMetallicMap( TextureAsset->Path );
				else if( Index == 3 )
					m_Info.HostMaterial->SetRoughnessMap( TextureAsset->Path );
			}
		}
		else
		{
			if( ThrowIfNotLinked )
			{
				// TODO: Make a better message.
				return NodeEditorCompilationStatus::Failed;
			}
			else
			{
				//m_NodeEditor->ThrowWarning( "Pin was not linked, this may result in errors in the material." );
			}
		}

		return NodeEditorCompilationStatus::Success;
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL ASSET VIEWER

	MaterialAssetViewer::MaterialAssetViewer( AssetID id )
		: AssetViewer( id )
	{
		m_AssetType = AssetType::Material;

		AddMaterialAsset();
	}

	MaterialAssetViewer::~MaterialAssetViewer()
	{
		m_HostMaterialAsset = nullptr;
		m_EditingMaterial = nullptr;

		//NodeEditorCache::WriteNodeEditorCache( m_NodeEditor );

		m_NodeEditor = nullptr;
	}

	void MaterialAssetViewer::OnImGuiRender()
	{
		DrawInternal();
	}

	void MaterialAssetViewer::AddMaterialAsset()
	{
		Ref<MaterialAsset> materialAsset = AssetManager::Get().GetAssetAs<MaterialAsset>( m_AssetID );

		m_HostMaterialAsset = materialAsset;
		m_EditingMaterial = Ref<Material>( m_HostMaterialAsset->GetMaterial() );

		if( false )
		{
			m_NodeEditor = Ref<NodeEditor>::Create();

			// Try to read the cache.
			NodeEditorCache::ReadNodeEditorCache( m_NodeEditor, m_AssetID );

			m_OutputNodeID = ( int ) ( size_t ) m_NodeEditor->FindNode( "Material Output" )->ID;
		}
		else
		{
			m_NodeEditor = Ref<NodeEditor>::Create( m_AssetID );
			SetupNewNodeEditor();
		}

		MaterialNodeEditorRuntime::MaterialNodeEdInfo info;
		info.HostMaterial = m_HostMaterialAsset;
		info.OutputNode = m_OutputNodeID;

		auto rt = Ref<MaterialNodeEditorRuntime>::Create( info );
		rt->SetTargetNodeEditor( m_NodeEditor );

		m_NodeEditor->SetRuntime( rt );
		m_NodeEditor->SetWindowName( materialAsset->GetName() );

		SetupNodeEditorCallbacks();

		// Maybe in the future we would want to do some stuff here.
		m_NodeEditor->SetCloseFunction( []() -> void {} );
		m_NodeEditor->Open( true );
		m_Open = true;
	}

	void MaterialAssetViewer::SetupNodeEditorCallbacks()
	{
		m_NodeEditor->SetCreateNewNodeFunction(
			[&]() -> Ref<Node>
			{
				Ref<Node> node = nullptr;

				if( ImGui::MenuItem( "Texture Sampler2D" ) )
					node = MaterialNodeLibrary::SpawnSampler2D( m_NodeEditor );

				if( ImGui::MenuItem( "Get Asset" ) )
					node = MaterialNodeLibrary::SpawnGetAsset( m_NodeEditor );

				if( ImGui::MenuItem( "Color Picker" ) )
					node = MaterialNodeLibrary::SpawnColorPicker( m_NodeEditor );

				return node;
			} );

		m_NodeEditor->SetDetailsFunction(
			[]( const Ref<Node>& rNode ) -> void
			{
			}
		);
	}

	void MaterialAssetViewer::SetupNewNodeEditor()
	{
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
		pin.Name = "Emission";
		pin.Type = PinType::Float;
		node.Inputs.push_back( pin );

		Ref<Node> OutputNode = m_NodeEditor->AddNode( node );
		OutputNode->CanBeDeleted = false;

		m_OutputNodeID = ( int ) ( size_t ) OutputNode->ID;

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
					Sampler2DNode = MaterialNodeLibrary::SpawnSampler2D( m_NodeEditor );
					AssetNode = MaterialNodeLibrary::SpawnGetAsset( m_NodeEditor );

					AssetNode->ExtraData.Allocate( 1024 );
					AssetNode->ExtraData.Zero_Memory();
					AssetNode->ExtraData.Write( ( uint8_t* ) &TextureAssetID, sizeof( UUID ), 0 );

					Ref<Node> OutputNode = m_NodeEditor->FindNode( m_OutputNodeID );

					// Link the get asset node with the texture node.
					//s_NodeEditors[ rAsset->GetAssetID() ]->LinkPin( AssetNode->Outputs[ 0 ].ID, Sampler2DNode->Inputs[ 0 ].ID );

					m_NodeEditor->LinkPin( AssetNode->Outputs[ 0 ]->ID, Sampler2DNode->Inputs[ 0 ]->ID );

					// Link the texture node with the corresponding input in the material output.
					m_NodeEditor->LinkPin( Sampler2DNode->Outputs[ 0 ]->ID, OutputNode->Inputs[ Index ]->ID );
				}
				else if( Index == 0 )
				{
					Ref<Node> node = MaterialNodeLibrary::SpawnColorPicker( m_NodeEditor );

					auto& albedoColor = m_HostMaterialAsset->Get<glm::vec3>( "u_Materials.AlbedoColor" );
					ImVec4 color = ImVec4( albedoColor.x, albedoColor.y, albedoColor.z, 1.0f );

					node->ExtraData.Write( ( uint8_t* ) &color, sizeof( ImVec4 ), 0 );

					Ref<Node> pNode = m_NodeEditor->FindNode( m_OutputNodeID );

					m_NodeEditor->LinkPin( node->Outputs[ Index ]->ID, pNode->Inputs[ Index ]->ID );
				}

			};

		// Albedo
		{
			Fn( m_HostMaterialAsset->GetAlbeoMap(), 0 );
		}

		// Normal
		{
			Fn( m_HostMaterialAsset->GetNormalMap(), 1 );
		}

		// Metallic
		{
			Fn( m_HostMaterialAsset->GetMetallicMap(), 2 );
		}

		// Roughness
		{
			Fn( m_HostMaterialAsset->GetRoughnessMap(), 3 );
		}
	}

	void MaterialAssetViewer::DrawInternal()
	{
		if( m_NodeEditor->IsOpen() )
		{
			m_NodeEditor->OnImGuiRender();
		}
		else if( m_HostMaterialAsset )
		{
			m_NodeEditor->Open( false );
			m_Open = false;

			DestroyViewer( m_AssetID );
		}
	}
}