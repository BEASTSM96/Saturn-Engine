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

#include "Saturn/Asset/Asset.h"

namespace YAML {
	class Node;
}

namespace Saturn {

	class NodeEditor;

	class AssetSerialiser
	{
	public:
		virtual void Serialise   ( const Ref<Asset>& rAsset ) const = 0;
		virtual void Deserialise ( const Ref<Asset>& rAsset ) const = 0;
		virtual bool TryLoadData (       Ref<Asset>& rAsset ) const = 0;
	};

	class MaterialAssetSerialiser : public AssetSerialiser
	{
	public:
		virtual void Serialise  ( const Ref<Asset>& rAsset, NodeEditor* pNodeEditor ) const;
		virtual void Serialise  ( const Ref<Asset>& rAsset ) const override;
		virtual void Deserialise( const Ref<Asset>& rAsset ) const override;
		virtual bool TryLoadData(       Ref<Asset>& rAsset ) const override;

		void TryLoadData        (       Ref<Asset>& rAsset, bool LoadNodeEditorData, NodeEditor* pNodeEditor ) const;

	private:
		void LoadMaterialData( YAML::Node& rNode, Ref<Asset>& rAsset ) const;
	};

	class PrefabSerialiser : public AssetSerialiser
	{
	public:
		virtual void Serialise   ( const Ref<Asset>& rAsset ) const override;
		virtual void Deserialise ( const Ref<Asset>& rAsset ) const override;
		virtual bool TryLoadData ( Ref<Asset>& rAsset ) const override;
	};

	class StaticMeshAssetSerialiser : public AssetSerialiser
	{
	public:
		virtual void Serialise( const Ref<Asset>& rAsset ) const override;
		virtual void Deserialise( const Ref<Asset>& rAsset ) const override;
		virtual bool TryLoadData( Ref<Asset>& rAsset ) const override;
	};

	class Sound2DAssetSerialiser : public AssetSerialiser
	{
	public:
		virtual void Serialise( const Ref<Asset>& rAsset ) const override;
		virtual void Deserialise( const Ref<Asset>& rAsset ) const override;
		virtual bool TryLoadData( Ref<Asset>& rAsset ) const override;
	};
}