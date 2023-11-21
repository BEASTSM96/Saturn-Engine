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

		Ref<Node> AddNode( NodeSpecification& spec, ImVec2 position = ImVec2( 0.0f, 0.0f ) );

		bool IsPinLinked( ed::PinId id );
		bool CanCreateLink( const Ref<Pin>& a, const Ref<Pin>& b );
		
		Ref<Pin> FindPin( ed::PinId id );
		const Ref<Pin>& FindPin( ed::PinId id ) const;
		
		Ref<Link> FindLink( ed::LinkId id );
		const Ref<Link>& FindLink( ed::LinkId id ) const;
	
		Ref<Node> FindNode( ed::NodeId id );
		const Ref<Node>& FindNode( ed::NodeId id ) const;
	
		Ref<Link> FindLinkByPin( ed::PinId id );
		Ref<Node> FindNodeByPin( ed::PinId id );

		const std::vector<Ref<Node>>& GetNodes() const { return m_Nodes; }
		std::vector<Ref<Node>>& GetNodes() { return m_Nodes; }

		const std::vector<Ref<Link>>& GetLinks() const { return m_Links; }
		std::vector<Ref<Link>>& GetLinks() { return m_Links; }

		virtual void OnImGuiRender() override;
		virtual void OnUpdate( Timestep ts ) override {}
		virtual void OnEvent( RubyEvent& rEvent ) override {}

		void Open( bool open ) { m_Open = open; }

		// Only use this when creating a node that needs to link automatically.
		void LinkPin( ed::PinId Start, ed::PinId End );

		void SetDetailsFunction( std::function<void( Ref<Node> )>&& rrDetailsFunction )
		{
			m_DetailsFunction = std::move( rrDetailsFunction );
		}

		// Happens when the user clicks on the empty space.
		void SetCreateNewNodeFunction( std::function<Ref<Node>()>&& rrCreateNewNodeFunction )
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

		std::string& GetEditorState() { return m_NodeEditorState; }
		const std::string& GetEditorState() const { return m_NodeEditorState; }
		
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

		Ref<Pin> m_NewLinkPin = nullptr;
		Ref<Pin> m_NewNodeLinkPin = nullptr;

		std::function<void( Ref<Node> )> m_DetailsFunction;
		std::function<Ref<Node>()> m_CreateNewNodeFunction;
		std::function<NodeEditorCompilationStatus()> m_OnCompile;
		std::function<void()> m_OnClose;

		std::vector<Ref<Node>> m_Nodes;
		std::vector<Ref<Link>> m_Links;

		std::string m_Name;
	};
}