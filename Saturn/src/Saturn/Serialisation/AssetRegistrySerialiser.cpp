/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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

#include "sppch.h"
#include "AssetRegistrySerialiser.h"
#include "Saturn/Asset/AssetRegistry.h"

#include "Saturn/Project/Project.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace YAML {

	template <>
	struct convert<std::filesystem::path>
	{
		static Node encode( std::filesystem::path rhs )
		{
			return Node( rhs.string() );
		}

		static bool decode( const Node& node, std::filesystem::path& rhs )
		{
			rhs = node.as<std::string>();

			return true;
		}
	};

	inline Emitter& operator<<( Emitter& emitter, const std::filesystem::path& v )
	{
		return emitter.Write( v.string() );
	}
}

namespace Saturn {

	void AssetRegistrySerialiser::Serialise( const Ref<AssetRegistry>& rAssetRegistry )
	{
		YAML::Emitter out;

		auto& Assets = rAssetRegistry->GetAssetMap();

		out << YAML::BeginMap;

		out << YAML::Key << "Assets";

		out << YAML::BeginSeq;

		for( const auto& [id, asset] : Assets )
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Asset" << YAML::Value << id;

			out << YAML::Key << "Path" << YAML::Value << asset->GetPath();

			out << YAML::Key << "Type" << YAML::Value << AssetTypeToString( asset->GetAssetType() );

			out << YAML::Key << "Version" << YAML::Value << asset->Version;

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream stream( rAssetRegistry->GetPath() );
		stream << out.c_str();
	}

	void AssetRegistrySerialiser::Deserialise( Ref<AssetRegistry> AssetRegistry )
	{
		std::ifstream FileIn( AssetRegistry->GetPath() );
		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		auto assets = data[ "Assets" ];

		if( assets.IsNull() )
			return;

		for( auto asset : assets )
		{
			UUID assetID = asset[ "Asset" ].as< uint64_t >();

			auto path = asset[ "Path" ].as< std::filesystem::path >();
			auto type = asset[ "Type" ].as< std::string >();

			// Fallback to newest version if no version is present.
			auto version = asset[ "Version" ].as< uint32_t >( SAT_CURRENT_VERISON );

			AssetRegistry->AddAsset( assetID );

			Ref<Asset> DeserialisedAsset = AssetRegistry->FindAsset( assetID );

			DeserialisedAsset->Path = path;
			DeserialisedAsset->Name = path.filename().replace_extension().string();
			DeserialisedAsset->Type = AssetTypeFromString( type );
			DeserialisedAsset->Version = version;

			if( version != SAT_CURRENT_VERISON )
			{
				std::string versionString = SAT_CURRENT_VERISON_STRING;

				std::string assetVersionString;
				SAT_DECODE_VER_STRING( version, assetVersionString );

				SAT_CORE_WARN( "Asset \"{0}\" was created in a different version (asset version was {1}) Saturn version is {2}", DeserialisedAsset->Name, assetVersionString, versionString );
			}

			AssetRegistry->m_IsEditorRegistry ? DeserialisedAsset->Flags = (uint32_t)AssetFlag::Editor : DeserialisedAsset->Flags = ( uint32_t ) AssetFlag::None;
		}
	}

}