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

namespace Saturn {

	class EditorAssetRegistry : public AssetRegistryBase
	{
	public:
		static inline EditorAssetRegistry& Get() { return *SingletonStorage::Get().GetSingleton<EditorAssetRegistry>(); }
	public:
		EditorAssetRegistry();
		~EditorAssetRegistry();

		virtual AssetID CreateAsset( AssetType type ) override;
		virtual Ref<Asset> FindAsset( AssetID id ) override;

		Ref<Asset> FindAsset( const std::filesystem::path& rPath );
		Ref<Asset> FindAsset( const std::string& rName, AssetType type );

		std::vector<AssetID> FindAssetsWithType( AssetType type ) const;

		AssetID PathToID( const std::filesystem::path& rPath );

		// I feel like the name is misleading as we are not really checking for missing asset ref, we are checking if an assets exists on the filesystem and not in the registry.
		void CheckMissingAssetRefs();

	private:
		void AddAsset( AssetID id );

	private:
		friend class EditorAssetRegistrySerialiser;
	};

}