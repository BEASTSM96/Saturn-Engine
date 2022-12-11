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

#pragma once

#include "AssetViewer.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "NodeEditor/NodeEditorCompilationStatus.h"

namespace Saturn {

	class NodeEditor;

	class MaterialAssetViewer : public AssetViewer
	{
		SINGLETON( MaterialAssetViewer );
	public:
		MaterialAssetViewer();
		~MaterialAssetViewer();

		virtual void Draw() override;

		void AddMaterialAsset( Ref<Asset>& rAsset );

	private:
		void DrawInternal( Ref<MaterialAsset>& rMaterialAsset );

		NodeEditorCompilationStatus CheckOutputNodeInput( NodeEditor* pNodeEditor, int PinID, bool ThrowIfNotLinked, const std::string& rErrorMessage, int Index, bool AllowColorPicker, Ref<MaterialAsset>& rMaterialAsset );

	private:
		std::vector<Ref<MaterialAsset>> m_MaterialAssets;

		int m_OutputNodeID = 0;
	};
}