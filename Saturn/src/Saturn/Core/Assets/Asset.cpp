#include "sppch.h"
#include "Asset.h"

#include "AssetCollection.h"

namespace Saturn {

	Ref<Saturn::Asset> Asset::Load( std::string path, std::string RootDir, AssetCollection* assetCollection )
	{

	}

	Ref<Asset> Asset::GetParent() const
	{
		if (m_Path == "/")
			return nullptr;

		SAT_CORE_ASSERT(m_AssetCollection, "Asset is not owned by a AssetCollector");

		auto lastSlash = m_Path.find_first_of('/');
		return m_AssetCollection->GetAsset(m_Path.substr(0, lastSlash +1 ));

	}

	Saturn::FolderAsset* Asset::GetFolderAsset()
	{
	
		if (m_Type != AssetType::Folder)
		{
			return nullptr;
		}

		return (FolderAsset*)this;
	}

}