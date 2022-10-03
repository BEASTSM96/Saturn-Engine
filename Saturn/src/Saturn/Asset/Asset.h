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

#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Ref.h"
#include "Saturn/Core/Memory/Buffer.h"

#include <filesystem>

namespace Saturn {

	using AssetID = UUID;

	enum class AssetType
	{
		Texture,
		StaticMesh,
		SkeletalMesh,
		Material,
		MaterialInstance,
		Audio,
		Scene,
		Prefab,
		Script,
		Unknown,
		COUNT,
	};

	inline std::string AssetTypeToString( AssetType type )
	{
		switch( type )
		{
			case Saturn::AssetType::Texture:
				return "Texture";
			case Saturn::AssetType::StaticMesh:
				return "StaticMesh";
			case Saturn::AssetType::SkeletalMesh:
				return "SkeletalMesh";
			case Saturn::AssetType::Material:
				return "Material";
			case Saturn::AssetType::MaterialInstance:
				return "MaterialInstance";
			case Saturn::AssetType::Audio:
				return "Audio";
			case Saturn::AssetType::Scene:
				return "Scene";
			case Saturn::AssetType::Prefab:
				return "Prefab";
			case Saturn::AssetType::Script:
				return "Script";
			case Saturn::AssetType::Unknown:
				return "Unknown";
			default:
				break;
		}
	}

	inline AssetType AssetTypeFromString( const std::string& str )
	{
		if( str == "Texture" )
			return AssetType::Texture;
		else if( str == "StaticMesh" )
			return AssetType::StaticMesh;
		else if( str == "SkeletalMesh" )
			return AssetType::SkeletalMesh;
		else if( str == "Material" )
			return AssetType::Material;
		else if( str == "MaterialInstance" )
			return AssetType::MaterialInstance;
		else if( str == "Audio" )
			return AssetType::Audio;
		else if( str == "Scene" )
			return AssetType::Scene;
		else if( str == "Prefab" )
			return AssetType::Prefab;
		else if( str == "Script" )
			return AssetType::Script;
		else
			return AssetType::Unknown;
	}

	inline AssetType AssetTypeFromExtension( const std::string& str )
	{
		if( str == ".png" || str == ".tga" || str == ".jpg" || str == ".jpeg" )
			return AssetType::Texture;
		else if( str == ".fbx" || str == ".gltf" )
			return AssetType::StaticMesh;
		else if( str == ".mp3" || str == ".wav" || str == ".ogg" )
			return AssetType::Audio;
		else if( str == ".scene" )
			return AssetType::Scene;
		else if( str == ".smaterial" )
			return AssetType::Material;
		else
			return AssetType::Unknown;
	}

	struct AssetData
	{
		Buffer DataBuffer;
	};

	class Asset : public CountedObj
	{
	public:
		Asset() {}

		void SetAssetType( AssetType type ) { if( m_AssetType != AssetType::Unknown ) m_AssetType = type; }

		AssetType GetAssetType() { return m_AssetType; }
		AssetID GetAssetID() { return m_ID; }

		const AssetType GetAssetType() const { return m_AssetType; }
		const AssetID& GetAssetID() const { return m_ID; }
		
		void SetPath( std::filesystem::path p ) { m_Path = p; }

		void AssignAssetData( AssetData& rData ) { m_AssetData = std::move( rData ); }

		AssetData& GetAssetData() { return m_AssetData; }
		const AssetData& GetAssetData() const { return m_AssetData; }
		
		std::filesystem::path& GetPath() { return m_Path; }
		const std::filesystem::path& GetPath() const { return m_Path; }

	protected:
		AssetID m_ID;
		AssetType m_AssetType;

		AssetData m_AssetData;

		std::filesystem::path m_Path;
	};
}