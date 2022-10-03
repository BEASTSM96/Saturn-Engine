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

#include "sppch.h"
#include "AssetSerialisers.h"

#include "YamlAux.h"

#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Vulkan/Renderer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL

	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto materialAsset = rAsset.As<MaterialAsset>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Material" << YAML::Value;

		out << YAML::BeginMap;

		out << YAML::Key << "AlbedoColor" << YAML::Value << materialAsset->GetAlbeoColor();
		out << YAML::Key << "AlbedoPath" << YAML::Value << materialAsset->GetAlbeoMap()->GetPath();

		out << YAML::Key << "UseNormal" << YAML::Value << materialAsset->IsUsingNormalMap();
		out << YAML::Key << "NormalPath" << YAML::Value << materialAsset->GetNormalMap()->GetPath();

		out << YAML::Key << "Metalness" << YAML::Value << materialAsset->GetMetalness();
		out << YAML::Key << "MetalnessPath" << YAML::Value << materialAsset->GetMetallicMap()->GetPath();

		out << YAML::Key << "Roughness" << YAML::Value << materialAsset->GetRoughness();
		out << YAML::Key << "RoughnessPath" << YAML::Value << materialAsset->GetRoughnessMap()->GetPath();

		out << YAML::EndMap;
		out << YAML::EndMap;

		std::ofstream file( rAsset->GetPath() );
		file << out.c_str();
	}

	void MaterialAssetSerialiser::Derialise( const Ref<Asset>& rAsset ) const
	{
		auto materialAsset = rAsset.As<MaterialAsset>();

		std::ifstream FileIn( rAsset->GetPath() );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;

		auto materialData = data[ "Material" ];

		auto albedoColor = materialData[ "AlbedoColor" ].as<glm::vec3>();
		auto albedoPath  = materialData[ "AlbedoPath" ].as<std::filesystem::path>();

		Ref<Texture2D> texture = Renderer::Get().GetPinkTexture();

		if( albedoPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( albedoPath, AddressingMode::Repeat );

		materialAsset->SetAlbeoColor( albedoColor );
		materialAsset->SetAlbeoMap( texture );

		auto useNormal = materialData[ "UseNormal" ].as<float>();
		auto normalPath = materialData[ "NormalPath" ].as<std::filesystem::path>();

		if( normalPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( normalPath, AddressingMode::Repeat );

		materialAsset->UseNormalMap( useNormal );
		materialAsset->SetNormalMap( texture );

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicPath = materialData[ "MetalnessPath" ].as<std::filesystem::path>();

		if( metallicPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( metallicPath, AddressingMode::Repeat );

		materialAsset->SetMetalness( metalness);
		materialAsset->SetMetallicMap( texture );

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessPath = materialData[ "RoughnessPath" ].as<std::filesystem::path>();

		if( roughnessPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( roughnessPath, AddressingMode::Repeat );

		materialAsset->SetRoughness( val );
		materialAsset->SetRoughnessMap( texture );
	}

}