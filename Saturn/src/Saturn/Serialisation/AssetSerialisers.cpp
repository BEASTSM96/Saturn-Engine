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
#include "Saturn/Audio/Sound.h"

#include "YamlAux.h"

#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Vulkan/Renderer.h"
#include "Saturn/NodeEditor/UI/NodeEditor.h"

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

	void MaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		//auto materialAsset = AssetManager::Get().GetAssetAs<MaterialAsset>( rAsset->GetAssetID() );

		auto materialAsset = rAsset.As<MaterialAsset>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Material" << YAML::Value;

		out << YAML::BeginMap;

		out << YAML::Key << "AlbedoColor" << YAML::Value << materialAsset->GetAlbeoColor();

		auto writeTexture = [&](const char* key, Ref<Texture2D> texture ) 
		{
			std::filesystem::path relativePath = std::filesystem::relative( texture->GetPath(), Project::GetActiveProjectRootPath() );
			auto asset = AssetManager::Get().FindAsset( relativePath );

			if( asset )
				out << YAML::Key << key << YAML::Value << asset->ID;
			else
				out << YAML::Key << key << YAML::Value << 0;
		};

		writeTexture( "AlbedoTexture", materialAsset->GetAlbeoMap() );

		out << YAML::Key << "UseNormal" << YAML::Value << materialAsset->IsUsingNormalMap();

		writeTexture( "NormalTexture", materialAsset->GetNormalMap() );

		out << YAML::Key << "Metalness" << YAML::Value << materialAsset->GetMetalness();

		writeTexture( "MetalnessTexture", materialAsset->GetMetallicMap() );

		out << YAML::Key << "Roughness" << YAML::Value << materialAsset->GetRoughness();

		writeTexture( "RoughnessTexture", materialAsset->GetRoughnessMap() );

		out << YAML::Key << "Emissive" << YAML::Value << materialAsset->GetEmissive();

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream file( fullPath );
		file << out.c_str();
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
		auto albedoID = materialData[ "AlbedoTexture" ].as<uint64_t>( 0 );

		materialAsset->SetAlbeoColor( albedoColor );

		Ref<Texture2D> texture = nullptr;

		if( AssetManager::Get().DoesAssetIDExist( albedoID ) )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( albedoID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetAlbeoMap( texture );
		}

		auto useNormal = materialData[ "UseNormal" ].as<float>();
		auto normalID = materialData[ "NormalTexture" ].as<uint64_t>( 0 );

		materialAsset->UseNormalMap( useNormal );

		if( AssetManager::Get().DoesAssetIDExist( normalID ) )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( normalID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetNormalMap( texture );
		}

		auto metalness = materialData[ "Metalness" ].as<float>();
		auto metallicID = materialData[ "MetalnessTexture" ].as<uint64_t>( 0 );

		materialAsset->SetMetalness( metalness );

		if( AssetManager::Get().DoesAssetIDExist( metallicID ) )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( metallicID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetMetallicMap( texture );
		}
		

		auto val = materialData[ "Roughness" ].as<float>();
		auto roughnessID = materialData[ "RoughnessTexture" ].as<uint64_t>( 0 );

		materialAsset->SetRoughness( val );

		if( AssetManager::Get().DoesAssetIDExist( roughnessID ) )
		{
			Ref<Asset> rAsset = AssetManager::Get().FindAsset( roughnessID );
			texture = Ref<Texture2D>::Create( Project::GetActiveProject()->FilepathAbs( rAsset->Path ), AddressingMode::Repeat );

			materialAsset->SetRoughnessMap( texture );
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
	// SOUND

	void SoundAssetSerialiser::Serialise( const Ref<Asset>& rAsset ) const
	{
		auto sound = rAsset.As<Sound>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Sound" << YAML::Value;

		out << YAML::BeginMap;

		out << YAML::Key << "Filepath" << YAML::Value << std::filesystem::relative( sound->GetRawPath(), Project::GetActiveProject()->GetRootDir() );

		out << YAML::EndMap;

		out << YAML::EndMap;

		auto& basePath = rAsset->GetPath();
		auto fullPath = GetFilepathAbs( basePath, rAsset->IsFlagSet( AssetFlag::Editor ) );

		std::ofstream fout( fullPath );
		fout << out.c_str();
	}

	bool SoundAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto absolutePath = GetFilepathAbs( rAsset->GetPath(), rAsset->IsFlagSet( AssetFlag::Editor ) );
		std::ifstream FileIn( absolutePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return false;

		auto soundData = data[ "Sound" ];
		auto filepath = soundData[ "Filepath" ].as<std::string>();

		auto realPath = Project::GetActiveProject()->FilepathAbs( filepath );

		auto sound = Ref<Sound>::Create();
		sound->SetRawPath( realPath );

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
}