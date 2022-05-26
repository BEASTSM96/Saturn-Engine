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

#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Core/AABB/AABB.h"
#include "Saturn/Core/Renderer/EditorCamera.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp {
	class Importer;
}

namespace Saturn {

	struct Triangle
	{
		MeshVertex V0, V1, V2;

		Triangle( const MeshVertex& v0, const MeshVertex& v1, const MeshVertex& v2 )
			: V0( v0 ), V1( v1 ), V2( v2 )
		{
		}
	};

	class Submesh
	{
	public:
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex;
		uint32_t IndexCount;
		uint32_t VertexCount;

		glm::mat4 Transform;
		AABB BoundingBox;

		std::string NodeName, MeshName;
	public:
		bool operator==( const Submesh& other ) const
		{
			return BaseVertex == other.BaseVertex && BaseIndex == other.BaseIndex && MaterialIndex == other.MaterialIndex && IndexCount == other.IndexCount && VertexCount == other.VertexCount && NodeName == other.NodeName && MeshName == other.MeshName;
		}
	};

}

namespace std {

	template<>
	struct hash< Saturn::Submesh >
	{
		size_t operator()( const Saturn::Submesh& rOther ) const
		{
			return hash< std::string >()( rOther.NodeName );
		}
	};

}

namespace Saturn {

	class DescriptorSet;

	class Mesh
	{
	public:
		Mesh( const std::string& filename );
		Mesh( const std::vector<MeshVertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform );
		~Mesh();

		void TraverseNodes( aiNode* node, const glm::mat4& parentTransform = glm::mat4( 1.0f ), uint32_t level = 0 );

		Ref<Shader> MeshShader() { return m_MeshShader; }
		std::vector<Submesh>& Submeshes() { return m_Submeshes; }
		const std::vector<Submesh>& Submeshes() const { return m_Submeshes; }

		std::unordered_map< Submesh, Ref< DescriptorSet > >& GetDescriptorSets() { return m_DescriptorSets; }
		const std::unordered_map< Submesh, Ref< DescriptorSet > >& GetDescriptorSets() const { return m_DescriptorSets; }

		std::string& FilePath() { return m_FilePath; }
		const std::string& FilePath() const { return m_FilePath; }

		const std::vector<Triangle> TriangleCache( uint32_t index ) const { return m_TriangleCache.at( index ); }

		const uint32_t& VertexCount() const { return m_VertexCount; }
		const uint32_t& TriangleCount() const { return m_TriangleCount; }
		const uint32_t& IndicesCount() const { return m_IndicesCount; }
		const uint32_t& VerticesCount() const { return m_VerticesCount; }

		const std::vector<Index>& Indices() const { return m_Indices; }
		const std::vector<MeshVertex>& Vertices() const { return m_StaticVertices; }

		const Ref<Shader>& GetShader() const { return m_MeshShader; }
		Ref<Shader>& GetShader() { return m_MeshShader; }

		Ref<VertexBuffer>& GetVertexBuffer() { return m_VertexBuffer; }
		Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; }
		
		Ref<Material>& GetMaterial() { return m_MeshMaterial; }
		const Ref<Material>& GetMaterial() const { return m_MeshMaterial; }
		

		glm::mat4 GetTransform() const { return m_InverseTransform; }

	private:

		std::vector<MeshVertex> m_StaticVertices;

		std::vector<Submesh> m_Submeshes;
		std::vector< Ref<Material> > m_Materials;

		std::vector<Index> m_Indices;
		std::vector<uint32_t> m_RealIndices;
		
		std::unordered_map< Submesh, Ref< DescriptorSet > > m_DescriptorSets;

		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

		std::unique_ptr<Assimp::Importer> m_Importer;

		std::string m_FilePath;

		glm::mat4 m_InverseTransform;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		Ref<Shader> m_MeshShader;
		
		Ref<Material> m_MeshMaterial;

		uint32_t m_VertexCount = 0;
		uint32_t m_TriangleCount = 0;
		uint32_t m_IndicesCount = 0;
		uint32_t m_VerticesCount = 0;

		const aiScene* m_Scene;
	};
}