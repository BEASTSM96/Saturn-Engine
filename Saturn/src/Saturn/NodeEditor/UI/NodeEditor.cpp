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
#include "NodeEditor.h"
#include "Saturn/Asset/AssetManager.h"

#include "Saturn/ImGui/ImGuiAuxiliary.h"

#include "Saturn/Serialisation/RawSerialisation.h"

#include "Saturn/NodeEditor/Runtime/NodeEditorRuntime.h"

#include "Saturn/NodeEditor/Serialisation/NodeCache.h"

#include "Saturn/ImGui/EditorIcons.h"

#include "Saturn/NodeEditor/GlobalNodesList.h"

#include "Saturn/Core/OptickProfiler.h"

namespace util = ax::NodeEditor::Utilities;

namespace Saturn {

	static constexpr inline bool operator==( const ImVec2& lhs, const ImVec2& rhs ) { return lhs.x == rhs.x && lhs.y == rhs.y; }
	static constexpr inline bool operator!=( const ImVec2& lhs, const ImVec2& rhs ) { return !( lhs == rhs ); }

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

	//////////////////////////////////////////////////////////////////////////
	// NODE EDITOR

	struct SelectAssetInfo
	{
		ed::PinId   ID = 0;
		ed::NodeId  NodeID = 0;
		AssetID     Asset = 0;
		std::string AssetName = "";
		AssetType DesiredAssetType = AssetType::Unknown;
	};

	static SelectAssetInfo s_SelectAssetInfo;

	// TODO: What if we do want to add this node editor the the asset viewers list?
	NodeEditor::NodeEditor( AssetID ID )
		: NodeEditorBase( ID )
	{
		m_AssetID = ID;

		CreateEditor();
	}

	NodeEditor::NodeEditor()
		: NodeEditorBase()
	{
		m_Editor = nullptr;
	}

	NodeEditor::~NodeEditor()
	{
		m_ZoomTexture = nullptr;
		m_CompileTexture = nullptr;
		
		Close();
	}

	void NodeEditor::CreateEditor()
	{
		// Zoom Levels in imgui_node_editor work backwards
		ImVector<float> zoomLvls;
		zoomLvls.push_back( 0.25f ); // Highest zoom level (least zoomed in)
		zoomLvls.push_back( 0.5f );
		zoomLvls.push_back( 0.75f );
		zoomLvls.push_back( 1.0f );
		zoomLvls.push_back( 1.25f );
		zoomLvls.push_back( 1.50f ); // Lowest zoom level (most zoomed in)

		ed::Config config;
		config.SettingsFile = nullptr;
		config.UserPointer = this;
		config.CustomZoomLevels = zoomLvls;

		config.SaveSettings = []( const char* pData, size_t size, ed::SaveReasonFlags reason, void* pUserPointer ) -> bool
		{
			auto* pThis = static_cast< NodeEditor* >( pUserPointer );

			pThis->m_ActiveNodeEditorState = pData;

			// TODO: Filter reasons
			// Node editor is always modified when loading.
			pThis->MarkDirty();

			return true;
		};

		config.LoadSettings = []( char* pData, void* pUserData ) -> size_t
		{
			auto* pThis = static_cast< NodeEditor* >( pUserData );

			const auto& State = pThis->m_ActiveNodeEditorState;

			if( !pData )
			{
				return State.size();
			}
			else
			{
				memcpy( pData, State.data(), State.size() );
			}

			return 0;
		};

		config.LoadNodeSettings = []( ed::NodeId nodeId, char* pData, void* pUserPointer ) -> size_t
		{
			auto* pThis = static_cast< NodeEditor* >( pUserPointer );

			auto pNode = pThis->FindNode( UUID( nodeId.Get() ) );

			if( !pNode )
				return 0;

			if( pData != nullptr )
				memcpy( pData, pNode->ActiveState.data(), pNode->ActiveState.size() );

			return pNode->ActiveState.size();
		};

		config.SaveNodeSettings = []( ed::NodeId nodeId, const char* pData, size_t size, ed::SaveReasonFlags reason, void* pUserPointer ) -> bool
		{
			auto* pThis = static_cast< NodeEditor* >( pUserPointer );

			auto pNode = pThis->FindNode( UUID( nodeId.Get() ) );

			if( !pNode )
				return false;

			pNode->ActiveState.assign( pData, size );
			pNode->Position = ed::GetNodePosition( nodeId );
			pNode->Size = ed::GetNodeSize( nodeId );

			// TODO: Filter reasons
			pThis->MarkDirty();

			return true;
		};

		m_Editor = ed::CreateEditor( &config );
		ed::SetCurrentEditor( m_Editor );
	
		m_ZoomTexture = EditorIcons::GetIcon( "Inspect" );
		m_CompileTexture = EditorIcons::GetIcon( "NoIcon" );
		
		auto texture = NodeEditorBase::GetBlueprintBackground();
		m_Builder = util::BlueprintNodeBuilder( ( ImTextureID ) texture->GetDescriptorSet(), texture->Width(), texture->Height() );

		m_OutputWindow.PushMessage( { .MessageText = "Initialised new editor!", .Type = NodeEditorMessageType::Info } );
	}

