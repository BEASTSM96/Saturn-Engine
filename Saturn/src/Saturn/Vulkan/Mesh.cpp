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

	Mesh::Mesh( const std::string& filename, UUID uuid ) : m_FilePath( filename )
	{
		LogStream::Initialize();

		SAT_CORE_INFO( "Loading mesh: {0}", filename.c_str() );

		m_Importer = std::make_unique<Assimp::Importer>();

		const aiScene* scene = m_Importer->ReadFile( filename, s_MeshImportFlags );
		if( scene == nullptr || !scene->HasMeshes() )
			SAT_CORE_ERROR( "Failed to load mesh file: {0}", filename );

		m_Scene = scene;
		m_MeshShader = Ref<Shader>::Create( "Mesh/VertexShader", "assets\\shaders\\shader_new.glsl" );
		
		m_InverseTransform = glm::inverse( Mat4FromAssimpMat4( scene->mRootNode->mTransformation ) );
		
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		
		std::vector<uint32_t> Indices;

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
				MeshVertex vertex ={};
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
				
				Indices.push_back( mesh->mFaces[ i ].mIndices[ 0 ] );
				Indices.push_back( mesh->mFaces[ i ].mIndices[ 1 ] );
				Indices.push_back( mesh->mFaces[ i ].mIndices[ 2 ] );
				

				m_TriangleCache[ m ].emplace_back( m_StaticVertices[ index.V1 + submesh.BaseVertex ], m_StaticVertices[ index.V2 + submesh.BaseVertex ], m_StaticVertices[ index.V3 + submesh.BaseVertex ] );
			}
		}

		TraverseNodes( scene->mRootNode );
		
		m_VertexBuffer = Ref<VertexBuffer>::Create( m_StaticVertices.data(), m_StaticVertices.size() * sizeof( MeshVertex ) );

		m_RealIndices.clear();

		for( Index& rIndex : m_Indices )
		{
			m_RealIndices.push_back( rIndex.V1 );
			m_RealIndices.push_back( rIndex.V2 );
			m_RealIndices.push_back( rIndex.V3 );
		}
		
		m_IndexBuffer = Ref<IndexBuffer>::Create( Indices.data(), Indices.size() );
		
		m_VertexCount = vertexCount;
		m_TriangleCount = m_TriangleCache.size();
		m_VerticesCount = m_StaticVertices.size();
		m_IndicesCount = m_Indices.size();

		// Create material.
		for( size_t m = 0; m < scene->mNumMaterials; m++ )
		{
			aiMaterial* material = scene->mMaterials[ m ];

			aiString name;
			material->Get( AI_MATKEY_NAME, name );

			aiColor3D color;
			material->Get( AI_MATKEY_COLOR_DIFFUSE, color );
			
			// Ask nicely to the shader if we can use the shader (and the shader will always say yes).
			m_MeshShader->UseUniform( "u_AlbedoTexture" );
			m_MeshShader->UseUniform( "u_NormalTexture" );
			m_MeshShader->UseUniform( "u_MetallicTexture" );
			m_MeshShader->UseUniform( "u_RoughnessTexture" );

			//ShaderUniform Albedo    = 
			//ShaderUniform Normal   =  
			//ShaderUniform Metallic =  
			//ShaderUniform Roughness = 
			
			std::string MatName = std::string( name.C_Str() );

			MaterialSpec* pSpec = new MaterialSpec(
				MatName,
				uuid,
				m_MeshShader->FindUniform( "u_AlbedoTexture" ),
				m_MeshShader->FindUniform( "u_NormalTexture" ),
				m_MeshShader->FindUniform( "u_MetallicTexture" ),
				m_MeshShader->FindUniform( "u_RoughnessTexture" ) );

			m_MeshMaterial = Ref<Material>::Create( pSpec );

			// Albedo Texture
			{
				aiString AlbedoTexturePath;
				if( material->GetTexture( aiTextureType_DIFFUSE, 0, &AlbedoTexturePath ) == AI_SUCCESS )
				{
					std::filesystem::path AlbedoPath = filename;
					auto pp = AlbedoPath.parent_path();
					
					pp /= std::string( AlbedoTexturePath.data );
					
					auto AlbedoTexturePath = pp.string();

					SAT_CORE_INFO( "MESH FOR ENTITY ID {0}: Albedo Map texture {1}", std::to_string( uuid ), AlbedoTexturePath );
					
					Ref< Texture2D > AlbedoTexture = Ref< Texture2D >::Create( AlbedoTexturePath, AddressingMode::Repeat );

					m_MeshMaterial->SetAlbedo( AlbedoTexture );	
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
	}

	Mesh::~Mesh()
	{
		m_MeshMaterial.Delete();

		m_VertexBuffer.Delete();
		m_IndexBuffer.Delete();

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
		}

		for( uint32_t i = 0; i < node->mNumChildren; i++ )
			TraverseNodes( node->mChildren[ i ], transform, level + 1 );
	}
}