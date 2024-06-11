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

#pragma once

#include "Saturn/NodeEditor/NodeEditorBase.h"

#include "Saturn/NodeEditor/NodeEditorCompilationStatus.h"

namespace Saturn {

	class NodeEditor : public NodeEditorBase
	{
	public:
		NodeEditor();
		NodeEditor( AssetID ID );
		~NodeEditor();

		bool CanCreateLink( const Ref<Pin>& a, const Ref<Pin>& b );	

		virtual void OnImGuiRender() override;
		virtual void OnUpdate( Timestep ts ) override {}
		virtual void OnEvent( RubyEvent& rEvent ) override {}

		void Open( bool open ) { m_WindowOpen = open; }

		void SetDetailsFunction( std::function<void( Ref<Node> )>&& rrDetailsFunction )
		{
			m_DetailsFunction = std::move( rrDetailsFunction );
		}

		// Happens when the user clicks on the empty space.
		void SetCreateNewNodeFunction( std::function<Ref<Node>()>&& rrCreateNewNodeFunction )
		{
			m_CreateNewNodeFunction = std::move( rrCreateNewNodeFunction );
		}

		void SetCloseFunction( std::function<void()>&& rrCloseFunction )
		{
			m_OnClose = std::move( rrCloseFunction );
		}

		std::string& GetEditorState() { return m_ActiveNodeEditorState; }
		const std::string& GetEditorState() const { return m_ActiveNodeEditorState; }
		
		void SetEditorState( const std::string& rState ) { m_ActiveNodeEditorState = rState; }

		NodeEditorCompilationStatus ThrowError( const std::string& rMessage );
		void ThrowWarning( const std::string& rMessage );

		void Reload();

		void SetWindowName( const std::string& rName ) { m_Name = rName; }

	private:
		virtual void SerialiseData( std::ofstream& rStream ) override;
		virtual void DeserialiseData( std::ifstream& rStream ) override;

	private:
		void CreateEditor();
		void Close();
		void DeleteDeadLinks( UUID nodeID );
		void DeleteLink( UUID id );

	private:
		bool m_CreateNewNode = false;

		Ref<Pin> m_NewLinkPin = nullptr;
		Ref<Pin> m_NewNodeLinkPin = nullptr;

		std::function<void( Ref<Node> )> m_DetailsFunction;
		std::function<Ref<Node>()> m_CreateNewNodeFunction;
		std::function<void()> m_OnClose;

		ImVec2 m_ViewportSize;

		Ref<Texture2D> m_ZoomTexture;
		Ref<Texture2D> m_CompileTexture;

	private:
		friend class NodeEditorCache;
		friend class NodeCacheSettings;
	};
}