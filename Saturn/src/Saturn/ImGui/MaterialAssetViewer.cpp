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
#include "Saturn/NodeEditor/Serialisation/NodeCache.h"

#include "ImGuiAuxiliary.h"
#include "Saturn/Vulkan/Renderer.h"

#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

#include <imgui.h>
#include <imgui_node_editor.h>
#include <stack>

namespace ed = ax::NodeEditor;

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL NODE LIBRARY

	Ref<Node> MaterialNodeLibrary::SpawnGetAsset( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Asset ID";
		pin.Type = PinType::AssetHandle;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 30, 117, 217 );
		nodeSpec.Name = "Get Asset";

		nodeSpec.Outputs.push_back( pin );

		Ref<Node> node = rNodeEditor->AddNode( nodeSpec );
		node->ExecutionType = NodeExecutionType::AssetID;

		return node;
	}

	Ref<Node> MaterialNodeLibrary::SpawnColorPicker( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "RGBA";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 252, 186, 3 );
		nodeSpec.Name = "Color Picker";

		nodeSpec.Outputs.push_back( pin );

		Ref<Node> node = rNodeEditor->AddNode( nodeSpec );
		node->ExecutionType = NodeExecutionType::ColorPicker;

		return node;
	}

	Ref<Node> MaterialNodeLibrary::SpawnSampler2D( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Albedo";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 0, 255, 0 );
		nodeSpec.Name = "Sampler2D";

		pin.Name = "RGBA";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "R";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "G";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "B";
		nodeSpec.Outputs.push_back( pin );
		pin.Name = "A";
		nodeSpec.Outputs.push_back( pin );

		pin.Name = "Asset";
		pin.Type = PinType::AssetHandle;
		nodeSpec.Inputs.push_back( pin );

		Ref<Node> node = rNodeEditor->AddNode( nodeSpec );
		node->ExecutionType = NodeExecutionType::Sampler2D;

		return node;
	}

	Ref<Node> MaterialNodeLibrary::SpawnOutputNode( Ref<NodeEditorBase> rNodeEditor )
	{
		PinSpecification pin;
		pin.Name = "Albedo";
		pin.Type = PinType::Material_Sampler2D;

		NodeSpecification nodeSpec;
		nodeSpec.Color = ImColor( 255, 128, 128 );
		nodeSpec.Name = "Material Output";
		nodeSpec.Inputs.push_back( pin );

		pin.Name = "Normal";
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Metallic";
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Roughness";
		nodeSpec.Inputs.push_back( pin );
		pin.Name = "Emission";
		pin.Type = PinType::Float;
		nodeSpec.Inputs.push_back( pin );

		Ref<Node> node = rNodeEditor->AddNode( nodeSpec );
		node->ExecutionType = NodeExecutionType::MaterialOutput;

		return node;
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL OUTPUT NODE RUNTIME

	void MaterialOutputNodeRuntime::EvaluateNode()
	{
		while( !TextureStack.empty() )
		{
			const auto& tv = TextureStack.top();
			TextureStack.pop();

			if( tv.Slot == 0 )
			{
				HandleAlbedo( tv );
			}
			else if( tv.Slot == 1 )
			{
				MaterialAsset->SetNormalMap( tv.TextureAssetID );
			}
			else if( tv.Slot == 2 )
			{
				MaterialAsset->SetMetallicMap( tv.TextureAssetID );
			}
			else if( tv.Slot == 3 )
			{
				MaterialAsset->SetRoughnessMap( tv.TextureAssetID );
			}
		}
	}

	void MaterialOutputNodeRuntime::HandleAlbedo( const TextureValue& rTextureValue )
	{
		if( rTextureValue.Color.r != 0.0f && rTextureValue.Color.g != 0.0f && rTextureValue.Color.b != 0.0f )
		{
			MaterialAsset->SetAlbeoColor( rTextureValue.Color );
		}
		else if( rTextureValue.TextureAssetID != 0 )
		{
			Ref<Asset> TextureAsset = nullptr;
			TextureAsset = AssetManager::Get().FindAsset( rTextureValue.TextureAssetID );

			MaterialAsset->SetAlbeoColor( glm::vec3( 1.0f ) );
			MaterialAsset->SetAlbeoMap( TextureAsset->Path );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL NODE EDITOR RUNTIME

	static Ref<Node> FindOtherNodeByPin( UUID id, Ref<NodeEditorBase> nodeEditor )
	{
		// The other end of the link.
		UUID OtherPinID;

		Ref<Link> pLink = nodeEditor->FindLinkByPin( id );

		if( pLink->EndPinID == id )
			OtherPinID = pLink->StartPinID;
		else
			OtherPinID = pLink->EndPinID;

		Ref<Pin> p = nodeEditor->FindPin( OtherPinID );

		return p->Node;
	}

	static UUID FindOtherNodeIDByPin( UUID id, Ref<NodeEditorBase> nodeEditor )
	{
		// The other end of the link.
		UUID OtherPinID;

		Ref<Link> pLink = nodeEditor->FindLinkByPin( id );

		if( pLink->EndPinID == id )
			OtherPinID = pLink->StartPinID;
		else
			OtherPinID = pLink->EndPinID;

		Ref<Pin> p = nodeEditor->FindPin( OtherPinID );

		return p->Node->ID;
	}

	MaterialNodeEditorRuntime::MaterialNodeEditorRuntime( const MaterialNodeEdInfo& rInfo )
		: m_Info( rInfo )
	{
	}

	NodeEditorCompilationStatus MaterialNodeEditorRuntime::Execute()
	{
		if( !m_NodeEditor )
			return NodeEditorCompilationStatus::Failed;

		m_Info.HostMaterial->Reset();

		Ref<Node> OutputNode = m_NodeEditor->FindNode( m_Info.OutputNodeID );
		UUID AlbedoPinID = OutputNode->Inputs[ 0 ]->ID;
		UUID NormalPinID = OutputNode->Inputs[ 1 ]->ID;
		UUID MetallicPinID = OutputNode->Inputs[ 2 ]->ID;
		UUID RoughnessPinID = OutputNode->Inputs[ 3 ]->ID;

		std::stack<UUID> order;
		m_NodeEditor->TraverseFromStart( OutputNode, 
			[&]( const UUID id )
			{
				order.push( id );
			} );
		
		std::stack<MaterialOutputNodeRuntime::TextureValue> values;

		// TODO List:
		// Get correct texture slot
		// Support texture loading
		// Add emssive values

		while( !order.empty() )
		{
			const UUID currentNodeID = order.top();
			order.pop();
			
			Ref<Node> currentNode = m_NodeEditor->FindNode( currentNodeID );
			Ref<NodeRuntime> nodeRT = nullptr;

			std::stack<AssetID> assetIDs;
			switch( currentNode->ExecutionType )
			{
				case NodeExecutionType::ColorPicker: 
				{
					size_t index = IsOutputsLinkedToOutNode( currentNode );
				
					nodeRT = Ref<ColorPickerNodeRuntime>::Create( currentNodeID );
					auto colorRT = nodeRT.As<ColorPickerNodeRuntime>();
					colorRT->Value = currentNode->ExtraData.Read<glm::vec4>();

					if( index != UINT64_MAX )
					{
						MaterialOutputNodeRuntime::TextureValue tv{};
						tv.Slot = index;
						tv.Color = colorRT->Value;

						values.push( tv );
					}
				} break;

				case NodeExecutionType::Sampler2D:
				{
					size_t index = IsOutputsLinkedToOutNode( currentNode );

					if( index != UINT64_MAX )
					{
						// Get asset node
						Ref<Node> node = FindOtherNodeByPin( currentNode->Inputs[ 0 ]->ID, m_NodeEditor );

						UUID AssetID = 2391113416952765199;
						
						if( false )
							AssetID = node->ExtraData.Read<UUID>( 0 );

						MaterialOutputNodeRuntime::TextureValue tv{};
						tv.TextureAssetID = AssetID;
						tv.Slot = index;

						values.push( tv );
					}
				} break;

				case NodeExecutionType::MaterialOutput: 
				{
					nodeRT = Ref<MaterialOutputNodeRuntime>::Create( currentNodeID );
					auto outputRT = nodeRT.As<MaterialOutputNodeRuntime>();
					outputRT->MaterialAsset = m_Info.HostMaterial;

					outputRT->TextureStack = values;
				} break;
			}

			if( nodeRT )
				nodeRT->EvaluateNode();
		}

		m_Info.HostMaterial->ApplyChanges();

		m_NodeEditor->ShowFlow();

		return NodeEditorCompilationStatus::Success;

		// We need to find the asset not the material asset.
		Ref<Asset> asset = AssetManager::Get().FindAsset( m_Info.HostMaterial->ID );

		// Save the material
		MaterialAssetSerialiser mas;
		//mas.Serialise( asset );
	}

	NodeEditorCompilationStatus MaterialNodeEditorRuntime::CheckOutputNodeInput(
		int PinID, bool ThrowIfNotLinked,
		const std::string& rErrorMessage, int Index,
		bool AllowColorPicker )
	{
		if( m_NodeEditor->IsLinked( PinID ) )
		{
			UUID nodeId = FindOtherNodeIDByPin( PinID, m_NodeEditor );
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
				UUID id;
				Ref<Node> pAssetNode = nullptr;
				Ref<Asset> TextureAsset = nullptr;
				UUID AssetID;

				if( m_NodeEditor->IsLinked( pOtherNode->Inputs[ 0 ]->ID ) )
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

	size_t MaterialNodeEditorRuntime::IsOutputsLinkedToOutNode( const Ref<Node>& rNode )
	{
		Ref<Node> outputNode = m_NodeEditor->FindNode( m_Info.OutputNodeID );

		size_t i = 0;
		for( const auto& rOutput : rNode->Outputs )
		{
			Ref<Link> link = m_NodeEditor->FindLinkByPin( rOutput->ID );
			if( !link )
				continue;

			if( link->StartPinID == outputNode->Inputs[ i ]->ID || link->EndPinID == outputNode->Inputs[ i ]->ID )
			{
				return i;
			}

			i++;
		}

		return UINT64_MAX;
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

		m_NodeEditor->SaveSettings();
		NodeCacheEditor::WriteNodeEditorCache( m_NodeEditor );

		m_NodeEditor->SetRuntime( nullptr );
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
			NodeCacheEditor::ReadNodeEditorCache( m_NodeEditor, m_AssetID );

			m_OutputNodeID = m_NodeEditor->FindNode( "Material Output" )->ID;
		}
		else
		{
			m_NodeEditor = Ref<NodeEditor>::Create( m_AssetID );
			SetupNewNodeEditor();
		}

		MaterialNodeEditorRuntime::MaterialNodeEdInfo info;
		info.HostMaterial = m_HostMaterialAsset;
		info.OutputNodeID = m_OutputNodeID;

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

				if( ImGui::MenuItem( "Add Floats" ) )
					node = DefaultNodeLibrary::SpawnAddFloats( m_NodeEditor );

				if( ImGui::MenuItem( "Subtract Floats" ) )
					node = DefaultNodeLibrary::SpawnSubFloats( m_NodeEditor );

				if( ImGui::MenuItem( "Multiply Floats" ) )
					node = DefaultNodeLibrary::SpawnMulFloats( m_NodeEditor );

				if( ImGui::MenuItem( "Divide Floats" ) )
					node = DefaultNodeLibrary::SpawnDivFloats( m_NodeEditor );

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
		Ref<Node> OutputNode = MaterialNodeLibrary::SpawnOutputNode( m_NodeEditor );
		OutputNode->CanBeDeleted = false;

		m_OutputNodeID = OutputNode->ID;

		// Read the material data, and create some nodes based of the info.
		SetupNodesFromMaterial();
	}

	void MaterialAssetViewer::SetupNodesFromMaterial()
	{
		std::map<uint32_t, Ref<Texture2D>> IndexToTextureIndex =
		{
			{ 0, m_HostMaterialAsset->GetAlbeoMap() },
			{ 1, m_HostMaterialAsset->GetNormalMap() },
			{ 2, m_HostMaterialAsset->GetMetallicMap() },
			{ 3, m_HostMaterialAsset->GetRoughnessMap() }
		};

		for( size_t i = 0; i < IndexToTextureIndex.size(); i++ )
		{
			CreateNodesFromTexture( IndexToTextureIndex[ i ], i );
		}
	}

	void MaterialAssetViewer::CreateNodesFromTexture( const Ref<Texture2D>& rTexture, int slot )
	{
		const auto& rPath = rTexture->GetPath();
		bool HasTexture = rTexture != Renderer::Get().GetPinkTexture();

		if( HasTexture )
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

			m_NodeEditor->CreateLink( AssetNode->Outputs[ 0 ], Sampler2DNode->Inputs[ 0 ] );
			m_NodeEditor->CreateLink( Sampler2DNode->Outputs[ 0 ], OutputNode->Inputs[ slot ] );
		}
		else if( slot == 0 )
		{
			Ref<Node> colorPickerNode = MaterialNodeLibrary::SpawnColorPicker( m_NodeEditor );

			auto& albedoColor = m_HostMaterialAsset->Get<glm::vec3>( "u_Materials.AlbedoColor" );
			ImVec4 color = ImVec4( albedoColor.x, albedoColor.y, albedoColor.z, 1.0f );

			colorPickerNode->ExtraData.Write( ( uint8_t* ) &color, sizeof( ImVec4 ), 0 );

			Ref<Node> outputNode = m_NodeEditor->FindNode( m_OutputNodeID );
			m_NodeEditor->CreateLink( colorPickerNode->Outputs[ slot ], outputNode->Inputs[ slot ] );
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