	void NodeEditor::Reload()
	{
		m_Dirty = false;

		ed::SetCurrentEditor( nullptr );

		ed::DestroyEditor( m_Editor );

		m_Editor = nullptr;

		CreateEditor();
	}

	void NodeEditor::Close()
	{
		m_OutputWindow.ClearOutput();

		ed::DestroyEditor( m_Editor );
		ed::SetCurrentEditor( nullptr );
		m_Editor = nullptr;

		for( auto& [id, rNode] : m_Nodes )
		{
			rNode->Destroy();
		}

		m_Nodes.clear();
		m_Links.clear();
		m_ActiveNodeEditorState = "";
	}

	bool NodeEditor::CanCreateLink( const Ref<Pin>& a, const Ref<Pin>& b )
	{
		if( !a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node )
			return false;

		return true;
	}

	void NodeEditor::OnImGuiRender()
	{
//		SAT_PF_EVENT();

		// Safety
		ed::SetCurrentEditor( m_Editor );

		if( !m_WindowOpen )
			return;

		// Draw main window
		ImGui::Begin( m_Name.c_str(), &m_WindowOpen, m_Dirty ? ImGuiWindowFlags_UnsavedDocument : 0 );

		if( m_ShowUnsavedChanges )
			ImGui::OpenPopup( "Unsaved Changes" );

		// Unsaved changes modal
		// TODO: Center window with our main window
		if( ImGui::BeginPopupModal( "Unsaved Changes", &m_ShowUnsavedChanges ) )
		{
			ImGui::Text( "You have unsaved changes to this editor." );
			ImGui::Text( "Would you like to save before closing?" );

			ImGui::BeginHorizontal( "##DirtyModalOpt" );

			if( ImGui::Button( "Save" ) )  
			{
				SaveSettings();
				NodeCacheEditor::WriteNodeEditorCache( this );

				m_Dirty = false;
				m_WindowOpen = false;

				m_ShowUnsavedChanges = false;
				ImGui::CloseCurrentPopup();
			}

			if( ImGui::Button( "Discard changes" ) ) 
			{
				m_Dirty = false;
				m_WindowOpen = false;

				m_ShowUnsavedChanges = false;
				ImGui::CloseCurrentPopup();
			}

			if( ImGui::Button( "Cancel" ) ) 
			{
				m_ShowUnsavedChanges = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndHorizontal();

			ImGui::EndPopup();
		}

		if( m_ViewportSize != ImGui::GetContentRegionAvail() )
			m_ViewportSize = ImGui::GetContentRegionAvail();

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar;

		ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );

		ImGui::BeginChild( "Topbar", ImVec2( 0, 30 ), 0, flags );
		ImGui::BeginHorizontal( "##TopbarItems" );

		if( Auxiliary::ImageButton( m_ZoomTexture, { 24, 24 } ) )
			ed::NavigateToContent( 0.25f );

		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();

			ImGui::Text( "Zoom in and find the content." );

			ImGui::EndTooltip();
		}

		if( Auxiliary::ImageButton( m_CompileTexture, { 24, 24 } ) )
		{
			m_OutputWindow.ClearOutput();

			if( m_Runtime )
			{
				NodeEditorCompilationStatus result = m_Runtime->EvaluateEditor();

				switch( result )
				{
					case NodeEditorCompilationStatus::Success:
					{
						m_OutputWindow.PushMessage( { .MessageText = "Successfully compiled and evaluated node editor!", .Type = NodeEditorMessageType::Info } );
					} break;

					case NodeEditorCompilationStatus::Failed: 
					{
						m_OutputWindow.PushMessage( { .MessageText = "Failed to compile node editor.", .Type = NodeEditorMessageType::Error } );
					} break;
				}
			}
			else
				m_OutputWindow.PushMessage( { .MessageText = "No active compiler was found!", .Type = NodeEditorMessageType::Error } );
		}

		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();

			ImGui::Text( "Compile and evaluate the node editor." );

