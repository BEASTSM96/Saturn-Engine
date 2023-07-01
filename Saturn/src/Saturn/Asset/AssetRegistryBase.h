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

#include "Asset.h"
#include "AssetImporter.h"

namespace Saturn {

	enum class AssetRegistryType
	{
		Game,
		Editor,
		Unknown
	};

	using AssetMap = std::unordered_map< AssetID, Ref<Asset> >;

	class AssetRegistryBase
	{
	public:
		AssetRegistryBase( AssetRegistryType Type );
		~AssetRegistryBase();

		virtual AssetID CreateAsset( AssetType type ) = 0;
		virtual Ref<Asset> FindAsset( AssetID id ) = 0;

		// Where Ty is an asset.
		// This will try to find the loaded asset, if it does not exists it will try to load it.
		// \return Ref<Ty> if found, nullptr if not
		template<typename Ty>
		Ref<Ty> GetAssetAs( AssetID id )
		{
			Ref<Asset> asset = m_Assets.at( id );

			if( !IsAssetLoaded( id ) )
			{
				bool loaded = AssetImporter::Get().TryLoadData( asset );
				if( !loaded )
					return nullptr;

				m_LoadedAssets[ id ] = asset;
			}
			else
				asset = m_LoadedAssets.at( id );

			return asset.As<Ty>();
		}

		const AssetMap& GetAssetMap() const { return m_Assets; }
		const AssetMap& GetLoadedAssetsMap() const { return m_LoadedAssets; }

	protected:
		virtual void AddAsset( AssetID id ) = 0;

		bool IsAssetLoaded( AssetID id );
	protected:
		AssetMap m_Assets;
		AssetMap m_LoadedAssets;

		AssetRegistryType m_Type = AssetRegistryType::Unknown;

	private:
		friend class GameAssetRegistrySerialiser;
	};

}