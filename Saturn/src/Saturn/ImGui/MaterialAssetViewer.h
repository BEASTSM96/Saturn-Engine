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
#include "NodeEditor/NodeEditorCompilationStatus.h"

namespace ax::NodeEditor {
	struct NodeId;
	struct PinId;
	struct LinkId;
}

namespace Saturn {

	class NodeEditor;
	class Node;

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

		ax::NodeEditor::NodeId FindOtherNodeIDByPin( ax::NodeEditor::PinId id );
		Ref<Node> FindOtherNodeByPin( ax::NodeEditor::PinId id );

		NodeEditorCompilationStatus CheckOutputNodeInput( int PinID, bool ThrowIfNotLinked, const std::string& rErrorMessage, int Index, bool AllowColorPicker, Ref<MaterialAsset>& rMaterialAsset );

		NodeEditorCompilationStatus Compile();

	private:
		Ref<MaterialAsset> m_HostMaterialAsset = nullptr;
		Ref<Material> m_EditingMaterial = nullptr;

		Ref<NodeEditor> m_NodeEditor = nullptr;

		int m_OutputNodeID = 0;
	};
}