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
#include "Saturn/Vendor/widgets.h"
#include "Saturn/Vendor/Drawing.h"

#include <backends/imgui_impl_vulkan.h>

namespace util = ax::NodeEditor::Utilities;

namespace Saturn {

	static inline ImVec2 operator+( const ImVec2& lhs, const ImVec2& rhs ) { return ImVec2( lhs.x + rhs.x, lhs.y + rhs.y ); }
	static inline ImVec2 operator-( const ImVec2& lhs, const ImVec2& rhs ) { return ImVec2( lhs.x - rhs.x, lhs.y - rhs.y ); }

	static int s_ID = 1;

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
	}

	NodeEditor::~NodeEditor()
	{
		s_BlueprintBackground = nullptr;
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

	Pin* NodeEditor::FindPin( ed::PinId id )
	{
		if( !id )
			return nullptr;

		for( auto& node : m_Nodes )
		{
			for( auto& pin : node.Inputs )
				if( pin.ID == id )
					return &pin;

			for( auto& pin : node.Outputs )
				if( pin.ID == id )
					return &pin;
		}

		return nullptr;
	}

	Link* NodeEditor::FindLink( ed::LinkId id )
	{
		for( auto& link : m_Links )
			if( link.ID == id )
				return &link;
	}

	Node* NodeEditor::FindNode( ed::NodeId id )
	{
		for( auto& node : m_Nodes )
			if( node.ID == id )
				return &node;
	}

	ImColor GetIconColor( PinType type )
	{
		switch( type )
		{
			default:
			case PinType::Flow:     return ImColor( 255, 255, 255 ); // Same as Material_Sampler2D
			case PinType::Bool:     return ImColor( 220, 48, 48 );
			case PinType::Int:      return ImColor( 68, 201, 156 );
			case PinType::Float:    return ImColor( 147, 226, 74 );
			case PinType::String:   return ImColor( 124, 21, 153 );
			case PinType::Object:   return ImColor( 51, 150, 215 );
			case PinType::Function: return ImColor( 218, 0, 183 );
			case PinType::Delegate: return ImColor( 255, 48, 48 );
		}
	}

	void DrawPinIcon( const Pin& pin, bool connected, int alpha )
	{
		ax::Drawing::IconType type;
		ImColor color = GetIconColor( pin.Type );
		color.Value.w = alpha / 255.0f;

		switch( pin.Type )
		{
			case PinType::Flow:				  type = ax::Drawing::IconType::Flow;   break;
			case PinType::Bool:				  type = ax::Drawing::IconType::Circle; break;
			case PinType::Int:				  type = ax::Drawing::IconType::Circle; break;
			case PinType::Float:			  type = ax::Drawing::IconType::Circle; break;
			case PinType::String:			  type = ax::Drawing::IconType::Circle; break;
			case PinType::Object:			  type = ax::Drawing::IconType::Circle; break;
			case PinType::Function:			  type = ax::Drawing::IconType::Circle; break;
			case PinType::Material_Sampler2D: type = ax::Drawing::IconType::Circle; break;
			case PinType::Delegate:           type = ax::Drawing::IconType::Square; break;
			default:
				return;
		}

		const float PIN_ICON_SIZE = 24;

		auto size = ImVec2( static_cast< float >( PIN_ICON_SIZE ), static_cast< float >( PIN_ICON_SIZE ) );

		if( ImGui::IsRectVisible( size ) ) 
		{
			auto cursorPos = ImGui::GetCursorScreenPos();
			auto drawList  = ImGui::GetWindowDrawList();

			ax::Drawing::DrawIcon( drawList, cursorPos, cursorPos + size, type, connected, color, ImColor( 32, 32, 32, alpha ) );
		}

		ImGui::Dummy( size );
	}

	void NodeEditor::Draw()
	{
		// Safety
		ed::SetCurrentEditor( m_Editor );

		ImGui::Begin( "##NODE_EDITOR" );

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar;

		ImGui::Begin( "##NODE_EDITOR_TOPBAR", (bool*)0, flags );
		ImGui::BeginHorizontal( "##TopbarItems" );

		if( ImGui::Button( "Zoom to content" ) )
			ed::NavigateToContent( false );

		if( ImGui::Button( "Show flow" ) )
			for( auto& link : m_Links )
				ed::Flow( link.ID );

		ImGui::EndHorizontal();
		ImGui::End();

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

				DrawPinIcon( input, IsPinLinked( input.ID ), (int)(alpha * 255) );

				ImGui::Spring( 0 );

				if( !input.Name.empty() )
				{
					ImGui::TextUnformatted( input.Name.c_str() );
					ImGui::Spring( 0 );
				}

				if( input.Type == PinType::Bool ) 
				{
					ImGui::Button( "Hello" );
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

				if( m_NewLinkPin && !CanCreateLink( m_NewLinkPin, &output ) && &output != m_NewLinkPin )
					alpha = alpha * ( 48.0f / 255.0f );

				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, alpha );

				builder.Output( output.ID );

				if( !output.Name.empty() )
				{
					ImGui::Spring( 0 );
					ImGui::TextUnformatted( output.Name.c_str() );
				}

				ImGui::Spring( 0 );
				DrawPinIcon( output, IsPinLinked( output.ID ), ( int ) ( alpha * 255 ) );

				builder.EndOutput();
				ImGui::PopStyleVar();
			}

			builder.End();
		}

		// Link the links
		for( auto& link : m_Links )
			ed::Link( link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f );

		if( !m_CreateNewNode )
		{
			if( ed::BeginCreate( ImColor( 255, 255, 255 ), 2.0f ) )
			{
				auto showLabel = []( const char* label, ImColor color )
				{
					ImGui::SetCursorPosY( ImGui::GetCursorPosY() - ImGui::GetTextLineHeight() );
					auto size = ImGui::CalcTextSize( label );

					auto padding = ImGui::GetStyle().FramePadding;
					auto spacing = ImGui::GetStyle().ItemSpacing;

					ImGui::SetCursorPos( ImGui::GetCursorPos() + ImVec2( spacing.x, -spacing.y ) );

					auto rectMin = ImGui::GetCursorScreenPos() - padding;
					auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

					auto drawList = ImGui::GetWindowDrawList();
					drawList->AddRectFilled( rectMin, rectMax, color, size.y * 0.15f );
					ImGui::TextUnformatted( label );
				};

				ed::PinId StartPinId = 0;
				ed::PinId EndPinId = 0;

				if( ed::QueryNewLink( &StartPinId, &EndPinId ) )
				{
					Pin* StartPin = nullptr; 
					Pin* EndPin   = nullptr;

					StartPin = FindPin( StartPinId );
					EndPin = FindPin( EndPinId );

					m_NewLinkPin = StartPin ? StartPin : EndPin;

					if( StartPin->Kind == PinKind::Input ) 
					{
						std::swap( StartPin, EndPin );
						std::swap( StartPinId, EndPinId );
					}

					if( StartPin && EndPin )
					{
						// Pin is the same, reject.
						if( EndPin == StartPin )
						{
							ed::RejectNewItem( ImColor( 225, 0, 0 ), 2.0f );
						}
						else if( EndPin->Kind == StartPin->Kind )  // Same kind, input/output into input/output.
						{
							showLabel( "x Incompatible Pin Kind, input/output into input/output", ImColor( 45, 32, 32, 180 ) );

							ed::RejectNewItem( ImColor( 225, 0, 0 ), 2.0f );
						}
						else if( EndPin->Type != StartPin->Type )
						{
							showLabel( "x Incompatible Pin Type", ImColor( 45, 32, 32, 180 ) );

							ed::RejectNewItem( ImColor( 225, 128, 128 ), 2.0f );
						}
						else // Vaild type, accept
						{
							showLabel( "+ Create Link", ImColor( 32, 45, 32, 180 ) );
							if( ed::AcceptNewItem( ImColor( 128, 255, 128 ), 4.0f ) )
							{
								m_Links.emplace_back( Link( GetNextID(), StartPinId, EndPinId ) );
								m_Links.back().Color = GetIconColor( StartPin->Type );
							}
						}
					}
				}
			
				// If the link is not connected, user maybe want to create a new node rather than link it.
				ed::PinId id = 0;
				if( ed::QueryNewNode( &id ) ) 
				{
					m_NewLinkPin = FindPin( id );

					if( m_NewLinkPin  )
						showLabel( "+ Create Node", ImColor( 32, 45, 32, 180 ) );

					if( ed::AcceptNewItem() ) 
					{
						m_CreateNewNode = true;
						
						m_NewNodeLinkPin = FindPin( id );
						m_NewLinkPin = nullptr;

						ed::Suspend();
						ImGui::OpenPopup( "Create New Node" );
						ed::Resume();
					}
				}
			}
			else
				m_NewLinkPin = nullptr;

			ed::EndCreate();
		}

		ed::Suspend();
		if( ed::ShowBackgroundContextMenu() )
		{
			ImGui::OpenPopup( "Create New Node" );
			m_NewNodeLinkPin = nullptr;
		}

		if( ImGui::BeginPopup( "Create New Node" ) )
		{
			Node* node = nullptr;

			if( m_CreateNewNodeFunction )
				node = m_CreateNewNodeFunction();

			if( node )
			{
				BuildNode( node );

				ed::SetNodePosition( node->ID, ImGui::GetMousePos() );

				if( auto startPin = m_NewNodeLinkPin ) 
				{
					auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

					for( Pin& pin : pins ) 
					{
						if( CanCreateLink( startPin, &pin ) )
						{
							auto endPin = &pin;

							if( startPin->Kind == PinKind::Input )
								std::swap( startPin, endPin );

							m_Links.emplace_back( Link( GetNextID(), startPin->ID, endPin->ID ) );
							m_Links.back().Color = GetIconColor( startPin->Type );

							break;
						}
					}
				}
			}

			ImGui::EndPopup();
		}
		else
			m_CreateNewNode = false;

		ed::Resume();

		ed::End();

		ImGui::Begin( "Details##NODE_INFO_PANEL" );
		{
			if( !ed::IsNodeSelected( rNode.ID ) )
				continue;

			if( m_DetailsFunction )
				m_DetailsFunction( rNode );
		}

		ImGui::End();

		ImGui::End(); // NODE_EDITOR
	}

}