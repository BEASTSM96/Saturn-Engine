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
#include "Mesh.h"

#include "VulkanContext.h"
#include "Renderer.h"
#include "DescriptorSet.h"
#include "MaterialInstance.h"

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
		aiProcess_ValidateDataStructure;

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
		
		// Get vertex and index data, also creates the vertex/index buffers.
		GetVetexAndIndexData();
		
		m_VerticesCount = m_StaticVertices.size();
		m_IndicesCount = m_Indices.size();

		// Create material.
		m_Materials.resize( m_Scene->mNumMaterials );
		for( size_t m = 0; m < m_Scene->mNumMaterials; m++ )
		{
			aiMaterial* material = m_Scene->mMaterials[ m ];

			aiString name;
			material->Get( AI_MATKEY_NAME, name );

			SAT_CORE_INFO( "Material: {0}", name.C_Str() );
			
			std::string MaterialName = std::string( name.C_Str() );

			Ref<Texture2D> PinkTexture = Renderer::Get().GetPinkTexture();

			auto mat = Ref<MaterialInstance>::Create( m_BaseMaterial, name.data );
			m_Materials[ m ] = mat;
			
			aiColor3D color;
			if( material->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS );
				mat->Set( "u_Materials.AlbedoColor", glm::vec3( color.r, color.g, color.b ) );

			SAT_CORE_INFO( " Albedo color: {0}", glm::vec3( color.r, color.g, color.g ) );

			float shininess, metalness;
			if( material->Get( AI_MATKEY_SHININESS, shininess ) == aiReturn_SUCCESS )
				shininess = 80.0f;

			if( material->Get( AI_MATKEY_REFLECTIVITY, metalness ) == aiReturn_SUCCESS )
				metalness = 0.0f;

			float roughness = 1.0f - glm::sqrt( shininess / 100.0f );
			SAT_CORE_INFO( " Roughness: {0}", roughness );

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
					
					if( std::filesystem::exists( AlbedoTexturePath ) )
						AlbedoTexture = Ref< Texture2D >::Create( AlbedoTexturePath, AddressingMode::Repeat );

					if( AlbedoTexture )
					{
						mat->SetResource( "u_AlbedoTexture", AlbedoTexture );
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

					if( std::filesystem::exists( NormalTexturePath ) )
						NormalTexture = Ref< Texture2D >::Create( NormalTexturePath, AddressingMode::Repeat );

					if( NormalTexture )
					{
						mat->SetResource( "u_NormalTexture", NormalTexture );
						mat->Set( "u_Materials.UseNormalMap", 1.0f );
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

				mat->Set( "u_Materials.Roughness", 1.0f );

				if( HasRoughnessTexture ) 
				{
					std::filesystem::path Path = filename;
					auto pp = Path.parent_path();

					pp /= std::string( RoughnessTexturePath.data );

					auto TexturePath = pp.string();

					Ref< Texture2D > RoughnessTexture;

					if( std::filesystem::exists( TexturePath ) )
						RoughnessTexture = Ref< Texture2D >::Create( TexturePath, AddressingMode::Repeat );

					if( RoughnessTexture )
					{
						mat->SetResource( "u_RoughnessTexture", RoughnessTexture );
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
							FoundMetalness = true;

							std::filesystem::path Path = filename;
							auto pp = Path.parent_path();

							pp /= String;

							auto TexturePath = pp.string();

							Ref< Texture2D > MetalnessTexture;

							if( MetalnessTexture ) 
							{
								mat->SetResource( "u_MetallicTexture", MetalnessTexture );
							}
							else
							{
								mat->SetResource( "u_MetallicTexture", PinkTexture );
							}

							mat->Set( "u_Materials.Metalness", metalness );

							break;
						}
					}
				}

				if( !FoundMetalness )
				{
					mat->SetResource( "u_MetallicTexture", PinkTexture );
				}
			}
		}
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

		m_IndexBuffer = Ref<IndexBuffer>::Create( m_Indices.data(), m_Indices.size() );
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
		std::vector<uint32_t> Indices;

		for( size_t m = 0; m < m_Scene->mNumMeshes; m++ )
		{
			aiMesh* mesh = m_Scene->mMeshes[ m ];

			Submesh& submesh = m_Submeshes.emplace_back();
			submesh.BaseVertex = m_VertexCount;
			submesh.BaseIndex = m_IndicesCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.MeshName = mesh->mName.C_Str();

			m_VertexCount += mesh->mNumVertices;
			submesh.VertexCount = m_VertexCount;
			m_IndicesCount += submesh.IndexCount;

			SAT_CORE_ASSERT( mesh->HasPositions(), "Meshes require positions." );
			SAT_CORE_ASSERT( mesh->HasNormals(), "Meshes require normals." );

			// Vertices
			for( size_t i = 0; i < mesh->mNumVertices; i++ )
			{
				MeshVertex vertex = {};
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

				Indices.push_back( mesh->mFaces[ i ].mIndices[ 0 ] );
				Indices.push_back( mesh->mFaces[ i ].mIndices[ 1 ] );
				Indices.push_back( mesh->mFaces[ i ].mIndices[ 2 ] );
			}
		}

		m_VertexBuffer = Ref<VertexBuffer>::Create( m_StaticVertices.data(), m_StaticVertices.size() * sizeof( MeshVertex ) );

		m_IndexBuffer = Ref<IndexBuffer>::Create( Indices.data(), Indices.size() );

		TraverseNodes( m_Scene->mRootNode );
	}

}