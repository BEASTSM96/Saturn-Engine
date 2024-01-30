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
#include "AssetSerialisers.h"

#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Asset/Prefab.h"
#include "Saturn/Asset/PhysicsMaterialAsset.h"
#include "Saturn/Asset/TextureSourceAsset.h"
#include "Saturn/Audio/Sound2D.h"

#include "YamlAux.h"

#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Vulkan/Renderer.h"
#include "Saturn/ImGui/NodeEditor/NodeEditor.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Saturn {

	static std::filesystem::path GetFilepathAbs( const std::filesystem::path& rPath, bool IsEditorAsset ) 
	{
		if( IsEditorAsset )
		{
			std::filesystem::path basePath = Application::Get().GetRootContentDir();
			basePath = basePath.parent_path();
			basePath = basePath.parent_path();
			basePath /= rPath;

			return basePath;
		}
		else
		{
			return Project::GetActiveProject()->FilepathAbs( rPath );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL

	// I really hate that there is 2 functions that saves the material and there is 3 functions that loads

	// Serialise without node editor info
	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		auto materialAsset = rAsset.As<MaterialAsset>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Material" << YAML::Value;

		out << YAML::BeginMap;

		out << YAML::Key << "AlbedoColor" << YAML::Value << materialAsset->GetAlbeoColor();

		auto asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );

		if( asset )
			out << YAML::Key << "AlbedoTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "AlbedoTexture" << YAML::Value << 0;

		out << YAML::Key << "UseNormal" << YAML::Value << materialAsset->IsUsingNormalMap();

		asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		if( asset )
			out << YAML::Key << "NormalTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "NormalTexture" << YAML::Value << 0;

		out << YAML::Key << "Metalness" << YAML::Value << materialAsset->GetMetalness();

		asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		if( asset )
			out << YAML::Key << "MetalnessTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "MetalnessTexture" << YAML::Value << 0;

		out << YAML::Key << "Roughness" << YAML::Value << materialAsset->GetRoughness();

		asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		if( asset )
			out << YAML::Key << "RoughnessTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "RoughnessTexture" << YAML::Value << 0;

		out << YAML::Key << "Emissive" << YAML::Value << materialAsset->GetEmissive();

		out << YAML::EndMap;
		out << YAML::EndMap;

		std::ofstream file( fullPath );
		file << out.c_str();
	}

	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset, const Ref<NodeEditor>& pNodeEditor ) const
	{
		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		auto materialAsset = AssetManager::Get().GetAssetAs<MaterialAsset>( rAsset->GetAssetID() );

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Material" << YAML::Value;

		out << YAML::BeginMap;

		// TODO: I really hate this.

		out << YAML::Key << "AlbedoColor" << YAML::Value << materialAsset->GetAlbeoColor();

		auto asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );

		if( asset )
			out << YAML::Key << "AlbedoTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "AlbedoTexture" << YAML::Value << 0;

		out << YAML::Key << "UseNormal" << YAML::Value << materialAsset->IsUsingNormalMap();

		asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		if( asset )
			out << YAML::Key << "NormalTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "NormalTexture" << YAML::Value << 0;

		out << YAML::Key << "Metalness" << YAML::Value << materialAsset->GetMetalness();

		asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		if( asset )
			out << YAML::Key << "MetalnessTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "MetalnessTexture" << YAML::Value << 0;

		out << YAML::Key << "Roughness" << YAML::Value << materialAsset->GetRoughness();

		asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		if( asset )
			out << YAML::Key << "RoughnessTexture" << YAML::Value << asset->ID;
		else
			out << YAML::Key << "RoughnessTexture" << YAML::Value << 0;

		out << YAML::Key << "Emissive" << YAML::Value << materialAsset->GetEmissive();

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

				out << YAML::Key << "Node" << YAML::Value << ( size_t ) rNode->ID;
				out << YAML::Key << "Name" << YAML::Value << rNode->Name;

				ImVec4 colorVec4 = rNode->Color;

				out << YAML::Key << "Color" << YAML::Value << glm::vec4( colorVec4.x, colorVec4.y, colorVec4.z, 1.0f );
				out << YAML::Key << "Size" << YAML::Value << glm::vec2( rNode->Size.x, rNode->Size.y );

				out << YAML::Key << "Inputs" << YAML::Value;
				out << YAML::BeginSeq;
				for ( auto& rInput : rNode->Inputs )
				{
					out << YAML::BeginMap;

					out << YAML::Key << "Input" << YAML::Value << ( size_t )rInput->ID;
					out << YAML::Key << "Name" << YAML::Value << rInput->Name;

					out << YAML::Key << "Node" << YAML::Value << ( size_t )rInput->Node->ID;

					out << YAML::Key << "Kind" << YAML::Value << ( int )rInput->Kind;
					out << YAML::Key << "Type" << YAML::Value << PinTypeToString( rInput->Type );

					if( rInput->ExtraData.Data && rInput->ExtraData.Size > 0 )
					{
						out << YAML::Key << "ED" << YAML::Value << ( uint8_t ) rInput->ExtraData.Data;
						out << YAML::Key << "ED_Size" << YAML::Value << rInput->ExtraData.Size;
					}

					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "Outputs" << YAML::Value;
				out << YAML::BeginSeq;
				for( auto& rOutput : rNode->Outputs )
				{
					out << YAML::BeginMap;

					out << YAML::Key << "Output" << YAML::Value << ( size_t ) rOutput->ID;
					out << YAML::Key << "Name" << YAML::Value << rOutput->Name;

					out << YAML::Key << "Node" << YAML::Value << ( size_t ) rOutput->Node->ID;

					out << YAML::Key << "Kind" << YAML::Value << ( int ) rOutput->Kind;
					out << YAML::Key << "Type" << YAML::Value << PinTypeToString( rOutput->Type );

					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "State" << YAML::Value << rNode->State;

				if( rNode->ExtraData.Data && rNode->ExtraData.Size > 0 )
				{
					out << YAML::Key << "ED" << YAML::Value << (char*)rNode->ExtraData.Data;
					out << YAML::Key << "ED_Size" << YAML::Value << rNode->ExtraData.Size;
				}

				out << YAML::EndMap;
			}

			out << YAML::EndSeq;

			out << YAML::Key << "Links" << YAML::Value;

			out << YAML::BeginSeq;
			for( auto& rLink : pNodeEditor->GetLinks() )
			{
				out << YAML::BeginMap;

				out << YAML::Key << "Link" << YAML::Value << ( size_t ) rLink->ID;
				out << YAML::Key << "Color" << YAML::Value << rLink->Color;

				out << YAML::Key << "Start" << YAML::Value << ( size_t ) rLink->StartPinID;
				out << YAML::Key << "End" << YAML::Value << ( size_t ) rLink->EndPinID;

				out << YAML::EndMap;
			}

			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream file( fullPath );
		file << out.c_str();
	}

	void MaterialAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{
		auto materialAsset = rAsset.As<MaterialAsset>();

		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;

		auto materialData = data[ "Material" ];

		auto albedoColor = materialData[ "AlbedoColor" ].as<glm::vec3>();
		auto albedoID  = materialData[ "AlbedoPath" ].as<uint64_t>();

		materialAsset->SetAlbeoColor( albedoColor );

		Ref<Texture2D> defaultTexture = Renderer::Get().GetPinkTexture();
		Ref<Texture2D> texture = nullptr;

		if( albedoID != 0 ) 
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( albedoID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );
			
			materialAsset->SetAlbeoMap( texture );
		}
		else
		{
			materialAsset->SetAlbeoMap( defaultTexture );
		}

		auto useNormal = materialData[ "UseNormal" ].as<float>();
		auto normalID = materialData[ "NormalPath" ].as<uint64_t>(0);

		materialAsset->UseNormalMap( useNormal );
		
		if( normalID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( normalID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetNormalMap( texture );
		}
		else
		{
			materialAsset->SetNormalMap( defaultTexture );
		}

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicID = materialData[ "MetalnessPath" ].as<uint64_t>(0);

		materialAsset->SetMetalness( metalness );

		if( metallicID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( metallicID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetMetallicMap( texture );
		}
		else
		{
			materialAsset->SetMetallicMap( defaultTexture );
		}

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessID = materialData[ "RoughnessPath" ].as<uint64_t>(0);
		
		materialAsset->SetRoughness( val );
	
		if( roughnessID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( roughnessID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetRoughnessMap( texture );
		}
		else
		{
			materialAsset->SetRoughnessMap( defaultTexture );
		}

		auto emissive = materialData[ "Emissive" ].as<float>( 0.0f );
		materialAsset->SetEmissive( emissive );
	}

	void MaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset, bool LoadNodeEditorData, Ref<NodeEditor>& pNodeEditor ) const
	{
		auto materialAsset = Ref<MaterialAsset>::Create( nullptr );

		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;

		auto materialData = data[ "Material" ];

		auto albedoColor = materialData[ "AlbedoColor" ].as<glm::vec3>();
		auto albedoID = materialData[ "AlbedoPath" ].as<uint64_t>();

		materialAsset->SetAlbeoColor( albedoColor );

		Ref<Texture2D> defaultTexture = Renderer::Get().GetPinkTexture();
		Ref<Texture2D> texture = nullptr;

		if( albedoID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( albedoID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetAlbeoMap( texture );
		}
		else
		{
			materialAsset->SetAlbeoMap( defaultTexture );
		}

		auto useNormal = materialData[ "UseNormal" ].as<float>();
		auto normalID = materialData[ "NormalPath" ].as<uint64_t>();

		materialAsset->UseNormalMap( useNormal );

		if( normalID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( normalID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetNormalMap( texture );
		}
		else
		{
			materialAsset->SetNormalMap( defaultTexture );
		}

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicID = materialData[ "MetalnessPath" ].as<uint64_t>();

		materialAsset->SetMetalness( metalness );

		if( metallicID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( metallicID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetMetallicMap( texture );
		}
		else
		{
			materialAsset->SetMetallicMap( defaultTexture );
		}

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessID = materialData[ "RoughnessPath" ].as<uint64_t>();

		materialAsset->SetRoughness( val );

		if( roughnessID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( roughnessID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetRoughnessMap( texture );
		}
		else
		{
			materialAsset->SetRoughnessMap( defaultTexture );
		}

		auto emissive = materialData[ "Emissive" ].as<float>( 0.0f );
		materialAsset->SetEmissive( emissive );

		if( LoadNodeEditorData )
		{
			// Node Editor data
			if( auto neData = data[ "NodeEditor" ] )
			{
				auto State = neData[ "State" ].as<std::string>();

				pNodeEditor->SetEditorState( State );

				auto nodes = neData[ "Nodes" ];
				for( auto node : nodes )
				{
					if( node.IsNull() )
						continue;

					size_t nodeID = node["Node"].as<size_t>();
					std::string name = node["Name"].as<std::string>();

					glm::vec4 colorVec4 = node["Color"].as<glm::vec4>();
					ImColor color = ImColor( ImVec4( colorVec4.x, colorVec4.y, colorVec4.z, colorVec4.w ) );

					glm::vec2 size = node["Size"].as<glm::vec2>();

					Ref<Node> pNewNode = Ref<Node>::Create( (int)nodeID, name, color );
					pNewNode->State = node[ "State" ].as<std::string>();
					pNewNode->Size = ImVec2( size.x, size.y );

					for( auto input : node["Inputs"] )
					{
						if( input.IsNull() )
							continue;

						std::string name = input[ "Name" ].as<std::string>();
						int pinID = input[ "Input" ].as<int>();

						// No need to do this right now.
						//ed::NodeId nodeID = node[ "Node" ].as<ed::NodeId>();

						PinKind kind = (PinKind) input[ "Kind" ].as<int>();
						PinType type = StringToPinType( input[ "Type" ].as<std::string>() );

						Ref<Pin> pin = Ref<Pin>::Create( pinID, name, type, pNewNode->ID );
						pin->Kind = kind;
						pin->Node = pNewNode;

						auto size = input[ "ED_Size" ].as<uint32_t>( 0 );
	
						if( size > 0 )
						{
							auto data = input[ "ED" ].as<char>();

							pin->ExtraData = Buffer::Copy( &data, size );
						}

						pNewNode->Inputs.push_back( pin );
					}

					for( auto output : node[ "Outputs" ] )
					{
						if( output.IsNull() )
							continue;

						int pinID = output[ "Output" ].as<int>();
						std::string name = output[ "Name" ].as<std::string>();

						// No need to do this right now.
						//ed::NodeId nodeID = node[ "Node" ].as<ed::NodeId>();

						PinKind kind = ( PinKind ) output[ "Kind" ].as<int>();
						PinType type = StringToPinType( output[ "Type" ].as<std::string>() );

						Ref<Pin> pin = Ref<Pin>::Create( pinID, name, type, pNewNode->ID );
						pin->Kind = kind;
						pin->Node = pNewNode;

						pNewNode->Outputs.push_back( pin );
					}
				}
			}

			pNodeEditor->Reload();
		}

		// TODO: (Asset) Fix this.
		struct
		{
			UUID ID;
			AssetType Type;
			uint32_t Flags;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID   = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Flags = rAsset->Flags;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = materialAsset;
		rAsset->ID    = OldAssetData.ID;
		rAsset->Type  = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path  = OldAssetData.Path;
		rAsset->Name  = OldAssetData.Name;
	}

	bool MaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto materialAsset = Ref<MaterialAsset>::Create( nullptr );

		auto materialData = data[ "Material" ];

		auto albedoColor = materialData[ "AlbedoColor" ].as<glm::vec3>();
		auto albedoID = materialData[ "AlbedoPath" ].as<uint64_t>(0);

		materialAsset->SetAlbeoColor( albedoColor );

		Ref<Texture2D> defaultTexture = Renderer::Get().GetPinkTexture();
		Ref<Texture2D> texture = nullptr;

		if( albedoID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( albedoID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetAlbeoMap( texture );
		}
		else
		{
			materialAsset->SetAlbeoMap( defaultTexture );
		}

		auto useNormal = materialData[ "UseNormal" ].as<float>();
		auto normalID = materialData[ "NormalPath" ].as<uint64_t>(0);

		materialAsset->UseNormalMap( useNormal );

		if( normalID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( normalID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetNormalMap( texture );
		}
		else
		{
			materialAsset->SetNormalMap( defaultTexture );
		}

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicID = materialData[ "MetalnessPath" ].as<uint64_t>(0);

		materialAsset->SetMetalness( metalness );

		if( metallicID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( metallicID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetMetallicMap( texture );
		}
		else
		{
			materialAsset->SetMetallicMap( defaultTexture );
		}

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessID = materialData[ "RoughnessPath" ].as<uint64_t>(0);

		materialAsset->SetRoughness( val );

		if( roughnessID != 0 )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( roughnessID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetRoughnessMap( texture );
		}
		else
		{
			materialAsset->SetRoughnessMap( defaultTexture );
		}

		auto emissive = materialData[ "Emissive" ].as<float>( 0.0f );
		materialAsset->SetEmissive( emissive );

		// We may not always need to do this because most of the time this material will be bound meaning will change the textures.
		// However, we don't always know if it will ever be bound, for instance if we open a material in the material asset viewer, the material will not bound.
		// Meaning that the textures will not be updated.
		materialAsset->ForceUpdate();
	
		// TODO: (Asset) Fix this.
		struct
		{
			UUID ID;
			AssetType Type;
			uint32_t Flags;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Flags = rAsset->Flags;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = materialAsset;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		materialAsset->SetName( rAsset->Name );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// PREFAB

	void PrefabSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto prefabAsset = rAsset.As<Prefab>();

		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Prefab" << YAML::Value << prefabAsset->GetAssetID();

		out << YAML::Key << "Entities";

		out << YAML::BeginSeq;

		prefabAsset->m_Scene->Each( [&]( Ref<Entity> entity ) 
			{
				SerialiseEntity( out, entity );
			} );

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout( fullPath );
		fout << out.c_str();
	}

	void PrefabSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{

	}

	bool PrefabSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto prefabAsset = Ref<Prefab>::Create();

		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto entities = data[ "Entities" ];

		prefabAsset->m_Scene = Ref<Scene>::Create();
		Scene* CurrentScene = GActiveScene;

		Scene::SetActiveScene( prefabAsset->m_Scene.Get() );

		DeserialiseEntities( entities, prefabAsset->m_Scene );

		auto view = prefabAsset->m_Scene->GetAllEntitiesWith<RelationshipComponent>();

		// Find root entity
		Ref<Entity> RootEntity = nullptr;

		for( auto& entity : view )
		{
			if( entity->GetComponent<RelationshipComponent>().Parent != 0 )
				continue;

			if( entity->GetChildren().size() > 0 )
				continue;

			RootEntity = entity;
		}

		prefabAsset->m_Entity = RootEntity;
		Scene::SetActiveScene( CurrentScene );

		// TODO: (Asset) Fix this.
		struct
		{
			UUID ID;
			AssetType Type;
			uint32_t Flags;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Flags = rAsset->Flags;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = prefabAsset;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// STATIC MESH

	void StaticMeshAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto mesh = rAsset.As<StaticMesh>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "StaticMesh" << YAML::Value;

		out << YAML::BeginMap;

		out << YAML::Key << "Filepath" << YAML::Value << std::filesystem::relative( mesh->FilePath(), Project::GetActiveProject()->GetRootDir() );

		out << YAML::Key << "Attached Shape" << YAML::Value << (int)mesh->GetAttachedShape();

		out << YAML::Key << "Physics Material ID" << YAML::Value << (int)mesh->GetPhysicsMaterial();

		out << YAML::EndMap;

		out << YAML::EndMap;

		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		std::ofstream fout( fullPath );
		fout << out.c_str();
	}

	void StaticMeshAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{

	}

	bool StaticMeshAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto meshData = data[ "StaticMesh" ];
		auto filepath = meshData[ "Filepath" ].as<std::string>();
		auto shapeType = meshData[ "Attached Shape" ].as<int>( 0 );
		auto physicsMaterial = meshData[ "Physics Material ID" ].as<uint64_t>( 0 );

		auto realMeshPath = Project::GetActiveProject()->FilepathAbs( filepath );
		auto mesh = Ref<StaticMesh>::Create( realMeshPath.string() );

		mesh->SetAttachedShape( (ShapeType)shapeType );
		mesh->SetPhysicsMaterial( physicsMaterial );

		// TODO: (Asset) Fix this.
		struct
		{
			UUID ID;
			AssetType Type;
			uint32_t Flags;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Flags = rAsset->Flags;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = mesh;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SOUND2D

	void Sound2DAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto sound = rAsset.As<Sound2D>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Sound2D" << YAML::Value;

		out << YAML::BeginMap;

		out << YAML::Key << "Filepath" << YAML::Value << std::filesystem::relative( sound->GetRawPath(), Project::GetActiveProject()->GetRootDir() );

		out << YAML::EndMap;

		out << YAML::EndMap;

		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		std::ofstream fout( fullPath );
		fout << out.c_str();
	}

	void Sound2DAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{

	}

	bool Sound2DAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto soundData = data[ "Sound2D" ];
		auto filepath = soundData[ "Filepath" ].as<std::string>();

		auto realPath = Project::GetActiveProject()->FilepathAbs( filepath );
		auto sound = Ref<Sound2D>::Create();
		sound->SetRawPath( realPath );
		sound->Load();

		// TODO: (Asset) Fix this.
		struct
		{
			UUID ID;
			AssetType Type;
			uint32_t Flags;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Flags = rAsset->Flags;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = sound;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// PhysicsMaterial

	void PhysicsMaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto material = rAsset.As<PhysicsMaterialAsset>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "PhysicsMaterial" << YAML::Value;

		out << YAML::BeginMap;

		out << YAML::Key << "Static Friction" << YAML::Value << material->GetStaticFriction();

		out << YAML::Key << "Dynamic Friction" << YAML::Value << material->GetDynamicFriction();

		out << YAML::Key << "Restitution" << YAML::Value << material->GetRestitution();

		out << YAML::Key << "Flags" << YAML::Value << material->GetFlags();

		out << YAML::EndMap;

		out << YAML::EndMap;

		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		std::ofstream fout( fullPath );
		fout << out.c_str();
	}

	void PhysicsMaterialAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{
	}

	bool PhysicsMaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto materialData = data[ "PhysicsMaterial" ];

		auto staticFriction = materialData[ "Static Friction" ].as<float>();
		auto dynamicFriction = materialData[ "Dynamic Friction" ].as<float>();
		auto restitution = materialData[ "Restitution" ].as<float>();

		auto flags = materialData[ "Flags" ].as<uint32_t>();

		auto material = Ref<PhysicsMaterialAsset>::Create( staticFriction, dynamicFriction, restitution, (PhysicsMaterialFlags)flags );

		// TODO: (Asset) Fix this.
		struct
		{
			UUID ID;
			AssetType Type;
			uint32_t Flags;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Flags = rAsset->Flags;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = material;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Texture Source Asset (BINARY FILE)

	void TextureSourceAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto textureSourceAsset = rAsset.As<TextureSourceAsset>();

		// No YAML, raw serialisation.

		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		std::ofstream fout( fullPath, std::ios::binary | std::ios::trunc );
		
		const char Magic[ 6 ] = ".TSA\0";

		fout.write( Magic, 6 );

		textureSourceAsset->SerialiseData( fout );

		fout.close();
	}

	void TextureSourceAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{
	}

	bool TextureSourceAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		std::ifstream stream( fullPath, std::ios::binary | std::ios::in );

		char* Magic = nullptr;

		stream.read( reinterpret_cast<char*>( Magic ), 6 );

		if( strcmp( Magic, ".TSA\0" ) )
		{
			SAT_CORE_ERROR( "Invalid shader bundle file header!" );
			return false;
		}

		Ref<TextureSourceAsset> textureSource = Ref<TextureSourceAsset>::Create();
		textureSource->DeserialiseData( stream );
		
		// We are done with the file.
		stream.close();

		// TODO: (Asset) Fix this.
		struct
		{
			UUID ID;
			AssetType Type;
			uint32_t Flags;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Flags = rAsset->Flags;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = textureSource;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

}