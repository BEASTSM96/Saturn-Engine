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

#include "Saturn/ImGui/AssetViewer.h"

#include "Node.h"
#include "Link.h"

#include "imgui_node_editor.h"

namespace ed = ax::NodeEditor;

namespace Saturn {

	enum class NodeEditorType
	{
		Unknown,
		Material,
		Sound
	};

	class NodeEditorRuntime;
	class Texture2D;

	class NodeEditorBase : public AssetViewer
	{
	public:
		NodeEditorBase();
		NodeEditorBase( AssetID id );
		virtual ~NodeEditorBase();

		virtual void OnImGuiRender() override = 0;
		virtual void OnUpdate( Timestep ts ) override = 0;
		virtual void OnEvent( RubyEvent& rEvent ) override = 0;

		static Ref<Texture2D> GetBlueprintBackground();

	public:
		AssetID GetAssetID() const { return m_AssetID; }

		const std::vector<Ref<Node>>& GetNodes() const { return m_Nodes; }
		std::vector<Ref<Node>>& GetNodes() { return m_Nodes; }

		const std::vector<Ref<Link>>& GetLinks() const { return m_Links; }
		std::vector<Ref<Link>>& GetLinks() { return m_Links; }

		Ref<Node> AddNode( const NodeSpecification& spec, ImVec2 position = ImVec2( 0.0f, 0.0f ) );

	protected:
		[[nodiscard]] int GetNextID() { return m_CurrentID++; }
		[[nodiscard]] int GeCurrentID() const { return m_CurrentID; }

	protected:
		std::string m_Name;

		ed::EditorContext* m_Editor = nullptr;
		std::string m_ActiveNodeEditorState;

		std::vector<Ref<Node>> m_Nodes;
		std::vector<Ref<Link>> m_Links;

		Ref<NodeEditorRuntime> m_Runtime;

	private:
		int m_CurrentID = 1;

	private:
		friend class NodeEditorCache;
		friend class NodeCacheEditor;
		friend class NodeCacheSettings;
	};
}