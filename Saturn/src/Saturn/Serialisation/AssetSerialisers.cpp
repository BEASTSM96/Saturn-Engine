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
#include "Saturn/ImGui/NodeEditor/NodeEditor.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL

	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto basePath = rAsset->GetPath();

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

	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset, NodeEditor* pNodeEditor ) const
	{
		auto basePath = rAsset->GetPath();

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

		if( pNodeEditor )
		{
			out << YAML::Key << "NodeEditor" << YAML::Value;
			out << YAML::BeginMap;

			out << YAML::Key << "State" << YAML::Value << pNodeEditor->GetEditorState();

			out << YAML::Key << "Nodes" << YAML::Value;

			out << YAML::BeginSeq;

			for( auto& rNode : pNodeEditor->GetNodes() )
			{
				out << YAML::BeginMap;

				out << YAML::Key << "Node" << YAML::Value << ( size_t ) rNode.ID;
				out << YAML::Key << "Name" << YAML::Value << rNode.Name;
				out << YAML::Key << "Color" << YAML::Value << rNode.Color;
				out << YAML::Key << "Size" << YAML::Value << glm::vec2( rNode.Size.x, rNode.Size.y );

				out << YAML::Key << "Inputs" << YAML::Value;
				out << YAML::BeginSeq;
				for ( auto& rInput : rNode.Inputs )
				{
					out << YAML::BeginMap;

					out << YAML::Key << "Input" << YAML::Value << ( size_t )rInput.ID;
					out << YAML::Key << "Name" << YAML::Value << rInput.Name;

					out << YAML::Key << "Node" << YAML::Value << ( size_t )rInput.Node->ID;

					out << YAML::Key << "Kind" << YAML::Value << ( int )rInput.Kind;
					out << YAML::Key << "Type" << YAML::Value << PinTypeToString( rInput.Type );

					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "Outputs" << YAML::Value;
				out << YAML::BeginSeq;
				for( auto& rOutput : rNode.Outputs )
				{
					out << YAML::BeginMap;

					out << YAML::Key << "Output" << YAML::Value << ( size_t ) rOutput.ID;
					out << YAML::Key << "Name" << YAML::Value << rOutput.Name;

					out << YAML::Key << "Node" << YAML::Value << ( size_t ) rOutput.Node->ID;

					out << YAML::Key << "Kind" << YAML::Value << ( int ) rOutput.Kind;
					out << YAML::Key << "Type" << YAML::Value << PinTypeToString( rOutput.Type );

					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "State" << YAML::Value << rNode.State;

				if( rNode.ExtraData.Data && rNode.ExtraData.Size > 0 )
				{
					out << YAML::Key << "ED" << YAML::Value << (char*)rNode.ExtraData.Data;
					out << YAML::Key << "ED_Size" << YAML::Value << rNode.ExtraData.Size;
				}

				out << YAML::EndMap;
			}

			out << YAML::EndSeq;

			out << YAML::Key << "Links" << YAML::Value;

			out << YAML::BeginSeq;
			for( auto& rLink : pNodeEditor->GetLinks() )
			{
				out << YAML::BeginMap;

				out << YAML::Key << "Link" << YAML::Value << ( size_t ) rLink.ID;
				out << YAML::Key << "Color" << YAML::Value << rLink.Color;

				out << YAML::Key << "Start" << YAML::Value << ( size_t ) rLink.StartPinID;
				out << YAML::Key << "End" << YAML::Value << ( size_t ) rLink.EndPinID;

				out << YAML::EndMap;
			}

			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

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
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->UseNormalMap( useNormal );
		materialAsset->SetNormalMap( texture );

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicPath = materialData[ "MetalnessPath" ].as<std::filesystem::path>();

		if( metallicPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( metallicPath, AddressingMode::Repeat );
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->SetMetalness( metalness);
		materialAsset->SetMetallicMap( texture );

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessPath = materialData[ "RoughnessPath" ].as<std::filesystem::path>();

		if( roughnessPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( roughnessPath, AddressingMode::Repeat );
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->SetRoughness( val );
		materialAsset->SetRoughnessMap( texture );
	}

	void MaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto materialAsset = Ref<MaterialAsset>::Create( nullptr );

		std::ifstream FileIn( rAsset->GetPath() );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;

		auto materialData = data[ "Material" ];

		auto albedoColor = materialData[ "AlbedoColor" ].as<glm::vec3>();
		auto albedoPath = materialData[ "AlbedoPath" ].as<std::filesystem::path>();

		Ref<Texture2D> texture = Renderer::Get().GetPinkTexture();

		if( albedoPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( albedoPath, AddressingMode::Repeat );

		materialAsset->SetAlbeoColor( albedoColor );
		materialAsset->SetAlbeoMap( texture );

		auto useNormal = materialData[ "UseNormal" ].as<float>();
		auto normalPath = materialData[ "NormalPath" ].as<std::filesystem::path>();

		if( normalPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( normalPath, AddressingMode::Repeat );
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->UseNormalMap( useNormal );
		materialAsset->SetNormalMap( texture );

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicPath = materialData[ "MetalnessPath" ].as<std::filesystem::path>();

		if( metallicPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( metallicPath, AddressingMode::Repeat );
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->SetMetalness( metalness );
		materialAsset->SetMetallicMap( texture );

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessPath = materialData[ "RoughnessPath" ].as<std::filesystem::path>();

		if( roughnessPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( roughnessPath, AddressingMode::Repeat );
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->SetRoughness( val );
		materialAsset->SetRoughnessMap( texture );

		struct
		{
			UUID ID;
			AssetType Type;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID   = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = materialAsset;
		rAsset->ID   = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

	}

}