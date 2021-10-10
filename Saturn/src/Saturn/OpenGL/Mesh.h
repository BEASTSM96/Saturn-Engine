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

#pragma once

#include "Common.h"
#include "Saturn/Core/Base.h"
#include "Saturn/Core/AABB/AABB.h"
#include "Saturn/Core/Renderer/EditorCamera.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

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

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;
	};

	struct Index
	{
		uint32_t V1, V2, V3;
	};

	struct Triangle
	{
		Vertex V0, V1, V2;

		Triangle( const Vertex& v0, const Vertex& v1, const Vertex& v2 )
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
	};

	class Mesh
	{
	public:
		Mesh( const std::string& filename );
		Mesh( const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform );
		~Mesh();

		Ref<Shader> GetMeshShader() { return m_MeshShader; }
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
		const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }
		const std::string& GetFilePath() const { return m_FilePath; }

		const std::vector<Triangle> GetTriangleCache( uint32_t index ) const { return m_TriangleCache.at( index ); }

		const uint32_t& GetVertexCount() const { return m_VertexCount; }
		const uint32_t& GetTriangleCount() const { return m_TriangleCount; }
		const uint32_t& GetIndicesCount() const { return m_IndicesCount; }
		const uint32_t& GetVerticesCount() const { return m_VerticesCount; }

		const std::vector<Index>& GetIndices() const { return m_Indices; }
		const std::vector<Vertex>& GetVertices() const { return m_StaticVertices; }

	private:

		std::vector<Vertex> m_StaticVertices;

		std::vector<Submesh> m_Submeshes;

		std::vector<Index> m_Indices;

		std::vector<Ref<Texture2D>> m_Textures;

		std::vector<Ref<Texture2D>> m_NormalMaps;

		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

		std::unique_ptr<Assimp::Importer> m_Importer;

		std::string m_FilePath;

		glm::mat4 m_InverseTransform;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		Ref<Shader> m_MeshShader;
	
		uint32_t m_VertexCount = 0;
		uint32_t m_TriangleCount = 0;
		uint32_t m_IndicesCount = 0;
		uint32_t m_VerticesCount = 0;

		const aiScene* m_Scene;
	};
}