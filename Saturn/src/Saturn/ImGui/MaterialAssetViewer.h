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

#include "AssetViewer.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/NodeEditor/NodeEditorBase.h"
#include "Saturn/NodeEditor/Runtime/NodeEditorRuntime.h"
#include "Saturn/NodeEditor/Runtime/NodeRuntime.h"

namespace Saturn {

	class NodeEditor;
	class Node;

	class MaterialNodeLibrary
	{
	public:
		MaterialNodeLibrary() = default;
		~MaterialNodeLibrary() = default;

		static NodeEditorType GetStaticType() { return NodeEditorType::Material; }

		static Ref<Node> SpawnOutputNode( Ref<NodeEditorBase> rNodeEditor );

		static Ref<Node> SpawnGetAsset( Ref<NodeEditorBase> rNodeEditor );
		static Ref<Node> SpawnColorPicker( Ref<NodeEditorBase> rNodeEditor );
		static Ref<Node> SpawnSampler2D( Ref<NodeEditorBase> rNodeEditor );
	};

	class MaterialNodeEditorRuntime : public NodeEditorRuntime
	{
	public:
		MaterialNodeEditorRuntime( const MaterialNodeEditorRuntime& ) = delete;

		struct MaterialNodeEdInfo
		{
			Ref<MaterialAsset> HostMaterial;
			UUID OutputNodeID;
		};

	public:
		MaterialNodeEditorRuntime( const MaterialNodeEdInfo& rInfo );
		virtual ~MaterialNodeEditorRuntime() = default;

		void SetTargetNodeEditor( Ref<NodeEditorBase> nodeEditor ) { m_NodeEditor = nodeEditor; }

		[[nodiscard]] virtual NodeEditorCompilationStatus Execute() override;

	private:
		size_t IsOutputsLinkedToOutNode( const Ref<Node>& rNode );

	private:
		MaterialNodeEdInfo m_Info;
		Ref<NodeEditorBase> m_NodeEditor;
	};

	class ColorPickerNodeRuntime : public NodeRuntime
	{
	public:
		explicit ColorPickerNodeRuntime( UUID id ) : NodeRuntime( id ) {}

		virtual void EvaluateNode() override {}

	public:
		glm::vec4 Value{};
	};

	class MaterialOutputNodeRuntime : public NodeRuntime 
	{
	public:
		struct TextureValue
		{
			uint32_t Slot = 0;
			glm::vec4 Color;
			UUID TextureAssetID = 0;
		};
	public:
		explicit MaterialOutputNodeRuntime( UUID id ) : NodeRuntime( id ) {}

		virtual void EvaluateNode() override;

	private:
		void HandleAlbedo( const TextureValue& rTextureVale );
	public:
		std::stack<TextureValue> TextureStack;
		Ref<MaterialAsset> MaterialAsset;
		Ref<Node> OutputNode;
	};

	class MaterialAssetViewer : public AssetViewer
	{
	public:
		MaterialAssetViewer( AssetID id );
		~MaterialAssetViewer();

		virtual void OnImGuiRender() override;
		virtual void OnUpdate( Timestep ts ) override {}
		virtual void OnEvent( RubyEvent& rEvent ) override {}

	private:
		void AddMaterialAsset();
		void DrawInternal();

		void SetupNodeEditorCallbacks();
		void SetupNewNodeEditor();
		void SetupNodesFromMaterial();
		void CreateNodesFromTexture( const Ref<Texture2D>& rTexture, int slot );

	private:
		Ref<MaterialAsset> m_HostMaterialAsset = nullptr;
		Ref<Material> m_EditingMaterial = nullptr;

		Ref<NodeEditor> m_NodeEditor = nullptr;

		UUID m_OutputNodeID = 0;
	};
}