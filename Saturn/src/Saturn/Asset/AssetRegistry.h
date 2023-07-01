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

#include "AssetRegistryBase.h"

#include <unordered_map>
#include <unordered_set>

namespace Saturn {

	// The Game AssetRegistry
	class AssetRegistry : public AssetRegistryBase
	{
	public:
		static inline AssetRegistry& Get() { return *SingletonStorage::Get().GetSingleton<AssetRegistry>(); }
	public:
		AssetRegistry();
		~AssetRegistry();

		template<typename Ty, typename... Args>
		Ref<Asset> CreateAsset( AssetType type, Args&&... rrArgs )
		{
			// This might not be the best way, first we create the "real" asset and add it to the registry, then create the template asset.
			auto id = CreateAsset( type );

			Ref<Ty> asset = Ref<Ty>::Create( std::forward<Args>( rrArgs )... );
			asset->ID = id;
			asset->Type = type;

			return asset;
		}

		virtual AssetID CreateAsset( AssetType type ) override;
		virtual Ref<Asset> FindAsset( AssetID id ) override;

		Ref<Asset> FindAsset( const std::filesystem::path& rPath );
		Ref<Asset> FindAsset( const std::string& rName, AssetType type );

		std::vector<AssetID> FindAssetsWithType( AssetType type ) const;

		AssetID PathToID( const std::filesystem::path& rPath );

	private:

		void AddAsset( AssetID id );

	private:
		friend class GameAssetRegistrySerialiser;
	};
}