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

#pragma once

#include "Saturn/ImGui/AssetViewer.h"
#include "Node.h"
#include "imgui_node_editor.h"

#include "NodeEditorCompilationStatus.h"

namespace ed = ax::NodeEditor;

namespace Saturn {

	class NodeEditor : public AssetViewer
	{
	public:
		NodeEditor( AssetID ID );
		~NodeEditor();

		Node* AddNode( NodeSpecification& spec, ImVec2 position = ImVec2( 0.0f, 0.0f ) );

		bool IsPinLinked( ed::PinId id );
		bool CanCreateLink( Pin* a, Pin* b );
		Pin* FindPin( ed::PinId id );
		Link* FindLink( ed::LinkId id );
		Node* FindNode( ed::NodeId id );
		Link* FindLinkByPin( ed::PinId id );
		Node* FindNodeByPin( ed::PinId id );

		const std::vector<Node>& GetNodes() const { return m_Nodes; }
		std::vector<Node>& GetNodes() { return m_Nodes; }

		const std::vector<Link>& GetLinks() const { return m_Links; }
		std::vector<Link>& GetLinks() { return m_Links; }

		virtual void OnImGuiRender() override;
		virtual void OnUpdate( Timestep ts ) override {}
		virtual void OnEvent( Event& rEvent ) override {}

		void Open( bool open ) { m_Open = open; }

		// Only use this when creating a node that needs to link automatically.
		void LinkPin( ed::PinId Start, ed::PinId End );

		void SetDetailsFunction( std::function<void( Node )>&& rrDetailsFunction )
		{
			m_DetailsFunction = std::move( rrDetailsFunction );
		}

		// Happens when the user clicks on the empty space.
		void SetCreateNewNodeFunction( std::function<Node*()>&& rrCreateNewNodeFunction )
		{
			m_CreateNewNodeFunction = std::move( rrCreateNewNodeFunction );
		}

		void SetCompileFunction( std::function<NodeEditorCompilationStatus()>&& rrCompileFunction )
		{
			m_OnCompile = std::move( rrCompileFunction );
		}

		void SetCloseFunction( std::function<void()>&& rrCloseFunction )
		{
			m_OnClose = std::move( rrCloseFunction );
		}

		AssetID GetAssetID() { return m_AssetID; }

		const std::string& GetEditorState() { return m_NodeEditorState; }
		
		void SetEditorState( const std::string& rState ) { m_NodeEditorState = rState; }

		NodeEditorCompilationStatus ThrowError( const std::string& rMessage );
		void ThrowWarning( const std::string& rMessage );

		void Reload();

		void SetWindowName( const std::string& rName ) { m_Name = rName; }

	private:

		void CreateEditor();
		void Close();
		void DeleteDeadLinks( ed::NodeId id );

	private:
		ed::EditorContext* m_Editor;

		std::string m_NodeEditorState;

		bool m_CreateNewNode = false;
		Pin* m_NewLinkPin = nullptr;
		Pin* m_NewNodeLinkPin = nullptr;

		std::function<void( Node )> m_DetailsFunction;
		std::function<Node*()> m_CreateNewNodeFunction;
		std::function<NodeEditorCompilationStatus()> m_OnCompile;
		std::function<void()> m_OnClose;

		std::vector<Node> m_Nodes;
		std::vector<Link> m_Links;

		std::string m_Name;
	};
}