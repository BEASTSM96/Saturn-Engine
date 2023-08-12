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

#pragma once

#include "Saturn/Core/App.h"

#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Ref.h"
#include "Saturn/Core/Memory/Buffer.h"

#include "Saturn/Project/Project.h"

#include <filesystem>

namespace Saturn {

	using AssetID = UUID;

	enum class AssetType
	{
		Texture,
		StaticMesh,
		SkeletalMesh,
		Material,
		MaterialInstance,
		Audio,
		Scene,
		Prefab,
		Script,
		MeshCollider,
		PhysicsMaterial,
		Unknown,
		COUNT,
	};

	enum class AssetFlag : uint32_t
	{
		None = BIT( 0 ),
		Editor = BIT( 1 ),
		COUNT
	};

	inline std::string AssetTypeToString( AssetType type )
	{
		switch( type )
		{
			case Saturn::AssetType::Texture:
				return "Texture";
			case Saturn::AssetType::StaticMesh:
				return "StaticMesh";
			case Saturn::AssetType::SkeletalMesh:
				return "SkeletalMesh";
			case Saturn::AssetType::Material:
				return "Material";
			case Saturn::AssetType::MaterialInstance:
				return "MaterialInstance";
			case Saturn::AssetType::Audio:
				return "Audio";
			case Saturn::AssetType::Scene:
				return "Scene";
			case Saturn::AssetType::Prefab:
				return "Prefab";
			case Saturn::AssetType::Script:
				return "Script";
			case Saturn::AssetType::MeshCollider:
				return "MeshCollider";
			case Saturn::AssetType::PhysicsMaterial:
				return "PhysicsMaterial";
			case Saturn::AssetType::Unknown:
				return "Unknown";
			default:
				break;
		}

		return "";
	}

// I don't want to include the imgui header so we will just copy the macro.
#define R_SHIFT 0
#define G_SHIFT 8
#define B_SHIFT 16
#define A_SHIFT 24
#define A_MASK  0xFF000000
#define COLOR_32( R, G, B, A )  (((uint32_t)(A)<<A_SHIFT) | ((uint32_t)(B)<<B_SHIFT) | ((uint32_t)(G)<<G_SHIFT) | ((uint32_t)(R)<<R_SHIFT))

	inline uint32_t AssetTypeToColor( AssetType type )
	{
		switch( type )
		{
			case Saturn::AssetType::Texture:
				return COLOR_32( 160, 118, 249, 255 );
			case Saturn::AssetType::StaticMesh:
				return COLOR_32( 161, 103, 11, 255 );
			case Saturn::AssetType::SkeletalMesh:
				return COLOR_32( 161, 103, 11, 255 );
			case Saturn::AssetType::Material:
				return COLOR_32( 237, 5, 229, 255 );
			case Saturn::AssetType::MaterialInstance:
				return COLOR_32( 237, 5, 229, 255 );
			case Saturn::AssetType::Audio:
				return COLOR_32( 237, 202, 5, 255 );
			case Saturn::AssetType::Scene:
				return COLOR_32( 255, 0, 0, 255 );
			case Saturn::AssetType::Prefab:
				return COLOR_32( 255, 0, 255, 255 );
			case Saturn::AssetType::Script:
				return COLOR_32( 5, 183, 237, 255 );
			case Saturn::AssetType::MeshCollider:
				return COLOR_32( 255, 255, 255, 255 );
			case Saturn::AssetType::PhysicsMaterial:
				return COLOR_32( 235, 0, 55, 255 );
			case Saturn::AssetType::Unknown:
				return COLOR_32( 255, 255, 255, 255 );
			default:
				break;
		}

		return 0;
	}

	inline AssetType AssetTypeFromString( const std::string& str )
	{
		if( str == "Texture" )
			return AssetType::Texture;
		else if( str == "StaticMesh" )
			return AssetType::StaticMesh;
		else if( str == "SkeletalMesh" )
			return AssetType::SkeletalMesh;
		else if( str == "Material" )
			return AssetType::Material;
		else if( str == "MaterialInstance" )
			return AssetType::MaterialInstance;
		else if( str == "Audio" )
			return AssetType::Audio;
		else if( str == "Scene" )
			return AssetType::Scene;
		else if( str == "Prefab" )
			return AssetType::Prefab;
		else if( str == "Script" )
			return AssetType::Script;
		else if( str == "MeshCollider" )
			return AssetType::MeshCollider;
		else if( str == "PhysicsMaterial" )
			return AssetType::PhysicsMaterial;
		else
			return AssetType::Unknown;
	}

	inline AssetType AssetTypeFromExtension( const std::string& str )
	{
		if( str == ".png" || str == ".tga" || str == ".jpg" || str == ".jpeg" || str == ".hdr" || str == ".ico" )
			return AssetType::Texture;
		else if( str == ".s2d" )
			return AssetType::Audio;
		else if( str == ".scene" )
			return AssetType::Scene;
		else if( str == ".smaterial" )
			return AssetType::Material;
		else if( str == ".cpp" || str == ".h" )
			return AssetType::Script;
		else if( str == ".prefab" )
			return AssetType::Prefab;
		else if( str == ".stmesh" )
			return AssetType::StaticMesh;
		else if( str == ".sphymaterial" )
			return AssetType::PhysicsMaterial;
		else
			return AssetType::Unknown;
	}

	struct AssetData
	{
		Buffer DataBuffer;
	};

	class Asset : public RefTarget
	{
	public:
		AssetID ID = 0;
		AssetType Type = AssetType::Unknown;
		uint32_t Flags = 0;

		std::filesystem::path Path;
		std::string Name;

	public:
		Asset() {}

		const AssetType GetAssetType() const { return Type; }
		const AssetID& GetAssetID() const { return ID; }

		const std::filesystem::path& GetPath() const { return Path; }
		const std::string& GetName() const { return Name; }

		bool IsFlagSet( AssetFlag flag ) const { return ( Flags & ( uint32_t ) flag ) != 0; }
		uint32_t GetFlags() const { return Flags; }

		void SetFlags( AssetFlag flag, bool value ) 
		{
			if( value )
				Flags |= ( uint32_t ) flag;
			else
				Flags &= ~( uint32_t ) flag;
		}

		// TODO: This is bad.
		//       I want to copy this just so I can get the name without the extension.
		// Note:
		//      p must be an absolute path.
		void SetPath( std::filesystem::path p )
		{
			std::filesystem::path base = "";
			
			// TODO: [GetRootContentDir] Change GetRootContentDir to return the actual path and not the path with the asset registry.
			if( IsFlagSet( AssetFlag::Editor ) )
				base = Application::Get().GetRootContentDir().parent_path().parent_path();
			else
				base = Project::GetActiveProject()->GetRootDir();

			Path = std::filesystem::relative( p, base );

			auto CopyPath = Path;
			Name = CopyPath.replace_extension().filename().string();
		}

	private:
		friend class AssetRegistrySerialiser;
		friend class AssetRegistry;
	};
}