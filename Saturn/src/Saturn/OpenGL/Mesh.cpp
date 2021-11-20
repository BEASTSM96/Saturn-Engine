/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "Renderer.h"

#include "xGL.h"

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

	void Mesh::DebugWork()
	{
	}

	Mesh::Mesh( const std::string& filename ) : m_FilePath( filename )
	{
		LogStream::Initialize();

		SAT_CORE_INFO( "Loading mesh: {0}", filename.c_str() );

		m_Importer = std::make_unique<Assimp::Importer>();

		const aiScene* scene = m_Importer->ReadFile( filename, s_MeshImportFlags );
		if( !scene || !scene->HasMeshes() )
			SAT_CORE_ERROR( "Failed to load mesh file: {0}", filename );

		m_Scene = scene;
		m_MeshShader = Ref<Shader>::Create( "assets\\shaders\\ShaderBasic.glsl" );
		m_InverseTransform = glm::inverse( Mat4FromAssimpMat4( scene->mRootNode->mTransformation ) );
		
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		m_Submeshes.reserve( scene->mNumMeshes );
		for( size_t m = 0; m < scene->mNumMeshes; m++ )
		{
			aiMesh* mesh = scene->mMeshes[ m ];

			Submesh& submesh = m_Submeshes.emplace_back();
			submesh.BaseVertex = vertexCount;
			submesh.BaseIndex = indexCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.MeshName = mesh->mName.C_Str();

			vertexCount += mesh->mNumVertices;
			submesh.VertexCount = vertexCount;
			indexCount += submesh.IndexCount;

			SAT_CORE_ASSERT( mesh->HasPositions(), "Meshes require positions." );
			SAT_CORE_ASSERT( mesh->HasNormals(), "Meshes require normals." );

			// Vertices
			for( size_t i = 0; i < mesh->mNumVertices; i++ )
			{
				Vertex vertex;
				vertex.Position ={ mesh->mVertices[ i ].x, mesh->mVertices[ i ].y, mesh->mVertices[ i ].z };
				vertex.Normal ={ mesh->mNormals[ i ].x, mesh->mNormals[ i ].y, mesh->mNormals[ i ].z };

				if( mesh->HasTangentsAndBitangents() )
				{
					vertex.Tangent ={ mesh->mTangents[ i ].x, mesh->mTangents[ i ].y, mesh->mTangents[ i ].z };
					vertex.Binormal ={ mesh->mBitangents[ i ].x, mesh->mBitangents[ i ].y, mesh->mBitangents[ i ].z };
				}

				if( mesh->HasTextureCoords( 0 ) )
					vertex.Texcoord ={ mesh->mTextureCoords[ 0 ][ i ].x, mesh->mTextureCoords[ 0 ][ i ].y };

				m_StaticVertices.push_back( vertex );
			}

			// Indices
			for( size_t i = 0; i < mesh->mNumFaces; i++ )
			{
				SAT_CORE_ASSERT( mesh->mFaces[ i ].mNumIndices == 3, "Mesh must have 3 indices." );

				Index index ={ mesh->mFaces[ i ].mIndices[ 0 ], mesh->mFaces[ i ].mIndices[ 1 ], mesh->mFaces[ i ].mIndices[ 2 ] };
				m_Indices.push_back( index );

				m_TriangleCache[ m ].emplace_back( m_StaticVertices[ index.V1 + submesh.BaseVertex ], m_StaticVertices[ index.V2 + submesh.BaseVertex ], m_StaticVertices[ index.V3 + submesh.BaseVertex ] );
			}
		}

		TraverseNodes( scene->mRootNode );

		// Materials
		if( scene->HasMaterials() )
		{
			m_Textures.resize( scene->mNumMaterials );

			for( uint32_t i = 0; i < scene->mNumMaterials; i++ )
			{
				auto aiMaterial = scene->mMaterials[ i ];
				auto aiMaterialName = aiMaterial->GetName();

				// TODO: More than one material!
				Ref<Material> mat = Ref<Material>::Create( m_MeshShader );

				SAT_CORE_INFO( "  {0} (Index = {1})", aiMaterialName.data, i );
				aiString aiTexPath;
				uint32_t textureCount = aiMaterial->GetTextureCount( aiTextureType_DIFFUSE );
				SAT_CORE_INFO( "    TextureCount = {0}", textureCount );

				aiColor3D aiColor;
				aiMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, aiColor );

				float shininess, metalness;
				if( aiMaterial->Get( AI_MATKEY_SHININESS, shininess ) != aiReturn_SUCCESS )
					shininess = 80.0f; // Default value

				if( aiMaterial->Get( AI_MATKEY_REFLECTIVITY, metalness ) != aiReturn_SUCCESS )
					metalness = 0.0f;

				float roughness = 1.0f - glm::sqrt( shininess / 100.0f );
				SAT_CORE_INFO( "    COLOR = {0}, {1}, {2}", aiColor.r, aiColor.g, aiColor.b );
				SAT_CORE_INFO( "    ROUGHNESS = {0}", roughness );
				bool hasAlbedoMap = aiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &aiTexPath ) == AI_SUCCESS;

				if( hasAlbedoMap )
				{
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string( aiTexPath.data );
					std::string texturePath = parentPath.string();

					SAT_CORE_INFO( "    Albedo map path = {0}", texturePath );

					auto texture = Ref<Texture2D>::Create( texturePath, true );

					if( texture->Loaded() )
					{
						m_Textures[ i ] = texture;

						mat->Add( m_Textures[ i ]->Filename(), m_Textures[ i ], MaterialTextureType::Albedo );
						mat->Set( m_Textures[ i ]->Filename(), m_Textures[ i ] );
					}
					else
					{
						SAT_CORE_ERROR( "Could not load texture: {0}", texturePath );
						// Fallback to albedo color
						mat->Set( "u_AlbedoColor", glm::vec3{ aiColor.r, aiColor.g, aiColor.b } );
					}
				}

				// Specular
				bool hasSpecularMap = aiMaterial->GetTexture( aiTextureType_SPECULAR, 0, &aiTexPath ) == AI_SUCCESS;
				if( hasSpecularMap ) 
				{
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string( aiTexPath.data );
					std::string texturePath = parentPath.string();

					SAT_CORE_INFO( "    Specular map path = {0}", texturePath );

					auto texture = Ref<Texture2D>::Create( texturePath, false, true );

					if( texture->Loaded() )
					{
						m_Textures[ i ] = texture;

						mat->Add( m_Textures[ i ]->Filename(), m_Textures[ i ], MaterialTextureType::Specular );
						mat->Set( m_Textures[ i ]->Filename(), m_Textures[ i ] );
					}
					else
					{
						SAT_CORE_ERROR( "Could not load texture: {0}", texturePath );
						// Fallback to specular color
						//mat->Set( "u_SpecularColor", glm::vec3{ aiColor.r, aiColor.g, aiColor.b } );
					}
				}

				// Normal
				bool hasNormalMap = aiMaterial->GetTexture( aiTextureType_NORMALS, 0, &aiTexPath ) == AI_SUCCESS;
				if( hasNormalMap )
				{
					std::filesystem::path path = filename;
					auto parentPath = path.parent_path();
					parentPath /= std::string( aiTexPath.data );
					std::string texturePath = parentPath.string();

					SAT_CORE_INFO( "    Normal map path = {0}", texturePath );

					auto texture = Ref<Texture2D>::Create( texturePath, false, true );

					if( texture->Loaded() )
					{
						m_Textures[ i ] = texture;

						mat->Add( m_Textures[ i ]->Filename(), m_Textures[ i ], MaterialTextureType::Normal );
						mat->Set( m_Textures[ i ]->Filename(), m_Textures[ i ] );
					}
					else
					{
						SAT_CORE_ERROR( "Could not load texture: {0}", texturePath );
						// Fallback to normal color
						//mat->Set( "u_NormalColor", glm::vec3{ aiColor.r, aiColor.g, aiColor.b } );
					}
				}

				m_MeshMaterial = mat;
			}
		}

		VertexBufferLayout vertexLayout;

		vertexLayout ={
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Binormal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		m_VertexBuffer = Ref<VertexBuffer>::Create( m_StaticVertices.data(), m_StaticVertices.size() * sizeof( Vertex ) );
		m_IndexBuffer = Ref<IndexBuffer>::Create( m_Indices.data(), m_Indices.size() * sizeof( Index ) );

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.Layout = vertexLayout;
		m_Pipeline = Ref<Pipeline>::Create( pipelineSpecification );

		m_VertexCount = vertexCount;
		m_TriangleCount = m_TriangleCache.size();
		m_VerticesCount = m_StaticVertices.size();
		m_IndicesCount = m_Indices.size();
	}

	Mesh::Mesh( const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform ) : m_StaticVertices( vertices ), m_Indices( indices )
	{
		Submesh submesh;
		submesh.BaseVertex = 0;
		submesh.BaseIndex = 0;
		submesh.IndexCount = indices.size() * 3;
		submesh.Transform = transform;
		m_Submeshes.push_back( submesh );

		VertexBufferLayout vertexLayout;

		vertexLayout ={
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float3, "a_Tangent"  },
			{ ShaderDataType::Float3, "a_Binormal" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.Layout = vertexLayout;
		m_Pipeline = Ref<Pipeline>::Create( pipelineSpecification );

		m_VertexBuffer = Ref<VertexBuffer>::Create( m_StaticVertices.data(), m_StaticVertices.size() * sizeof( Vertex ) );
		m_IndexBuffer = Ref<IndexBuffer>::Create( m_Indices.data(), m_Indices.size() * sizeof( Index ) );
	}

	Mesh::~Mesh()
	{
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
		}

		for( uint32_t i = 0; i < node->mNumChildren; i++ )
			TraverseNodes( node->mChildren[ i ], transform, level + 1 );
	}

	Ref<Material> Mesh::GetMaterial()
	{
		return m_MeshMaterial;
	}

	void Mesh::UnbindLastTexture()
	{
		glBindTexture( GL_TEXTURE_2D, 0 );
	}

}