			ImGui::EndTooltip();
		}

		if( m_TopbarItemsFunction )
			m_TopbarItemsFunction();

		ImGui::EndHorizontal();
		ImGui::EndChild();
		ImGui::PopStyleColor();

		// Hand off to imgui_node_editor and draw the actual node editor and nodes
		ed::Begin( "Node Editor", ImGui::GetContentRegionAvail() );

		auto cursorTopLeft = ImGui::GetCursorScreenPos();

		for( auto& [id, rNode] : m_Nodes )
		{
			rNode->Render( m_Builder, this );
		}

		for( const auto& rLink : m_Links )
			ed::Link( ed::LinkId( rLink->ID ), ed::PinId( rLink->StartPinID ), ed::PinId( rLink->EndPinID ) );

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
					Ref<Pin> StartPin = nullptr;
					Ref<Pin> EndPin = nullptr;

					StartPin = FindPin( UUID( StartPinId.Get() ) );
					EndPin = FindPin( UUID( EndPinId.Get() ) );

					m_NewLinkPin = StartPin ? StartPin : EndPin;

					// If we started from an input swap the start to the output side
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
							showLabel( "x Cannot link to self!", ImColor( 45, 32, 32, 180 ) );
							ed::RejectNewItem( ImColor( 225, 0, 0 ), 2.0f );
						}
						else if( EndPin->Kind == StartPin->Kind )  // Same kind, input/output into input/output.
						{
							showLabel( "x Incompatible Pin Kind, input/output into input/output", ImColor( 45, 32, 32, 180 ) );

							ed::RejectNewItem( ImColor( 225, 0, 0 ), 2.0f );
						}
						else if( EndPin->Type != StartPin->Type )
						{
							showLabel( "x Incompatible Pin Type!", ImColor( 45, 32, 32, 180 ) );

							ed::RejectNewItem( ImColor( 225, 128, 128 ), 2.0f );
						}
						else // Valid type, accept (create new link)
						{
							showLabel( "+ Create Link", ImColor( 32, 45, 32, 180 ) );
							
							if( ed::AcceptNewItem( ImColor( 128, 255, 128 ), 4.0f ) )
							{
								UUID start = UUID( StartPinId.Get() );
								UUID end = UUID( EndPinId.Get() );

								m_Links.push_back( Ref<Link>::Create( UUID(), start, end ) );

								MarkDirty();
							}
						}
					}
				}

				// If the link is not connected, user maybe want to create a new node rather than link it.
				ed::PinId id = 0;
				if( ed::QueryNewNode( &id ) )
				{
					m_NewLinkPin = FindPin( UUID( id.Get() ) );

					if( m_NewLinkPin )
						showLabel( "+ Create Node", ImColor( 32, 45, 32, 180 ) );

					if( ed::AcceptNewItem() )
					{
						m_CreateNewNode = true;

						m_NewNodeLinkPin = FindPin( UUID( id.Get() ) );
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

			if( ed::BeginDelete() )
			{
				ed::LinkId linkId = 0;
				while( ed::QueryDeletedLink( &linkId ) )
				{
					if( ed::AcceptDeletedItem() )
					{
						DeleteLink( linkId.Get() );

						MarkDirty();
					}
				}

				ed::NodeId nodeId = 0;
				while( ed::QueryDeletedNode( &nodeId ) )
				{
					UUID id = nodeId.Get();

					const auto itr = std::find_if( m_Nodes.begin(), m_Nodes.end(),
						[id]( const auto& kv )
						{
							return kv.first == id;
						} );

					if( itr != m_Nodes.end() )
					{
						auto& rNode = ( itr->second );

						if( rNode->CanBeDeleted )
						{
							if( ed::AcceptDeletedItem() )
							{
								DeleteDeadLinks( id );

								rNode = nullptr;
								m_Nodes.erase( itr );

								MarkDirty();
							}
						}
						else
						{
							ed::RejectDeletedItem();
						}
					}
				}
			}
			ed::EndDelete();
		}

		ImGui::SetCursorScreenPos( cursorTopLeft );

		ed::Suspend();

		if( ed::ShowBackgroundContextMenu() )
		{
			ImGui::OpenPopup( "Create New Node" );
			m_NewNodeLinkPin = nullptr;
		}
		ed::Resume();

		ed::Suspend();

		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 8, 8 ) );
		if( ImGui::BeginPopup( "Create New Node" ) )
		{
			auto mousePos = ed::ScreenToCanvas( ImGui::GetMousePosOnOpeningCurrentPopup() );

			Ref<Node> node = nullptr;

			if( m_CreateNewNodeFunction )
				node = m_CreateNewNodeFunction();

			if( node )
			{
				BuildNode( node );

				m_CreateNewNode = false;

				ed::SetNodePosition( ed::NodeId( node->ID ), mousePos );

				if( auto& startPin = m_NewNodeLinkPin ) 
				{
					auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

					for( auto& pin : pins ) 
					{
						if( CanCreateLink( startPin, pin ) )
						{
							auto& endPin = pin;

							UUID startID = startPin->ID;
							UUID endID = endPin->ID;

							// Start of the link must be the output pin
							if( startPin->Kind == PinKind::Input ) 
								std::swap( startID, endID );

							m_Links.emplace_back( Ref<Link>::Create( UUID(), startID, endID ) );
							m_Links.back()->Color = startPin->GetPinColor();

							break;
						}
					}
				}

				MarkDirty();
			}

			ImGui::EndPopup();
		}
		else
			m_CreateNewNode = false;

		ImGui::PopStyleVar();
		ed::Resume();

		ed::End();

		m_OutputWindow.Draw();

		ImGui::End(); // NODE_EDITOR

		// Window closed but we are dirty, show unsaved changes modal and keep window open
		if( !m_WindowOpen && m_Dirty )
		{
			m_WindowOpen = true;
		
			m_ShowUnsavedChanges = true;
		}
	}

	void NodeEditor::ThrowError( const std::string & rMessage )
	{
		m_OutputWindow.PushMessage( { .MessageText = rMessage, .Type = NodeEditorMessageType::Error } );
	}

	void NodeEditor::ThrowWarning( const std::string& rMessage )
	{
		m_OutputWindow.PushMessage( { .MessageText = rMessage, .Type = NodeEditorMessageType::Warning } );
	}

	void NodeEditor::PushInfoMessage( const std::string& rMessage )
	{
		m_OutputWindow.PushMessage( { .MessageText = rMessage, .Type = NodeEditorMessageType::Info } );
	}

	void NodeEditor::DeleteDeadLinks( UUID nodeID )
	{
		auto wasConnectedToTheNode = [&]( const Ref<Link>& link )
			{
				return ( !FindPin( link->StartPinID ) ) || ( !FindPin( link->EndPinID ) )
					|| FindPin( link->StartPinID )->Node->ID == nodeID
					|| FindPin( link->EndPinID )->Node->ID == nodeID;
			};

		auto removeIt = std::remove_if( m_Links.begin(), m_Links.end(), wasConnectedToTheNode );
		m_Links.erase( removeIt, m_Links.end() );
	}

	void NodeEditor::DeleteLink( UUID id )
	{
		const auto Itr = std::find_if( m_Links.begin(), m_Links.end(),
			[id]( const auto& rLink )
			{
				return rLink->ID == id;
			} );

		if( Itr != m_Links.end() )
		{
			m_Links.erase( Itr );
		}
	}

	void NodeEditor::CreateNewEditorIfNeeded()
	{
		if( !m_Editor )
			CreateEditor();
	}

	//////////////////////////////////////////////////////////////////////////
	// SERIALISATION

	void NodeEditor::SerialiseData( std::ofstream& rStream )
	{
		RawSerialisation::WriteString( m_Name, rStream );

		size_t mapSize = m_Nodes.size();
		rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

		for( const auto& [key, value] : m_Nodes )
		{
			UUID::Serialise( key, rStream );

			RawSerialisation::WriteObject( (uint64_t)value->ExecutionType, rStream );

			Node::Serialise( value, rStream );
		}

		mapSize = m_Links.size();
		RawSerialisation::WriteObject( mapSize, rStream );

		for( auto& rLinks : m_Links )
		{
			Link::Serialise( rLinks, rStream );
		}
	}

	void NodeEditor::DeserialiseData( std::ifstream& rStream )
	{
		m_Loading = true;

		NodeCacheSettings::ReadEditorSettings( this );

		m_Name = RawSerialisation::ReadString( rStream );

		CreateNewEditorIfNeeded();

		size_t mapSize = 0;
		RawSerialisation::ReadObject( mapSize, rStream );

		for( size_t i = 0; i < mapSize; i++ )
		{
			UUID key = 0;
			UUID::Deserialise( key, rStream );

			uint64_t executionValue = 0;
			RawSerialisation::ReadObject( executionValue, rStream );
			NodeExecutionType executionType = ( NodeExecutionType ) executionValue;

			Ref<Node> node = GlobalNodesList::ConvertExecutionTypeToNode( executionType, this );
			
			if( !node )
				node = Ref<Node>::Create();

			Node::Deserialise( node, rStream );

			m_Nodes[ key ] = node;
			BuildNode( node );
		}

		mapSize = 0;
		RawSerialisation::ReadObject( mapSize, rStream );

		m_Links.resize( mapSize );

		for( size_t i = 0; i < mapSize; i++ )
		{
			Ref<Link> link = Ref<Link>::Create();

			Link::Deserialise( link, rStream );

			m_Links[ i ] = link;
		}

		m_Loading = false;
	}
}