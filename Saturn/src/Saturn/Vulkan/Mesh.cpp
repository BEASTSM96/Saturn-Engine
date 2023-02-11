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
#include "Mesh.h"

#include "VulkanContext.h"
#include "Renderer.h"
#include "DescriptorSet.h"
#include "MaterialInstance.h"

#include "Saturn/Serialisation/AssetRegistrySerialiser.h"
#include "Saturn/Serialisation/AssetSerialisers.h"

#include "Saturn/Asset/AssetRegistry.h"
#include "Saturn/Asset/Asset.h"
#include "Saturn/Asset/MaterialAsset.h"
#include "Saturn/Asset/AssetImporter.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>


#include <filesystem>

namespace Saturn {

#if defined( SAT_DEBUG )
	std::thread s_DebugThread;
#endif

	glm::mat4 Mat4FromAssimpMat4( const aiMatrix4x4& matrix )
	{
		glm::mat4 result;
		result[ 0 ][ 0 ] = matrix.a1; result[ 1 ][ 0 ] = matrix.a2; result[ 2 ][ 0 ] = matrix.a3; result[ 3 ][ 0 ] = matrix.a4;
		result[ 0 ][ 1 ] = matrix.b1; result[ 1 ][ 1 ] = matrix.b2; result[ 2 ][ 1 ] = matrix.b3; result[ 3 ][ 1 ] = matrix.b4;
		result[ 0 ][ 2 ] = matrix.c1; result[ 1 ][ 2 ] = matrix.c2; result[ 2 ][ 2 ] = matrix.c3; result[ 3 ][ 2 ] = matrix.c4;
		result[ 0 ][ 3 ] = matrix.d1; result[ 1 ][ 3 ] = matrix.d2; result[ 2 ][ 3 ] = matrix.d3; result[ 3 ][ 3 ] = matrix.d4;
		return result;
	}

	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_JoinIdenticalVertices |
		aiProcess_ValidateDataStructure;    // Validation
		//aiProcess_GlobalScale |             // e.g. convert cm to m for fbx import (and other formats where cm is native)

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if( Assimp::DefaultLogger::isNullLogger() )
			{
				Assimp::DefaultLogger::create( "", Assimp::Logger::VERBOSE );
				Assimp::DefaultLogger::get()->attachStream( new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn );
			}
		}

		virtual void write( const char* message ) override
		{
			SAT_CORE_WARN( "Assimp error: {0}", message );
		}
	};
	
	Mesh::Mesh( const std::string& filename ) : m_FilePath( filename )
	{
		LogStream::Initialize();

		SAT_CORE_INFO( "Loading mesh: {0}", filename.c_str() );

		if( !std::filesystem::exists( m_FilePath ) )
			SAT_CORE_ERROR( "Failed to load mesh file: {0}", filename );

		m_Importer = std::make_unique<Assimp::Importer>();

		const aiScene* scene = m_Importer->ReadFile( filename, s_MeshImportFlags );
		if( scene == nullptr || !scene->HasMeshes() )
			SAT_CORE_ERROR( "Failed to load mesh file: {0}", filename );

		m_Scene = scene;
		m_MeshShader = ShaderLibrary::Get().Find( "shader_new" );
		m_BaseMaterial = Ref< Material >::Create( m_MeshShader, "Base Material");
		
		m_InverseTransform = glm::inverse( Mat4FromAssimpMat4( m_Scene->mRootNode->mTransformation ) );
		m_Transform		   = Mat4FromAssimpMat4( m_Scene->mRootNode->mTransformation );
		
		// Get vertex and index data, also creates the vertex/index buffers.
		GetVetexAndIndexData();
		
		m_VerticesCount = m_StaticVertices.size();
		m_IndicesCount = m_Indices.size();

		// Create material.
		m_Materials.resize( m_Scene->mNumMaterials );
		m_MaterialsAssets.resize( m_Scene->mNumMaterials );

		for( size_t m = 0; m < m_Scene->mNumMaterials; m++ )
		{
			aiMaterial* material = m_Scene->mMaterials[ m ];

			aiString name;
			material->Get( AI_MATKEY_NAME, name );

			SAT_CORE_INFO( "Material: {0}", name.C_Str() );
			
			std::string MaterialName = std::string( name.C_Str() );

			Ref<Texture2D> PinkTexture = Renderer::Get().GetPinkTexture();

			auto assetPath = std::filesystem::path( m_FilePath ).parent_path();

			assetPath /= name.data;
			assetPath += ".smaterial";

			auto mat = Ref<MaterialInstance>::Create( m_BaseMaterial, name.data );
			m_Materials[ m ] = mat;

			Ref<Asset> realAsset = AssetRegistry::Get().FindAsset( assetPath );

			Ref<MaterialAsset> asset;

			if( realAsset == nullptr )
			{
				// Create a new material asset.
				realAsset = AssetRegistry::Get().FindAsset( AssetRegistry::Get().CreateAsset( AssetTypeFromExtension( assetPath.extension().string() ) ) );

				realAsset->SetPath( assetPath );

				// Try load
				asset = AssetRegistry::Get().GetAssetAs<MaterialAsset>( realAsset->GetAssetID() );

				if( asset == nullptr ) 
				{
					asset = Ref<MaterialAsset>::Create( nullptr );
				}

				// Write to disk, create file.
				struct
				{
					UUID ID;
					AssetType Type;
					std::filesystem::path Path;
					std::string Name;
				} OldAssetData = {};

				OldAssetData.ID   = realAsset->ID;
				OldAssetData.Type = realAsset->Type;
				OldAssetData.Path = realAsset->Path;
				OldAssetData.Name = realAsset->Name;

				realAsset = asset;
				realAsset->ID = OldAssetData.ID;
				realAsset->Type = OldAssetData.Type;
				realAsset->Path = OldAssetData.Path;
				realAsset->Name = OldAssetData.Name;

				MaterialAssetSerialiser mas;
				mas.Serialise( realAsset );

				asset->SetName( MaterialName );

				m_MaterialsAssets[ m ] = asset;
			}
			else
			{
				asset = AssetRegistry::Get().GetAssetAs<MaterialAsset>( realAsset->GetAssetID() );
				m_MaterialsAssets[ m ] = asset;

				continue;
			}

			aiColor3D color;
			if( material->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS );
				mat->Set( "u_Materials.AlbedoColor", glm::vec3( color.r, color.g, color.b ) );

			asset->SetAlbeoColor( glm::vec3( color.r, color.g, color.b ) );

			SAT_CORE_INFO( " Albedo color: {0}", glm::vec3( color.r, color.g, color.b ) );

			float shininess, metalness;
			if( material->Get( AI_MATKEY_SHININESS, shininess ) != aiReturn_SUCCESS )
				shininess = 80.0f;

			if( material->Get( AI_MATKEY_REFLECTIVITY, metalness ) != aiReturn_SUCCESS )
				metalness = 0.0f;

			float roughness = 1.0f - glm::sqrt( shininess / 100.0f );
			SAT_CORE_INFO( " Roughness: {0}", roughness );
			SAT_CORE_INFO( " shininess: {0}", shininess );
			SAT_CORE_INFO( " metalness: {0}", metalness );

			// Albedo Texture
			{
				aiString AlbedoTexturePath;
				bool HasAlbedoTexture = material->GetTexture( aiTextureType_DIFFUSE, 0, &AlbedoTexturePath ) == AI_SUCCESS;

				if( HasAlbedoTexture )
				{
					std::filesystem::path AlbedoPath = filename;
					auto pp = AlbedoPath.parent_path();

					pp /= std::string( AlbedoTexturePath.data );

					auto AlbedoTexturePath = pp.string();
					
					Ref< Texture2D > AlbedoTexture;

					SAT_CORE_INFO( " Albedo Map texture {0}", AlbedoTexturePath );
					
					auto localTexturePath = assetPath.parent_path();

					localTexturePath /= pp.filename();

					if( !std::filesystem::exists( localTexturePath ) )
						std::filesystem::copy_file( AlbedoTexturePath, localTexturePath );

					if( std::filesystem::exists( localTexturePath ) )
							AlbedoTexture = Ref< Texture2D >::Create( localTexturePath, AddressingMode::Repeat, false );

					if( AlbedoTexture )
					{
						mat->SetResource( "u_AlbedoTexture", AlbedoTexture );
						asset->SetAlbeoMap( AlbedoTexture );
					}
					else
					{
						mat->SetResource( "u_AlbedoTexture", PinkTexture );
					}
				}
				else
				{
					mat->SetResource( "u_AlbedoTexture", PinkTexture );
				}
			}

			// Normal Texture
			{
				aiString NormalTexturePath;
				bool HasNormalTexture = material->GetTexture( aiTextureType_NORMALS, 0, &NormalTexturePath ) == AI_SUCCESS;

				if( HasNormalTexture )
				{
					std::filesystem::path Path = filename;
					auto pp = Path.parent_path();

					pp /= std::string( NormalTexturePath.data );

					auto NormalTexturePath = pp.string();

					Ref< Texture2D > NormalTexture;

					SAT_CORE_INFO( " Normal Map texture {0}", NormalTexturePath );

					auto localTexturePath = assetPath.parent_path();

					localTexturePath /= pp.filename();

					if( !std::filesystem::exists( localTexturePath ) )
						std::filesystem::copy_file( NormalTexturePath, localTexturePath );

					if( std::filesystem::exists( localTexturePath ) )
						NormalTexture = Ref< Texture2D >::Create( localTexturePath, AddressingMode::Repeat, false );

					if( NormalTexture )
					{
						mat->SetResource( "u_NormalTexture", NormalTexture );
						mat->Set( "u_Materials.UseNormalMap", 1.0f );

						asset->SetNormalMap( NormalTexture );
						asset->UseNormalMap( 1.0f );
					}
					else
					{
						mat->SetResource( "u_NormalTexture", PinkTexture );
						mat->Set( "u_Materials.UseNormalMap", 0.0f );
					}
				}
				else
				{
					mat->SetResource( "u_NormalTexture", PinkTexture );
					mat->Set( "u_Materials.UseNormalMap", 0.0f );
				}
			}

			// Roughness texture
			{
				aiString RoughnessTexturePath;
				bool HasRoughnessTexture = material->GetTexture( aiTextureType_SHININESS, 0, &RoughnessTexturePath ) == AI_SUCCESS;

				mat->Set( "u_Materials.Roughness", roughness );

				asset->SetRoughness( roughness );

				if( HasRoughnessTexture ) 
				{
					std::filesystem::path Path = filename;
					auto pp = Path.parent_path();

					pp /= std::string( RoughnessTexturePath.data );

					auto TexturePath = pp.string();

					Ref< Texture2D > RoughnessTexture;

					auto localTexturePath = assetPath.parent_path();

					localTexturePath /= pp.filename();

					if( !std::filesystem::exists( localTexturePath ) )
						std::filesystem::copy_file( TexturePath, localTexturePath );

					if( std::filesystem::exists( localTexturePath ) )
						RoughnessTexture = Ref< Texture2D >::Create( localTexturePath, AddressingMode::Repeat, false );

					if( RoughnessTexture )
					{
						mat->SetResource( "u_RoughnessTexture", RoughnessTexture );
						asset->SetRoughnessMap( RoughnessTexture );
					}
					else
					{
						mat->SetResource( "u_RoughnessTexture", PinkTexture );
					}
				}
				else
				{
					mat->SetResource( "u_RoughnessTexture", PinkTexture );
				}
			}

			// Metalness
			{
				bool FoundMetalness = false;
				
				for( uint32_t i = 0; i < material->mNumProperties; i++ )
				{
					auto prop = material->mProperties[ i ];

					if( prop->mType == aiPTI_String )
					{
						uint32_t StringLen = *( uint32_t* ) prop->mData;
						std::string String( prop->mData + 4, StringLen );

						std::string Key = prop->mKey.data;
						if( Key == "$raw.ReflectionFactor|file" )
						{
							std::filesystem::path Path = filename;
							auto pp = Path.parent_path();

							pp /= String;

							auto TexturePath = pp.string();

							Ref< Texture2D > MetalnessTexture;

							auto localTexturePath = assetPath.parent_path();

							localTexturePath /= pp.filename();

							if( !std::filesystem::exists( localTexturePath ) )
								std::filesystem::copy_file( TexturePath, localTexturePath );

							if( std::filesystem::exists( localTexturePath ) )
								MetalnessTexture = Ref< Texture2D >::Create( localTexturePath, AddressingMode::Repeat, false );

							if( MetalnessTexture ) 
							{
								FoundMetalness = true;
								mat->SetResource( "u_MetallicTexture", MetalnessTexture );
								asset->SetMetallicMap( MetalnessTexture );
							}
							else
							{
								mat->SetResource( "u_MetallicTexture", PinkTexture );
							}

							mat->Set( "u_Materials.Metalness", 1.0f );
							asset->SetMetalness( 1.0f );

							break;
						}
					}
				}

				if( !FoundMetalness )
				{
					mat->SetResource( "u_MetallicTexture", PinkTexture );
					mat->Set( "u_Materials.Metalness", metalness );
					asset->SetMetalness( metalness );
				}

				MaterialAssetSerialiser mas;
				mas.Serialise( asset, nullptr );
			}
		}

		AssetRegistrySerialiser ars;
		ars.Serialise();
	}

	Mesh::Mesh( const std::vector<MeshVertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform ) : m_StaticVertices( vertices ), m_Indices( indices )
	{
		Submesh submesh;
		submesh.BaseVertex = 0;
		submesh.BaseIndex = 0;
		submesh.IndexCount = indices.size() * 3;
		submesh.Transform = transform;
		m_Submeshes.push_back( submesh );

		m_VertexBuffer = Ref<VertexBuffer>::Create( m_StaticVertices.data(), m_StaticVertices.size() * sizeof( MeshVertex ) );

		m_IndexBuffer = Ref<IndexBuffer>::Create( m_Indices.data(), m_Indices.size() * sizeof( Index ) );
	}

	Mesh::~Mesh()
	{
		if( m_BaseMaterial )
			m_BaseMaterial = nullptr;

		m_VertexBuffer = nullptr;
		m_IndexBuffer = nullptr;

		m_Indices.clear();
		m_StaticVertices.clear();
	}

	void Mesh::TraverseNodes( aiNode* node, const glm::mat4& parentTransform, uint32_t level ) 
	{
		glm::mat4 transform = parentTransform * Mat4FromAssimpMat4( node->mTransformation );

		for( uint32_t i = 0; i < node->mNumMeshes; i++ )
		{
			uint32_t mesh = node->mMeshes[ i ];
			auto& submesh = m_Submeshes[ mesh ];
			submesh.NodeName = node->mName.C_Str();
			submesh.Transform = transform;

			// Create a descriptor set for each submesh to use.
			// Set 0 = material data and vertex data.
			m_DescriptorSets[ submesh ] = m_MeshShader->CreateDescriptorSet( 0 );
		}

		for( uint32_t i = 0; i < node->mNumChildren; i++ )
			TraverseNodes( node->mChildren[ i ], transform, level + 1 );
	}

	void Mesh::RefreshDescriptorSets()
	{
		SAT_CORE_ASSERT( false, "Not added!" );
	}

	void Mesh::GetVetexAndIndexData()
	{
		m_Submeshes.reserve( m_Scene->mNumMeshes );

		for( unsigned m = 0; m < m_Scene->mNumMeshes; m++ )
		{
			aiMesh* mesh = m_Scene->mMeshes[ m ];

			Submesh& submesh = m_Submeshes.emplace_back();
			submesh.BaseVertex = m_VertexCount;
			submesh.BaseIndex = m_IndicesCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.VertexCount = mesh->mNumVertices;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.MeshName = mesh->mName.C_Str();

			m_VertexCount += mesh->mNumVertices;
			m_IndicesCount += submesh.IndexCount;

			SAT_CORE_ASSERT( mesh->HasPositions(), "Meshes require positions." );
			SAT_CORE_ASSERT( mesh->HasNormals(), "Meshes require normals." );

			// Vertices
			for( size_t i = 0; i < mesh->mNumVertices; i++ )
			{
				MeshVertex vertex;
				vertex.Position = { mesh->mVertices[ i ].x, mesh->mVertices[ i ].y, mesh->mVertices[ i ].z };
				vertex.Normal = { mesh->mNormals[ i ].x, mesh->mNormals[ i ].y, mesh->mNormals[ i ].z };

				if( mesh->HasTangentsAndBitangents() )
				{
					vertex.Tangent = { mesh->mTangents[ i ].x, mesh->mTangents[ i ].y, mesh->mTangents[ i ].z };
					vertex.Binormal = { mesh->mBitangents[ i ].x, mesh->mBitangents[ i ].y, mesh->mBitangents[ i ].z };
				}

				if( mesh->HasTextureCoords( 0 ) )
					vertex.Texcoord = { mesh->mTextureCoords[ 0 ][ i ].x, mesh->mTextureCoords[ 0 ][ i ].y };

				m_StaticVertices.push_back( vertex );
			}

			// Indices
			for( size_t i = 0; i < mesh->mNumFaces; i++ )
			{
				SAT_CORE_ASSERT( mesh->mFaces[ i ].mNumIndices == 3, "Mesh must have 3 indices." );

				m_Indices.push_back( { mesh->mFaces[ i ].mIndices[ 0 ], mesh->mFaces[ i ].mIndices[ 1 ], mesh->mFaces[ i ].mIndices[ 2 ] } );
			}
		}

		m_VertexBuffer = Ref<VertexBuffer>::Create( m_StaticVertices.data(), (uint32_t)(m_StaticVertices.size() * sizeof( MeshVertex ) ) );

		m_IndexBuffer = Ref<IndexBuffer>::Create( m_Indices.data(), m_Indices.size() * sizeof( Index ) );

		TraverseNodes( m_Scene->mRootNode );
	}

	void Mesh::CopyTextures()
	{

	}

	//////////////////////////////////////////////////////////////////////////

	MeshSource::MeshSource( const std::filesystem::path& rPath, const std::filesystem::path& rDstPath )
	{
		LogStream::Initialize();

		m_Importer = std::make_unique<Assimp::Importer>();

		const aiScene* scene = m_Importer->ReadFile( rPath.string(), s_MeshImportFlags );

		m_Scene = scene;
		
		for( size_t m = 0; m < m_Scene->mNumMaterials; m++ )
		{
			aiMaterial* material = m_Scene->mMaterials[ m ];

			aiString name;
			material->Get( AI_MATKEY_NAME, name );

			std::string MaterialName = std::string( name.C_Str() );

			// Albedo Texture
			{
				aiString AlbedoTexturePath;
				bool HasAlbedoTexture = material->GetTexture( aiTextureType_DIFFUSE, 0, &AlbedoTexturePath ) == AI_SUCCESS;

				if( HasAlbedoTexture )
				{
					auto pp = rPath.parent_path();

					pp /= std::string( AlbedoTexturePath.data );

					auto AlbedoTexturePath = pp.string();

					auto LocalPath = rDstPath;

					LocalPath /= pp.filename();

					if( !std::filesystem::exists( LocalPath ) )
						std::filesystem::copy_file( AlbedoTexturePath, LocalPath );
				}
			}

			// Normal Texture
			{
				aiString TexturePath;
				bool HasTexture = material->GetTexture( aiTextureType_NORMALS, 0, &TexturePath ) == AI_SUCCESS;

				if( HasTexture )
				{
					auto pp = rPath.parent_path();

					pp /= std::string( TexturePath.data );

					auto NormalTexturePath = pp.string();

					auto LocalPath = rDstPath;

					LocalPath /= pp.filename();

					if( !std::filesystem::exists( LocalPath ) )
						std::filesystem::copy_file( NormalTexturePath, LocalPath );
				}
			}

			// Normal Texture
			{
				aiString TexturePath;
				bool HasTexture = material->GetTexture( aiTextureType_SHININESS, 0, &TexturePath ) == AI_SUCCESS;

				if( HasTexture )
				{
					auto pp = rPath.parent_path();

					pp /= std::string( TexturePath.data );

					auto RoughnessTexturePath = pp.string();

					auto LocalPath = rDstPath;

					LocalPath /= pp.filename();

					if( !std::filesystem::exists( LocalPath ) )
						std::filesystem::copy_file( RoughnessTexturePath, LocalPath );
				}
			}

			// Metalness
			{
				bool FoundMetalness = false;

				for( uint32_t i = 0; i < material->mNumProperties; i++ )
				{
					auto prop = material->mProperties[ i ];

					if( prop->mType == aiPTI_String )
					{
						uint32_t StringLen = *( uint32_t* ) prop->mData;
						std::string String( prop->mData + 4, StringLen );

						std::string Key = prop->mKey.data;
						if( Key == "$raw.ReflectionFactor|file" )
						{
							auto pp = rPath.parent_path();

							pp /= String;

							auto TexturePath = pp.string();

							Ref< Texture2D > MetalnessTexture;

							auto localTexturePath = rDstPath;

							localTexturePath /= pp.filename();

							if( !std::filesystem::exists( localTexturePath ) )
								std::filesystem::copy_file( TexturePath, localTexturePath );

							break;
						}
					}
				}
			}
		}
	}

	MeshSource::~MeshSource()
	{

	}

	void MeshSource::TraverseNodes( aiNode* node, const glm::mat4& parentTransform /*= glm::mat4( 1.0f )*/, uint32_t level /*= 0 */ )
	{

	}

}