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

#include "AssetRegistry.h"

namespace Saturn {

	class AssetManager
	{
	public:
		static inline AssetManager& Get() { return *SingletonStorage::Get().GetSingleton<AssetManager>(); }
	public:
		AssetManager();
		~AssetManager();

		Ref<Asset> FindAsset( AssetID id, AssetRegistryType Dst );
		Ref<Asset> FindAsset( AssetID id );

		AssetID CreateAsset( AssetType type, AssetRegistryType Dst = AssetRegistryType::Game );

		Ref<Asset> FindAsset( const std::filesystem::path& rPath, AssetRegistryType Dst = AssetRegistryType::Game );
		Ref<Asset> FindAsset( const std::string& rName, AssetType type, AssetRegistryType Dst = AssetRegistryType::Game );

		template<typename Ty, typename... Args>
		Ref<Asset> CreateAsset( AssetType type, Args&&... rrArgs, AssetRegistryType Dst = AssetRegistryType::Game )
		{
			// This might not be the best way, first we create the "real" asset and add it to the registry, then create the template asset.
			auto id = CreateAsset( type, Dst );

			Ref<Ty> asset = Ref<Ty>::Create( std::forward<Args>( rrArgs )... );
			asset->ID = id;
			asset->Type = type;

			return asset;
		}

		// Where Ty is an asset.
		// This will try to find the loaded asset, if it does not exists it will try to load it.
		// \return Ref<Ty> if found, nullptr if not
		template<typename Ty>
		Ref<Ty> GetAssetAs( AssetID id, AssetRegistryType Dst )
		{
			switch( Dst )
			{
				case AssetRegistryType::Game: 
					return m_Assets->GetAssetAs<Ty>( id );

				case AssetRegistryType::Editor:
					return m_EditorAssets->GetAssetAs<Ty>( id );

				case AssetRegistryType::Unknown:
				default:
					return nullptr;
			}
		}

		template<typename Ty>
		Ref<Ty> GetAssetAs( AssetID id )
		{
			Ref<Ty> asset = m_Assets->GetAssetAs<Ty>( id );

			if( !asset )
				asset = m_EditorAssets->GetAssetAs<Ty>( id );

			return asset;
		}

		AssetMap GetCombinedAssetMap();
		AssetMap GetCombinedLoadedAssetMap();

		Ref<AssetRegistry>& GetAssetRegistry() { return m_Assets; }
		const Ref<AssetRegistry>& GetAssetRegistry() const { return m_Assets; }

		Ref<AssetRegistry>& GetEditorAssetRegistry() { return m_EditorAssets; }
		const Ref<AssetRegistry>& GetEditorAssetRegistry() const { return m_EditorAssets; }

		bool IsAssetLoaded( AssetID id, AssetRegistryType Dst = AssetRegistryType::Game );

		AssetID PathToID( const std::filesystem::path& rPath, AssetRegistryType Dst = AssetRegistryType::Game );

		void Save( AssetRegistryType Dst = AssetRegistryType::Game );

	private:
		Ref<AssetRegistry> m_Assets;
		Ref<AssetRegistry> m_EditorAssets;
	};

}