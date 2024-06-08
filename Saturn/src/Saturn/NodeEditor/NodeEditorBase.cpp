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
#include "NodeEditorBase.h"

#include "Runtime/NodeEditorRuntime.h"

#include "Saturn/Vulkan/Texture.h"

#include "Saturn/ImGui/EditorIcons.h"

#include <backends/imgui_impl_vulkan.h>

namespace Saturn {

	Ref<Texture2D> NodeEditorBase::GetBlueprintBackground()
	{
		if( EditorIcons::GetIcon( "BlueprintBackground" ) == nullptr )
		{
			auto texture = Ref<Texture2D>::Create( "content/textures/editor/BlueprintBackground.png", AddressingMode::Repeat, false );

			EditorIcons::AddIcon( texture );

			ImGui_ImplVulkan_AddTexture( texture->GetSampler(), texture->GetImageView(), texture->GetDescriptorInfo().imageLayout );

			return texture;
		}

		return EditorIcons::GetIcon( "BlueprintBackground" );
	}

	static void BuildNode( Ref<Node>& rNode )
	{
		for( auto& input : rNode->Inputs )
		{
			input->Node = rNode;
			input->Kind = PinKind::Input;
		}

		for( auto& output : rNode->Outputs )
		{
			output->Node = rNode;
			output->Kind = PinKind::Output;
		}
	}

	Ref<Node> NodeEditorBase::AddNode( const NodeSpecification& spec, ImVec2 position /*= ImVec2( 0.0f, 0.0f ) */ )
	{
		Ref<Node> node = Ref<Node>::Create( GetNextID(), spec.Name.c_str(), spec.Color );
		m_Nodes.emplace_back( node );

		for( auto& rOutput : spec.Outputs )
		{
			Ref<Pin> pin = Ref<Pin>::Create( GetNextID(), rOutput.Name.c_str(), rOutput.Type, node->ID );
			node->Outputs.push_back( pin );
		}

		for( auto& rInput : spec.Inputs )
		{
			Ref<Pin> pin = Ref<Pin>::Create( GetNextID(), rInput.Name.c_str(), rInput.Type, node->ID );
			node->Inputs.push_back( pin );

			// This should be more than enough data for one pin, holds 16 floats.
			pin->ExtraData.Allocate( 64 );
			pin->ExtraData.Zero_Memory();
		}

		BuildNode( node );

		if( position.x != 0.0f && position.y != 0.0f )
			ed::SetNodePosition( node->ID, position );

		node->ExtraData.Allocate( 1024 );
		node->ExtraData.Zero_Memory();

		return node;
		//return m_Nodes.back();
	}

	//////////////////////////////////////////////////////////////////////////
	// NODE EDITOR

	NodeEditorBase::NodeEditorBase()
		: AssetViewer()
	{
		m_Runtime = Ref<NodeEditorRuntime>::Create();
	}

	NodeEditorBase::NodeEditorBase( AssetID id )
		: AssetViewer( id )
	{
		m_Runtime = Ref<NodeEditorRuntime>::Create();
	}

	NodeEditorBase::~NodeEditorBase()
	{
		m_Runtime = nullptr;
	}
}