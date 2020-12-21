#pragma once

#include "Saturn/Core/Base.h"

namespace Saturn {

	enum class AssetType
	{
		None = 0, Folder
	};

	class FolderAsset;
	class AssetCollection;

	class Asset
	{
	public:
		Asset() = default;
		virtual ~Asset() = default;

		static Ref<Asset> Load(std::string path, std::string RootDir, AssetCollection* assetCollection);

		Ref<Asset> GetParent() const;

		std::string& GetName() { return m_Name; }
		std::string& GetPath() { return m_Path; }

		const std::string& GetName() const { return m_Name; }
		const std::string& GetPath() const { return m_Path; }

		UUID& GetUUID() { return m_UUID; }
		const UUID& GetUUID() const { return m_UUID; }
		void SetUUID();

		bool IsFolder() const { m_Type == AssetType::Folder; }

		FolderAsset* GetFolderAsset();
	protected:
		AssetType m_Type = AssetType::None;

		std::string m_Name;
		std::string m_Path;
		UUID m_UUID;

		AssetCollection* m_AssetCollection = nullptr;

		friend class AssetCollection;

	};
}