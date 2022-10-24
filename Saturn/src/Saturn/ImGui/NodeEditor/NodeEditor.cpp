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
#include "NodeEditor.h"
#include "Saturn/Vulkan/Image2D.h"
#include "Saturn/Vulkan/Texture.h"

// imgui_node_editor
#include "builders.h"

#include <backends/imgui_impl_vulkan.h>

namespace util = ax::NodeEditor::Utilities;

namespace Saturn {

	static int s_ID = -1;

	static Ref<Texture2D> s_BlueprintBackground;
	static ImTextureID s_BlueprintBackgroundID;

	int GetNextID()
	{
		return s_ID++;
	}

	void BuildNode( Node* node )
	{
		for( auto& input : node->Inputs )
		{
			input.Node = node;
			input.Kind = PinKind::Input;
		}

		for( auto& output : node->Outputs )
		{
			output.Node = node;
			output.Kind = PinKind::Output;
		}
	}

	NodeEditor::NodeEditor()
	{
		m_Editor = ed::CreateEditor( {} );
		ed::SetCurrentEditor( m_Editor );
		ed::NavigateToContent();

		s_BlueprintBackground = Ref<Texture2D>::Create( "assets/textures/BlueprintBackground.png", AddressingMode::Repeat );

		s_BlueprintBackgroundID = ImGui_ImplVulkan_AddTexture( s_BlueprintBackground->GetSampler(), s_BlueprintBackground->GetImageView(), s_BlueprintBackground->GetDescriptorInfo().imageLayout );

		NodeSpecification spec;
		spec.Color = ImColor( 255, 128, 128 );
		spec.Name = "Test BP Node";

		PinSpecification pinSpec = {};
		pinSpec.Name = "Hello";
		pinSpec.Type = PinType::String;

		spec.Inputs.push_back( pinSpec );
		pinSpec.Name = "World";
		spec.Outputs.push_back( pinSpec );

		AddNode( spec );
	}

	NodeEditor::~NodeEditor()
	{

	}

	Node* NodeEditor::AddNode( NodeSpecification& spec, ImVec2 position )
	{
		auto& node = m_Nodes.emplace_back( GetNextID(), spec.Name.c_str(), spec.Color );
		
		for ( auto& rOutput : spec.Outputs )
			m_Nodes.back().Outputs.push_back( { GetNextID(), rOutput.Name.c_str(), rOutput.Type } );

		for( auto& rInput : spec.Inputs )
			m_Nodes.back().Inputs.push_back( { GetNextID(), rInput.Name.c_str(), rInput.Type } );

		BuildNode( &m_Nodes.back() );

		if( position.x != 0.0f && position.y != 0.0f )
			ed::SetNodePosition( node.ID, position );

		return &m_Nodes.back();
	}

	bool NodeEditor::IsPinLinked( ed::PinId id )
	{
		if( !id )
			return false;

		for( auto& link : m_Links )
			if( link.StartPinID == id || link.EndPinID == id )
				return true;

		return false;
	}

	bool NodeEditor::CanCreateLink( Pin* a, Pin* b )
	{
		if( !a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node )
			return false;

		return true;
	}

	void DrawPinIcon( const Pin& pin, bool connected, int alpha )
	{
		// TODO
	}

	void NodeEditor::Draw()
	{
		// Safety
		ed::SetCurrentEditor( m_Editor );

		ImGui::Begin( "##NODE_EDITOR" );

		ed::Begin( "Node Editor" );

		util::BlueprintNodeBuilder builder( s_BlueprintBackgroundID, s_BlueprintBackground->Width(), s_BlueprintBackground->Height() );

		for( auto& node : m_Nodes )
		{
			if( node.Type != NodeType::Blueprint && node.Type != NodeType::Simple )
				continue;

			const auto isSimple = node.Type == NodeType::Simple;

			builder.Begin( node.ID );

			if( !isSimple )
			{
				builder.Header( node.Color );

				ImGui::Spring( 0 );
				ImGui::TextUnformatted( node.Name.c_str() );
				ImGui::Spring( 1 );
				ImGui::Dummy( ImVec2( 0, 28 ) );
				ImGui::Spring( 0 );

				builder.EndHeader();
			}

			for( auto& input : node.Inputs )
			{
				auto alpha = ImGui::GetStyle().Alpha;

				builder.Input( input.ID );

				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, alpha );

				DrawPinIcon( input, IsPinLinked( input.ID ), alpha );

				ImGui::Spring( 0 );

				if( !input.Name.empty() )
				{
					ImGui::TextUnformatted( input.Name.c_str() );
					ImGui::Spring( 0 );
				}

				ImGui::PopStyleVar();

				builder.EndInput();
			}

			if( isSimple )
			{
				builder.Middle();

				ImGui::Spring( 1, 0 );
				ImGui::TextUnformatted( node.Name.c_str() );
				ImGui::Spring( 1, 0 );
			}

			for( auto& output : node.Outputs )
			{
				if( !isSimple && output.Type == PinType::Delegate )
					continue;

				auto alpha = ImGui::GetStyle().Alpha;

				//if( newLinkPin && !CanCreateLink( newLinkPin, &output ) && &output != newLinkPin )
				//	alpha = alpha * ( 48.0f / 255.0f );

				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, alpha );

				builder.Output( output.ID );

				if( !output.Name.empty() )
				{
					ImGui::Spring( 0 );
					ImGui::TextUnformatted( output.Name.c_str() );
				}

				builder.EndOutput();

				ImGui::PopStyleVar();
			}

			builder.End();
		}

		// Link the links
		for( auto& link : m_Links )
			ed::Link( link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f );

		ed::End();

		ImGui::End();
	}

}