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
#include "RawAssetSerialisers.h"

#include "Saturn/Asset/AssetManager.h"
#include "Saturn/Asset/Prefab.h"
#include "Saturn/Asset/PhysicsMaterialAsset.h"
#include "Saturn/Asset/TextureSourceAsset.h"
#include "Saturn/Audio/Sound.h"

#include "Saturn/Core/VirtualFS.h"

#include "Saturn/Physics/PhysicsShapeTypes.h"

#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Vulkan/Renderer.h"

#include "RawSerialisation.h"
#include "Saturn/Core/MemoryStream.h"

// #NOTE
// NOTES WHEN ADDING NEW FIELDS TO SERIALISE:
// First, make sure to add the field in both Raw and YAML serialisers
// Next, add it to TryLoadData.
// Finally, for VFS Assets, make sure to load it and write into the VFile!

namespace Saturn {

	bool RawMaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, rAsset->Path );

		if( !file )
			return false;

		PakFileMemoryBuffer membuf( file->FileContent );

		std::istream stream( &membuf );

		/////////////////////////////////////

		Ref<MaterialAsset> materialAsset = Ref<MaterialAsset>::Create( nullptr );
		AssetID currentTextureID = 0;

		// ALBEO COLOR

		glm::vec3 albeoColor = glm::vec3( 0.0f );
		RawSerialisation::ReadVec3( albeoColor, stream );

		materialAsset->SetAlbeoColor( albeoColor );

		// ALBEO MAP
		RawSerialisation::ReadObject( currentTextureID, stream );
		
		materialAsset->SetAlbeoMap( currentTextureID );

		// IS USING NORMAL MAPS
		bool isUsingNormalMaps = false;
		RawSerialisation::ReadObject( isUsingNormalMaps, stream );
		materialAsset->UseNormalMap( isUsingNormalMaps );

		// NORMAL MAP
		RawSerialisation::ReadObject( currentTextureID, stream );
		
		materialAsset->SetNormalMap( currentTextureID );

		// METALNESS
		float metalness = 0.0f;

		RawSerialisation::ReadObject( metalness, stream );

		materialAsset->SetMetalness( metalness );

		// METALLIC MAP
		RawSerialisation::ReadObject( currentTextureID, stream );
		materialAsset->SetMetallicMap( currentTextureID );

		// ROUGHNESS
		float roughness = 0.0f;

		RawSerialisation::ReadObject( roughness, stream );

		materialAsset->SetRoughness( roughness );

		// ROUGHNESS MAP
		RawSerialisation::ReadObject( currentTextureID, stream );
		materialAsset->SetRoughnessMap( currentTextureID );

		// EMISSIVE
		float emissive = 0.0f;
		RawSerialisation::ReadObject( emissive, stream );
		
		materialAsset->SetEmissive( emissive );

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

		return true;
	}

	bool RawMaterialAssetSerialiser::DumpAndWriteToVFS( const Ref<Asset>& rAsset ) const
	{
		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, rAsset->Path );

		if( !file )
			return false;

		auto materialAsset = rAsset.As<MaterialAsset>();

		if( !materialAsset )
			return false;

		std::filesystem::path out = Project::GetActiveProject()->GetTempDir();
		out /= std::to_string( rAsset->ID );
		out.replace_extension( ".vfs" );

		std::ofstream ss( out, std::ios::binary | std::ios::trunc );

		/////////////////////////////////////

		RawSerialisation::WriteVec3( materialAsset->GetAlbeoColor(), ss );

		// We are fine to use the main asset registry here, we are only looking for an asset.
		auto asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		AssetID fallbackAssetID = 0;

		// ALBEO
		if( asset )
			RawSerialisation::WriteObject( asset->ID, ss );
		else
			RawSerialisation::WriteObject( fallbackAssetID, ss );

		// NORMAL MAP

		asset = AssetManager::Get().FindAsset( materialAsset->GetNormalMap()->GetPath() );
		bool isUsingNormalMaps = materialAsset->IsUsingNormalMap();

		RawSerialisation::WriteObject( isUsingNormalMaps, ss );

		if( asset )
			RawSerialisation::WriteObject( asset->ID, ss );
		else
			RawSerialisation::WriteObject( fallbackAssetID, ss );

		// METALLIC MAP

		asset = AssetManager::Get().FindAsset( materialAsset->GetMetallicMap()->GetPath() );
		RawSerialisation::WriteObject( materialAsset->GetMetalness(), ss );

		if( asset )
			RawSerialisation::WriteObject( asset->ID, ss );
		else
			RawSerialisation::WriteObject( fallbackAssetID, ss );

		// ROUGHNESS MAP

		asset = AssetManager::Get().FindAsset( materialAsset->GetRoughnessMap()->GetPath() );
		RawSerialisation::WriteObject( materialAsset->GetRoughness(), ss );

		if( asset )
			RawSerialisation::WriteObject( asset->ID, ss );
		else
			RawSerialisation::WriteObject( fallbackAssetID, ss );

		RawSerialisation::WriteObject( materialAsset->GetEmissive(), ss );
		
		ss.close();

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// PREFAB

	bool RawPrefabSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto prefabAsset = Ref<Prefab>::Create();
		prefabAsset->Create();

		//prefabAsset->DeserialisePrefab();

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

	bool RawPrefabSerialiser::DumpAndWriteToVFS( const Ref<Asset>& rAsset ) const
	{
		auto prefabAsset = rAsset.As<Prefab>();

		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, rAsset->Path );

		if( !file )
			return false;

		std::filesystem::path out = Project::GetActiveProject()->GetTempDir();
		out /= std::to_string( rAsset->ID );
		out.replace_extension( ".vfs" );

		std::ofstream fout( out, std::ios::binary | std::ios::trunc );

		/////////////////////////////////////

		prefabAsset->SerialisePrefab( fout );
		
		fout.close();

		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// STATIC MESH

	bool RawStaticMeshAssetSerialiser::DumpAndWriteToVFS( const Ref<Asset>& rAsset ) const
	{
		auto staticMeshAsset = rAsset.As<StaticMesh>();

		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, rAsset->Path );

		if( !file )
			return false;

		std::filesystem::path out = Project::GetActiveProject()->GetTempDir();
		out /= std::to_string( rAsset->ID );
		out.replace_extension( ".vfs" );

		std::ofstream fout( out, std::ios::binary | std::ios::trunc );

		/////////////////////////////////////

		RawSerialisation::WriteObject( staticMeshAsset->GetAttachedShape(), fout );
		RawSerialisation::WriteObject( staticMeshAsset->GetPhysicsMaterial(), fout );

		staticMeshAsset->SerialiseData( fout );

		fout.close();

		return true;
	}

	bool RawStaticMeshAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto staticMeshAsset = Ref<StaticMesh>::Create();

		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, rAsset->Path );

		if( !file )
			return false;

		PakFileMemoryBuffer membuf( file->FileContent );
		std::istream stream( &membuf );

		/////////////////////////////////////

		ShapeType shapeType = ShapeType::Unknown;
		AssetID physicsMaterial = 0;

		RawSerialisation::ReadObject( shapeType, stream );
		RawSerialisation::ReadObject( physicsMaterial, stream );

		staticMeshAsset->SetAttachedShape( shapeType );
		staticMeshAsset->SetPhysicsMaterial( physicsMaterial );

		staticMeshAsset->DeserialiseData( stream );

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

		rAsset = staticMeshAsset;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// PHYSICS MATERIAL

	bool RawPhysicsMaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, rAsset->Path );

		if( !file )
			return false;

		/////////////////////////////////////
		PakFileMemoryBuffer membuf( file->FileContent );
		std::istream stream( &membuf );

		glm::vec3 StaticDynamicFrictionRestitution{};
		uint32_t assetFlags = 0;

		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.x, stream );
		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.y, stream );
		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.z, stream );

		RawSerialisation::ReadObject( assetFlags, stream );

		auto physMaterialAsset = Ref<PhysicsMaterialAsset>::Create( 
			StaticDynamicFrictionRestitution.x,
			StaticDynamicFrictionRestitution.y,
			StaticDynamicFrictionRestitution.z );
		
		physMaterialAsset->SetFlag( (PhysicsMaterialFlags)assetFlags, true );

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

		rAsset = physMaterialAsset;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

	bool RawPhysicsMaterialAssetSerialiser::DumpAndWriteToVFS( const Ref<Asset>& rAsset ) const
	{
		auto physMaterialAsset = rAsset.As<PhysicsMaterialAsset>();

		std::filesystem::path out = Project::GetActiveProject()->GetTempDir();
		out /= std::to_string( rAsset->ID );
		out.replace_extension( ".vfs" );

		std::ofstream stream( out, std::ios::binary | std::ios::trunc );

		RawSerialisation::WriteObject( physMaterialAsset->GetStaticFriction(), stream );
		RawSerialisation::WriteObject( physMaterialAsset->GetDynamicFriction(), stream );
		RawSerialisation::WriteObject( physMaterialAsset->GetRestitution(), stream );

		RawSerialisation::WriteObject( physMaterialAsset->GetFlags(), stream );

		stream.close();

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SOUND SPECIFICATION

	bool RawSoundSpecAssetSerialiser::DumpAndWriteToVFS( const Ref<Asset>& rAsset ) const
	{
		auto sound = rAsset.As<SoundSpecification>();

		std::filesystem::path out = Project::GetActiveProject()->GetTempDir();
		out /= std::to_string( rAsset->ID );
		out.replace_extension( ".vfs" );

		std::ofstream stream( out, std::ios::binary | std::ios::trunc );

		RawSerialisation::WriteString( std::filesystem::relative( sound->SoundSourcePath, Project::GetActiveProject()->GetRootDir() ), stream );
		
		// No need for this in dist
		//RawSerialisation::WriteString( sound->OriginalImportPath, stream );

		stream.close();

		return true;
	}

	bool RawSoundSpecAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		const std::string& rMountBase = Project::GetActiveConfig().Name;
		Ref<VFile> file = VirtualFS::Get().FindFile( rMountBase, rAsset->Path );

		if( !file )
			return false;

		/////////////////////////////////////

		PakFileMemoryBuffer membuf( file->FileContent );
		std::istream stream( &membuf );

		std::string sourcePath = RawSerialisation::ReadString( stream );

		auto soundSpec = Ref<SoundSpecification>::Create();
		soundSpec->SoundSourcePath = sourcePath;

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

		rAsset = soundSpec;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// TEXTURE SOURCE

	bool RawTextureSourceAssetSerialiser::TryLoadData( Ref<Asset>& rAsset ) const
	{
		auto textureSourceAsset = Ref<TextureSourceAsset>::Create();
		textureSourceAsset->Path = rAsset->Path;
		textureSourceAsset->ID = rAsset->ID;
		textureSourceAsset->Name = rAsset->Name;
		textureSourceAsset->Flags = rAsset->Flags;
		textureSourceAsset->Type = rAsset->Type;

		textureSourceAsset->ReadFromVFS();

		rAsset = textureSourceAsset;

		return true;
	}

	bool RawTextureSourceAssetSerialiser::DumpAndWriteToVFS( const Ref<Asset>& rAsset ) const
	{
		return false;
	}

}