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

#include "Saturn/NodeEditor/Serialisation/NodeCache.h"

#include <backends/imgui_impl_vulkan.h>

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	// DEFAULT NODE LIBRARY

	Ref<Node> DefaultNodeLibrary::SpawnAddFloats( Ref<NodeEditorBase> nodeEditorBase )
	{
		PinSpecification pinSpec;
		pinSpec.Type = PinType::Float;

		NodeSpecification nodeSpec{};
		nodeSpec.Inputs.push_back( pinSpec );
		nodeSpec.Inputs.push_back( pinSpec );

		nodeSpec.Outputs.push_back( pinSpec );

		nodeSpec.Name = "Add Floats";
		
		Ref<Node> node = Ref<Node>::Create( nodeSpec );
		node->ExecutionType = NodeExecutionType::Add;
		nodeEditorBase->AddNode( node );

		return node;
	}

	Ref<Node> DefaultNodeLibrary::SpawnSubFloats( Ref<NodeEditorBase> nodeEditorBase )
	{
		PinSpecification pinSpec;
		pinSpec.Type = PinType::Float;

		NodeSpecification nodeSpec{};
		nodeSpec.Inputs.push_back( pinSpec );
		nodeSpec.Inputs.push_back( pinSpec );

		nodeSpec.Outputs.push_back( pinSpec );

		nodeSpec.Name = "Subtract Floats";

		Ref<Node> node = Ref<Node>::Create( nodeSpec );
		node->ExecutionType = NodeExecutionType::Subtract;
		nodeEditorBase->AddNode( node );

		return node;
	}

	Ref<Node> DefaultNodeLibrary::SpawnMulFloats( Ref<NodeEditorBase> nodeEditorBase )
	{
		PinSpecification pinSpec;
		pinSpec.Type = PinType::Float;

		NodeSpecification nodeSpec{};
		nodeSpec.Inputs.push_back( pinSpec );
		nodeSpec.Inputs.push_back( pinSpec );

		nodeSpec.Outputs.push_back( pinSpec );

		nodeSpec.Name = "Multiply Floats";

		Ref<Node> node = Ref<Node>::Create( nodeSpec );
		node->ExecutionType = NodeExecutionType::Multiply;
		nodeEditorBase->AddNode( node );

		return node;
	}

	Ref<Node> DefaultNodeLibrary::SpawnDivFloats( Ref<NodeEditorBase> nodeEditorBase )
	{
		PinSpecification pinSpec;
		pinSpec.Type = PinType::Float;

		NodeSpecification nodeSpec{};
		nodeSpec.Inputs.push_back( pinSpec );
		nodeSpec.Inputs.push_back( pinSpec );

		nodeSpec.Outputs.push_back( pinSpec );

		nodeSpec.Name = "Divide Floats";

		Ref<Node> node = Ref<Node>::Create( nodeSpec );
		node->ExecutionType = NodeExecutionType::Divide;
		nodeEditorBase->AddNode( node );

		return node;
	}

	//////////////////////////////////////////////////////////////////////////
	// NODE EDITOR BASE

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

	NodeEditorBase::NodeEditorBase()
	{
		m_Runtime = Ref<NodeEditorRuntime>::Create();
	}

	NodeEditorBase::NodeEditorBase( AssetID id )
		: m_AssetID( id )
	{
		m_Runtime = Ref<NodeEditorRuntime>::Create();
	}

	NodeEditorBase::~NodeEditorBase()
	{
		m_Runtime = nullptr;
		ed::DestroyEditor( m_Editor );
		ed::SetCurrentEditor( nullptr );

		for( auto& [id, rNode] : m_Nodes )
		{
			rNode->Destroy();
		}

		m_Links.clear();
		m_Nodes.clear();
	}

	void NodeEditorBase::SaveSettings()
	{
		NodeCacheSettings::Get().WriteEditorSettings( this );
	}

	bool NodeEditorBase::IsLinked( UUID pinID )
	{
		if( !pinID )
			return false;

		const auto Itr = std::find_if( m_Links.begin(), m_Links.end(),
			[pinID]( const auto& rLink )
			{
				return rLink->StartPinID == pinID || rLink->EndPinID == pinID;
			} );

		if( Itr != m_Links.end() )
			return true;

		return false;
	}

	Ref<Pin> NodeEditorBase::FindPin( UUID id )
	{
		if( !id )
			return nullptr;

		for( const auto& [ nodeId, rNode ] : m_Nodes )
		{
			for( const auto& pin : rNode->Inputs )
			{
				if( pin->ID == id )
				{
					return pin;
				}
			}

			for( const auto& pin : rNode->Outputs )
			{
				if( pin->ID == id )
				{
					return pin;
				}
			}
		}

		return nullptr;
	}

	Ref<Link> NodeEditorBase::FindLink( UUID id )
	{
		const auto Itr = std::find_if( m_Links.begin(), m_Links.end(),
			[id]( const auto& rLink )
			{
				return rLink->ID == id;
			} );

		if( Itr != m_Links.end() )
			return *Itr;

		return nullptr;
	}

	Ref<Node> NodeEditorBase::FindNode( UUID id )
	{
		const auto Itr = m_Nodes.find( id );

		if( Itr != m_Nodes.end() )
			return Itr->second;

		return nullptr;
	}

	Ref<Node> NodeEditorBase::FindNode( const std::string& rName )
	{
		for( auto& [ id, node ]: m_Nodes )
		{
			if( node->Name == rName )
				return node;
		}

		return nullptr;
	}

	Ref<Link> NodeEditorBase::FindLinkByPin( UUID id )
	{
		if( !id )
			return nullptr;

		if( !IsLinked( id ) )
			return nullptr;

		const auto Itr = std::find_if( m_Links.begin(), m_Links.end(),
			[id]( const auto& rLink )
			{
				return rLink->StartPinID == id || rLink->EndPinID == id;
			} );

		if( Itr != m_Links.end() )
			return *Itr;

		return nullptr;
	}

	Ref<Node> NodeEditorBase::FindNodeByPin( UUID id )
	{
		if( const auto& rPin = FindPin( id ); rPin != nullptr ) 
		{
			return rPin->Node;
		}
		
		return nullptr;
	}

	void NodeEditorBase::SetRuntime( Ref<NodeEditorRuntime> runtime )
	{
		m_Runtime = runtime;
	}

	std::vector<UUID> NodeEditorBase::FindNeighbors( Ref<Node> node )
	{
		std::vector<UUID> ids;

		for( const auto& rInput : node->Inputs )
		{
			if( !IsLinked( rInput->ID ) )
				continue;

			// If the pin is linked find the other end of it and add it to our list.
			Ref<Link> rLink = FindLinkByPin( rInput->ID );

			if( !rLink )
				continue;

			const bool isStart = rLink->StartPinID == rInput->ID;
			Ref<Node> otherNode = FindNodeByPin( isStart ? rLink->EndPinID : rLink->StartPinID );

			ids.push_back( otherNode->ID );
		}

		return ids;
	}

	void NodeEditorBase::CreateLink( const Ref<Pin>& rStart, const Ref<Pin>& rEnd )
	{
		m_Links.push_back( Ref<Link>::Create( UUID(), rStart->ID, rEnd->ID ) );
	}

	void NodeEditorBase::ShowFlow()
	{
		for( const auto& rLink : m_Links )
			ed::Flow( ed::LinkId( rLink->ID ) );
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

	void NodeEditorBase::AddNode( Ref<Node> node )
	{
		m_Nodes[ node->ID ] = node;

		BuildNode( node );

		if( node->Position.x != 0.0f && node->Position.y != 0.0f )
			ed::SetNodePosition( ed::NodeId( node->ID ), node->Position );
	}
}