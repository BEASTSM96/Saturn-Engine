#pragma once


#include "Asset.h"
#include "Saturn/Core/UUID.h"
#include <yaml-cpp/yaml.h>

namespace Saturn {

	class FolderAsset : public Asset
	{
	public:
		FolderAsset() = default;
		FolderAsset( YAML::Node yamlnode, std::string RootDir );
		FolderAsset( std::string path, UUID uuid );
		FolderAsset( std::string name, std::string path, UUID uuid );

		auto& GetAssets() { return m_Assets; }
		const auto& GetAssets() const { return m_Assets; }

	protected:
	private:
		std::vector<Ref<Asset>> m_Assets;
	};

}