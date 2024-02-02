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
		auto fallbackAssetID = 0;

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
		auto staticMeshAsset = rAsset.As<StaticMesh>();

		ShapeType shapeType = ShapeType::Unknown;
		AssetID physicsMaterial = 0;

		RawSerialisation::ReadObject( shapeType, rStream );
		RawSerialisation::ReadObject( physicsMaterial, rStream );

		staticMeshAsset->SetAttachedShape( shapeType );
		staticMeshAsset->SetPhysicsMaterial( physicsMaterial );

		staticMeshAsset->DeserialiseData( rStream );

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
		auto physMaterialAsset = rAsset.As<PhysicsMaterialAsset>();

		glm::vec3 StaticDynamicFrictionRestitution{};
		uint32_t assetFlags = 0;

		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.x, rStream );
		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.y, rStream );
		RawSerialisation::ReadObject( StaticDynamicFrictionRestitution.z, rStream );

		RawSerialisation::ReadObject( assetFlags, rStream );

		physMaterialAsset->SetStaticFriction( StaticDynamicFrictionRestitution.x );
		physMaterialAsset->SetDynamicFriction( StaticDynamicFrictionRestitution.y );
		physMaterialAsset->SetRestitution( StaticDynamicFrictionRestitution.z );

		physMaterialAsset->SetFlag( (PhysicsMaterialFlags)assetFlags, true );

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
		return false;
	}

}