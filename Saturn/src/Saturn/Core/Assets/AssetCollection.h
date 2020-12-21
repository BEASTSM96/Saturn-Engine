#pragma once

#include "Asset.h"
#include "FolderAsset.h"
#include "Saturn/Core/UUID.h"

#include <unordered_set>

namespace Saturn {

	class AssetCollection
	{
	public:
		AssetCollection();

		void InitFromAssetsDirectory();
		void AddAsset( const Ref<Asset> asset, bool CreateFolders = false );

		Ref<Asset> GetAsset( std::string path );
		Ref<Asset> GetAsset( const UUID& uuid );

		bool AssetExists( std::string path );

	protected:
	private:
		Ref<FolderAsset> m_RootAsset;
		std::unordered_set<Ref<Asset>> m_AllAssets;
	};
}
