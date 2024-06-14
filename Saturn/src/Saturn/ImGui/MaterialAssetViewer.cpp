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

#include "MaterialNodeEditorEvaluator.h"
#include "MaterialViewerNodes.h"

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

		MaterialNodeEditorEvaluator::MaterialNodeEdInfo info;
		info.HostMaterial = m_HostMaterialAsset;
		info.OutputNodeID = m_OutputNodeID;

		auto rt = Ref<MaterialNodeEditorEvaluator>::Create( info );
		rt->SetTargetNodeEditor( m_NodeEditor );

		m_NodeEditor->SetRuntime( rt );
		m_NodeEditor->SetWindowName( materialAsset->GetName() );

		SetupNodeEditorCallbacks();

		// Maybe in the future we would want to do some stuff here.
		m_NodeEditor->Open( true );
		m_Open = true;
	}

	void MaterialAssetViewer::SetupNodeEditorCallbacks()
	{
		m_NodeEditor->SetCreateNewNodeFunction(
			[&]() -> Ref<Node>
			{
				Ref<Node> node = nullptr;

				ImGui::SeparatorText( "MATERIAL" );

				if( ImGui::MenuItem( "Texture Sampler2D" ) )
					node = MaterialNodeLibrary::SpawnSampler2D( m_NodeEditor );

				if( ImGui::MenuItem( "Get Asset" ) )
					node = MaterialNodeLibrary::SpawnGetAsset( m_NodeEditor );

				if( ImGui::MenuItem( "Color Picker" ) )
					node = MaterialNodeLibrary::SpawnColorPicker( m_NodeEditor );

				if( ImGui::MenuItem( "Color Mixer" ) )
					node = MaterialNodeLibrary::SpawnMixColors( m_NodeEditor );

				ImGui::SeparatorText( "MATH" );

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
	}

	void MaterialAssetViewer::SetupNewNodeEditor()
	{
		// Add material output node.
		Ref<MaterialOutputNode> OutputNode = MaterialNodeLibrary::SpawnOutputNode( m_NodeEditor );
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
			CreateNodesFromTexture( IndexToTextureIndex[ static_cast<uint32_t>( i ) ], static_cast<int>( i ) );
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