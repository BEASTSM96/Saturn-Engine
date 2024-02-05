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
#include "Saturn/Audio/Sound2D.h"

#include "Saturn/Physics/PhysicsShapeTypes.h"

#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Vulkan/Renderer.h"

#include "RawSerialisation.h"

namespace Saturn {

	void RawMaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset, std::ofstream& rStream ) const
	{
		auto materialAsset = rAsset.As<MaterialAsset>();

		if( !materialAsset )
			return;

		RawSerialisation::WriteVec3( materialAsset->GetAlbeoColor(), rStream );

		// We are fine to use the main asset registry here, we are only looking for an asset.
		auto asset = AssetManager::Get().FindAsset( materialAsset->GetAlbeoMap()->GetPath() );
		AssetID fallbackAssetID = 0;

		// ALBEO
		if( asset )
			RawSerialisation::WriteObject( asset->ID, rStream );
		else
			RawSerialisation::WriteObject( fallbackAssetID, rStream );

		// NORMAL MAP

		asset = AssetManager::Get().FindAsset( materialAsset->GetNormalMap()->GetPath() );
		bool isUsingNormalMaps = materialAsset->IsUsingNormalMap();

		RawSerialisation::WriteObject( isUsingNormalMaps, rStream );

		if( asset )
			RawSerialisation::WriteObject( asset->ID, rStream );
		else
			RawSerialisation::WriteObject( fallbackAssetID, rStream );

		// METALLIC MAP

		asset = AssetManager::Get().FindAsset( materialAsset->GetMetallicMap()->GetPath() );
		RawSerialisation::WriteObject( materialAsset->GetMetalness(), rStream );
		
		if( asset )
			RawSerialisation::WriteObject( asset->ID, rStream );
		else
			RawSerialisation::WriteObject( fallbackAssetID, rStream );

		// ROUGHNESS MAP

		asset = AssetManager::Get().FindAsset( materialAsset->GetRoughnessMap()->GetPath() );
		RawSerialisation::WriteObject( materialAsset->GetRoughness(), rStream );

		if( asset )
			RawSerialisation::WriteObject( asset->ID, rStream );
		else
			RawSerialisation::WriteObject( fallbackAssetID, rStream );

		RawSerialisation::WriteObject( materialAsset->GetEmissive(), rStream );
	}

	bool RawMaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset, std::ifstream& rStream ) const
	{
		Ref<MaterialAsset> materialAsset = Ref<MaterialAsset>::Create( nullptr );
		AssetID currentTextureID = 0;

		// ALBEO COLOR

		glm::vec3 albeoColor = glm::vec3( 0.0f );
		RawSerialisation::ReadVec3( albeoColor, rStream );

		materialAsset->SetAlbeoColor( albeoColor );

		// ALBEO MAP
		RawSerialisation::ReadObject( currentTextureID, rStream );
		
		// IS USING NORMAL MAPS
		bool isUsingNormalMaps = false;
		RawSerialisation::ReadObject( isUsingNormalMaps, rStream );

		// NORMAL MAP
		RawSerialisation::ReadObject( currentTextureID, rStream );
		
		materialAsset->UseNormalMap( isUsingNormalMaps );

		// METALNESS
		float metalness = 0.0f;

		RawSerialisation::ReadObject( metalness, rStream );

		materialAsset->SetMetalness( metalness );

		// METALLIC MAP
		RawSerialisation::ReadObject( currentTextureID, rStream );

		// ROUGHNESS
		float roughness = 0.0f;

		RawSerialisation::ReadObject( roughness, rStream );

		materialAsset->SetRoughness( roughness );

		// ROUGHNESS MAP
		RawSerialisation::ReadObject( currentTextureID, rStream );

		// EMISSIVE
		float emissive = 0.0f;
		RawSerialisation::ReadObject( emissive, rStream );
		
		materialAsset->SetEmissive( emissive );

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

	//////////////////////////////////////////////////////////////////////////
	// PREFAB

	void RawPrefabSerialiser::Serialise( const Ref<Asset>& rAsset, std::ofstream& rStream ) const
	{
		auto prefabAsset = rAsset.As<Prefab>();

		prefabAsset->GetScene()->SerialiseData( rStream );
	}

	bool RawPrefabSerialiser::TryLoadData( Ref<Asset>& rAsset, std::ifstream& rStream ) const
	{
		auto prefabAsset = Ref<Prefab>::Create();
		prefabAsset->Create();

		prefabAsset->GetScene()->DeserialiseData( rStream );

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

	void RawStaticMeshAssetSerialiser::Serialise( const Ref<Asset>& rAsset, std::ofstream& rStream ) const
	{
		auto staticMeshAsset = rAsset.As<StaticMesh>();

		RawSerialisation::WriteObject( staticMeshAsset->GetAttachedShape(), rStream );
		RawSerialisation::WriteObject( staticMeshAsset->GetPhysicsMaterial(), rStream );

		staticMeshAsset->SerialiseData( rStream );
	}

	bool RawStaticMeshAssetSerialiser::TryLoadData( Ref<Asset>& rAsset, std::ifstream& rStream ) const
	{
		auto staticMeshAsset = Ref<StaticMesh>::Create();

		ShapeType shapeType = ShapeType::Unknown;
		AssetID physicsMaterial = 0;

		RawSerialisation::ReadObject( shapeType, rStream );
		RawSerialisation::ReadObject( physicsMaterial, rStream );

		staticMeshAsset->SetAttachedShape( shapeType );
		staticMeshAsset->SetPhysicsMaterial( physicsMaterial );

		staticMeshAsset->DeserialiseData( rStream );

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

	void RawPhysicsMaterialAssetSerialiser::Serialise( const Ref<Asset>& rAsset, std::ofstream& rStream ) const
	{
		auto physMaterialAsset = rAsset.As<PhysicsMaterialAsset>();

		RawSerialisation::WriteObject( physMaterialAsset->GetStaticFriction(), rStream );
		RawSerialisation::WriteObject( physMaterialAsset->GetDynamicFriction(), rStream );
		RawSerialisation::WriteObject( physMaterialAsset->GetRestitution(), rStream );
		
		RawSerialisation::WriteObject( physMaterialAsset->GetFlags(), rStream );
	}

	bool RawPhysicsMaterialAssetSerialiser::TryLoadData( Ref<Asset>& rAsset, std::ifstream& rStream ) const
	{
		glm::vec3 StaticDynamicFrictionRestitution{};
		uint32_t assetFlags = 0;

		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.x, rStream );
		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.y, rStream );
		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.z, rStream );

		RawSerialisation::ReadObject( assetFlags, rStream );

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

	//////////////////////////////////////////////////////////////////////////
	// TEXTURE SOURCE

	void RawTextureSourceAssetSerialiser::Serialise( const Ref<Asset>& rAsset, std::ofstream& rStream ) const
	{
		auto textureSourceAsset = rAsset.As<TextureSourceAsset>();

		textureSourceAsset->SerialiseData( rStream );
	}

	bool RawTextureSourceAssetSerialiser::TryLoadData( Ref<Asset>& rAsset, std::ifstream& rStream ) const
	{
		auto textureSourceAsset = Ref<TextureSourceAsset>::Create();

		textureSourceAsset->DeserialiseData( rStream );

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

		rAsset = textureSourceAsset;
		rAsset->ID = OldAssetData.ID;
		rAsset->Type = OldAssetData.Type;
		rAsset->Flags = OldAssetData.Flags;
		rAsset->Path = OldAssetData.Path;
		rAsset->Name = OldAssetData.Name;

		return false;
	}

}