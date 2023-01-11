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

#include "Asset.h"
#include "AssetImporter.h"

#include <unordered_map>
#include <unordered_set>

namespace Saturn {

	using AssetMap = std::unordered_map< AssetID, Ref<Asset> >;
	using AssetIDVector = std::vector< AssetID >;

	class AssetRegistry
	{
		SINGLETON( AssetRegistry );
	public:
		AssetRegistry();
		~AssetRegistry();

		AssetID CreateAsset( AssetType type );

		Ref<Asset> FindAsset( AssetID id );
		
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

		Ref<Asset> FindAsset( const std::filesystem::path& rPath );

		const std::vector<AssetID>& FindAssetsWithType( AssetType type ) const;

		const AssetMap& GetAssetMap() const { return m_Assets; }
		const AssetMap& GetLoadedAssetsMap() const { return m_LoadedAssets; }

		AssetID PathToID( const std::filesystem::path& rPath );

	private:
		AssetMap m_Assets;
		AssetMap m_LoadedAssets;

		bool IsAssetLoaded( AssetID id );

	private:

		void AddAsset( AssetID id );

	private:
		friend class AssetRegistrySerialiser;
	};
}