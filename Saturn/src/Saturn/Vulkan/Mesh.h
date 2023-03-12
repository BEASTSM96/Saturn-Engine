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

#include "Saturn/Core/Base.h"
#include "Saturn/Core/AABB/AABB.h"
#include "Saturn/Core/Renderer/EditorCamera.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

#include "Saturn/Asset/MaterialAsset.h"

#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Saturn {

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
	class MaterialInstance;

	class StaticMesh : public Asset
	{
	public:
		StaticMesh() {}
		StaticMesh( const std::string& rFilepath );
		~StaticMesh();

		std::string& FilePath() { return m_FilePath; }
		const std::string& FilePath() const { return m_FilePath; }

		void SetFilepath( const std::string& rFilepath ) { m_FilePath = rFilepath; }

		glm::mat4 GetInverseTransform() const { return m_InverseTransform; }
		glm::mat4 GetTransform() const { return m_Transform; }

		std::vector< Ref< MaterialAsset > >& GetMaterialAssets() { return m_MaterialsAssets; }
		const std::vector< Ref< MaterialAsset > >& GetMaterialAssets() const { return m_MaterialsAssets; }

		std::vector<Submesh>& Submeshes() { return m_Submeshes; }
		const std::vector<Submesh>& Submeshes() const { return m_Submeshes; }

		Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
		Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

		Ref<Shader> GetShader() { return m_MeshShader; }

	private:
		void TraverseNodes( aiNode* node, const glm::mat4& parentTransform = glm::mat4( 1.0f ), uint32_t level = 0 );
		void CreateVertices();
		void CreateMaterials();
	private:
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		std::vector<StaticVertex> m_Vertices;
		std::vector<Submesh> m_Submeshes;

		std::string m_FilePath;

		std::vector<Index> m_Indices;

		glm::mat4 m_InverseTransform = {};
		glm::mat4 m_Transform = {};

		uint32_t m_IndicesCount = 0;
		uint32_t m_VertexCount = 0;

		Ref<Shader> m_MeshShader;
		Ref<Material> m_BaseMaterial;
		std::vector< Ref< MaterialAsset > > m_MaterialsAssets;

		std::unique_ptr<Assimp::Importer> m_Importer;
		const aiScene* m_Scene = nullptr;
	};

	struct MeshInformation
	{
		uint32_t TriangleCount = 0;
		uint32_t IndicesCount = 0;
		uint32_t VerticesCount = 0;
		uint32_t Submeshes = 0;
	};

	// A mesh source class only exists to get information about a mesh, use the mesh class to render meshes.
	class MeshSource : public CountedObj
	{
	public:
		MeshSource( const std::filesystem::path& rPath, const std::filesystem::path& rDstPath );
		~MeshSource();

	private:
		void TraverseNodes( aiNode* node, const glm::mat4& parentTransform = glm::mat4( 1.0f ), uint32_t level = 0 );

		MeshInformation m_MeshInformation;

		std::unique_ptr<Assimp::Importer> m_Importer;

		const aiScene* m_Scene;
	};
}