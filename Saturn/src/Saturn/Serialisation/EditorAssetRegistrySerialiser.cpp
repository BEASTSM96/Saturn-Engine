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

#include "sppch.h"
#include "EditorAssetRegistrySerialiser.h"

#include "Saturn/Asset/EditorAssetRegistry.h"

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

	void EditorAssetRegistrySerialiser::Serialise()
	{
		YAML::Emitter out;

		auto& Assets = EditorAssetRegistry::Get().GetAssetMap();

		out << YAML::BeginMap;

		out << YAML::Key << "Assets";

		out << YAML::BeginSeq;

		for( const auto& [id, asset] : Assets )
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Asset" << YAML::Value << id;

			out << YAML::Key << "Path" << YAML::Value << asset->GetPath();

			out << YAML::Key << "Type" << YAML::Value << AssetTypeToString( asset->GetAssetType() );

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;

		// The working dir will be the our exe path.
		// However what about in Dist builds we might not want to store the editor assets in the binary folder.
		std::ofstream stream( "content/AssetRegistry.sreg" );
		stream << out.c_str();
	}

	void EditorAssetRegistrySerialiser::Deserialise()
	{
		std::ifstream FileIn( "content/AssetRegistry.sreg" );
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

			EditorAssetRegistry::Get().AddAsset( assetID );

			Ref<Asset> DeserialisedAsset = EditorAssetRegistry::Get().FindAsset( assetID );

			DeserialisedAsset->Path = path;
			DeserialisedAsset->Name = path.filename().replace_extension().string();
			DeserialisedAsset->Type = AssetTypeFromString( type );
		}
	}

}