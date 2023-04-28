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
#include "AssetSerialisers.h"

#include "Saturn/Asset/AssetRegistry.h"
#include "Saturn/Asset/Prefab.h"
#include "Saturn/Audio/Sound2D.h"

#include "YamlAux.h"

#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Vulkan/Renderer.h"
#include "Saturn/ImGui/NodeEditor/NodeEditor.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	// MATERIAL

	// Serialise without node editor info
	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto& basePath = rAsset->GetPath();
		auto fullPath = Project::GetActiveProject()->FilepathAbs( basePath );

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

		std::ofstream file( fullPath );
		file << out.c_str();
	}

	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset, NodeEditor* pNodeEditor ) const
	{
		auto& basePath = rAsset->GetPath();
		auto fullPath = Project::GetActiveProject()->FilepathAbs( basePath );

		auto materialAsset = AssetRegistry::Get().GetAssetAs<MaterialAsset>( rAsset->GetAssetID() );

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

				ImVec4 colorVec4 = rNode.Color;

				out << YAML::Key << "Color" << YAML::Value << glm::vec4( colorVec4.x, colorVec4.y, colorVec4.z, 1.0f );
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

		std::ofstream file( fullPath );
		file << out.c_str();
	}

	void MaterialAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{
		auto materialAsset = rAsset.As<MaterialAsset>();

		auto absolutePath = Project::GetActiveProject()->FilepathAbs( rAsset->GetPath() );
		std::ifstream FileIn( absolutePath );

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

	void MaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset, bool LoadNodeEditorData, NodeEditor* pNodeEditor ) const
	{
		auto materialAsset = Ref<MaterialAsset>::Create( nullptr );

		auto absolutePath = Project::GetActiveProject()->FilepathAbs( rAsset->GetPath() );
		std::ifstream FileIn( absolutePath );

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

					Node* pNewNode = new Node( (int)nodeID, name.c_str(), color );
					pNewNode->State = node[ "State" ].as<std::string>();
					pNewNode->Size = ImVec2( size.x, size.y );

					for( auto input : node["Inputs"] )
					{
						if( input.IsNull() )
							continue;

						std::string name = input[ "Name" ].as<std::string>();
						size_t pinID = input[ "Input" ].as<size_t>();

						// No need to do this right now.
						//ed::NodeId nodeID = node[ "Node" ].as<ed::NodeId>();

						PinKind kind = (PinKind) input[ "Kind" ].as<int>();
						PinType type = StringToPinType( input[ "Type" ].as<std::string>() );

						Pin pin( pinID, name.c_str(), type, (int)(size_t)pNewNode->ID );
						pin.Kind = kind;
						pin.Node = pNewNode;

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

						Pin pin( pinID, name.c_str(), type, pNewNode->ID );
						pin.Kind = kind;
						pin.Node = pNewNode;

						pNewNode->Outputs.push_back( pin );
					}
				}
			}

			pNodeEditor->Reload();
		}

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

	bool MaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = Project::GetActiveProject()->FilepathAbs( rAsset->GetPath() );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto materialAsset = Ref<MaterialAsset>::Create( nullptr );

		auto materialData = data[ "Material" ];

		auto albedoColor = materialData[ "AlbedoColor" ].as<glm::vec3>();
		auto albedoPath = materialData[ "AlbedoPath" ].as<std::filesystem::path>();

		Ref<Texture2D> texture = Renderer::Get().GetPinkTexture();

		if( albedoPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( albedoPath, AddressingMode::Repeat, false );

		materialAsset->SetAlbeoColor( albedoColor );
		materialAsset->SetAlbeoMap( texture );

		auto useNormal = materialData[ "UseNormal" ].as<float>();
		auto normalPath = materialData[ "NormalPath" ].as<std::filesystem::path>();

		if( normalPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( normalPath, AddressingMode::Repeat, false );
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->UseNormalMap( useNormal );
		materialAsset->SetNormalMap( texture );

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicPath = materialData[ "MetalnessPath" ].as<std::filesystem::path>();

		if( metallicPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( metallicPath, AddressingMode::Repeat, false );
		else
			texture = Renderer::Get().GetPinkTexture();

		materialAsset->SetMetalness( metalness );
		materialAsset->SetMetallicMap( texture );

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessPath = materialData[ "RoughnessPath" ].as<std::filesystem::path>();

		if( roughnessPath != "Renderer Pink Texture" )
			texture = Ref<Texture2D>::Create( roughnessPath, AddressingMode::Repeat, false );
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

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = materialAsset;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
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
		auto fullPath = Project::GetActiveProject()->FilepathAbs( basePath );

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Prefab" << YAML::Value << prefabAsset->GetAssetID();

		out << YAML::Key << "Entities";

		out << YAML::BeginSeq;

		prefabAsset->m_Scene->GetRegistry().each( [ & ]( auto ID )
		{
			Entity e = { ID, prefabAsset->m_Scene.Pointer() };

			if( !e )
				return;

			if( e.HasComponent<SceneComponent>() )
				return;

			SerialiseEntity( out, e );
		});

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

		auto absolutePath = Project::GetActiveProject()->FilepathAbs( rAsset->GetPath() );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto entities = data[ "Entities" ];

		prefabAsset->m_Scene = Ref<Scene>::Create();

		DeserialiseEntites( entities, prefabAsset->m_Scene );

		auto view = prefabAsset->m_Scene->GetAllEntitiesWith<RelationshipComponent>();

		// Find root entity
		Entity RootEntity;

		for( auto& entity : view )
		{
			Entity ent( entity, prefabAsset->m_Scene.Pointer() );

			if( ent.GetComponent<RelationshipComponent>().Parent != 0 )
				continue;

			if( ent.GetChildren().size() > 0 )
				continue;

			RootEntity = ent;
		}

		prefabAsset->m_Entity = RootEntity;

		struct
		{
			UUID ID;
			AssetType Type;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = prefabAsset;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
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

		out << YAML::EndMap;

		out << YAML::EndMap;

		auto& basePath = rAsset->GetPath();
		auto fullPath = Project::GetActiveProject()->FilepathAbs( basePath );

		std::ofstream fout( fullPath );
		fout << out.c_str();
	}

	void StaticMeshAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{

	}

	bool StaticMeshAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = Project::GetActiveProject()->FilepathAbs( rAsset->GetPath() );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto meshData = data[ "StaticMesh" ];
		auto filepath = meshData[ "Filepath" ].as<std::string>();

		auto realMeshPath = Project::GetActiveProject()->FilepathAbs( filepath );
		auto mesh = Ref<StaticMesh>::Create( realMeshPath.string() );

		struct
		{
			UUID ID;
			AssetType Type;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = mesh;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
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
		auto fullPath = Project::GetActiveProject()->FilepathAbs( basePath );

		std::ofstream fout( fullPath );
		fout << out.c_str();
	}

	void Sound2DAssetSerialiser::Deserialise( const Ref<Asset>& rAsset ) const
	{

	}

	bool Sound2DAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = Project::GetActiveProject()->FilepathAbs( rAsset->GetPath() );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto meshData = data[ "Sound2D" ];
		auto filepath = meshData[ "Filepath" ].as<std::string>();

		auto realPath = Project::GetActiveProject()->FilepathAbs( filepath );
		auto sound = Ref<StaticMesh>::Create( realPath.string() );

		struct
		{
			UUID ID;
			AssetType Type;
			std::filesystem::path Path;
			std::string Name;
		} OldAssetData = {};

		OldAssetData.ID = rAsset->ID;
		OldAssetData.Type = rAsset->Type;
		OldAssetData.Path = rAsset->Path;
		OldAssetData.Name = rAsset->Name;

		rAsset = sound;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

}