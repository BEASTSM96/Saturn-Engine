#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Saturn/Core/Timestep.h"

#include "Saturn/Renderer/Pipeline.h"
#include "Saturn/Renderer/IndexBuffer.h"
#include "Saturn/Renderer/VertexBuffer.h"
#include "Saturn/Renderer/Shader.h"
#include "Saturn/Renderer/Material.h"

#include "Saturn/Core/AABB/AABB.h"

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

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 Texcoord;

		u32 IDs[4] = { 0, 0,0, 0 };
		float Weights[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(u32 BoneID, float Weight)
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (Weights[i] == 0.0)
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}

			// TODO: Keep top weights
			//SAT_CORE_WARN("Vertex has more than four bones/weights affecting it, extra data will be discarded (BoneID={0}, Weight={1})", BoneID, Weight);
		}
	};

	static const int NumAttributes = 5;

	struct Index
	{
		u32 V1, V2, V3;
	};

	static_assert(sizeof(Index) == 3 * sizeof(u32));

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	struct VertexBoneData
	{
		u32 IDs[4];
		float Weights[4];

		VertexBoneData()
		{
			memset(IDs, 0, sizeof(IDs));
			memset(Weights, 0, sizeof(Weights));
		};

		void AddBoneData(u32 BoneID, float Weight)
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (Weights[i] == 0.0)
				{
					IDs[i] = BoneID;
					Weights[i] = Weight;
					return;
				}
			}

			// should never get here - more bones than we have space for
			SAT_CORE_ASSERT(false, "Too many bones!");
		}
	};

	struct Triangle
	{
		Vertex V0, V1, V2;

		Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
			: V0(v0), V1(v1), V2(v2) {}
	};

	class Submesh
	{
	public:
		u32 BaseVertex;
		u32 BaseIndex;
		u32 MaterialIndex;
		u32 IndexCount;

		glm::mat4 Transform;
		AABB BoundingBox;

		std::string NodeName, MeshName;
	};

	class Mesh : public RefCounted
	{
	public:
		Mesh(const std::string& filename);
		~Mesh();

		void OnUpdate(Timestep ts);
		void DumpVertexBuffer();

		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

		Ref<Shader> GetMeshShader() { return m_MeshShader; }
		Ref<Material> GetMaterial() { return m_BaseMaterial; }
		std::vector<Ref<MaterialInstance>> GetMaterials() { return m_Materials; }
		const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }
		const std::string& GetFilePath() const { return m_FilePath; }

		const std::vector<Triangle> GetTriangleCache(u32 index) const { return m_TriangleCache.at(index); }
	private:
		void BoneTransform(float time);
		void ReadNodeHierarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), u32 level = 0);

		const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		u32 FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
		u32 FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		u32 FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
		glm::vec3 InterpolateTranslation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::quat InterpolateRotation(float animationTime, const aiNodeAnim* nodeAnim);
		glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* nodeAnim);
	private:
		std::vector<Submesh> m_Submeshes;

		std::unique_ptr<Assimp::Importer> m_Importer;

		glm::mat4 m_InverseTransform;

		u32 m_BoneCount = 0;
		std::vector<BoneInfo> m_BoneInfo;

		Ref<Pipeline> m_Pipeline;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		std::vector<Vertex> m_StaticVertices;
		std::vector<AnimatedVertex> m_AnimatedVertices;
		std::vector<Index> m_Indices;
		std::unordered_map<std::string, u32> m_BoneMapping;
		std::vector<glm::mat4> m_BoneTransforms;
		const aiScene* m_Scene;

		// Materials
		Ref<Shader> m_MeshShader;
		Ref<Material> m_BaseMaterial;
		std::vector<Ref<Texture2D>> m_Textures;
		std::vector<Ref<Texture2D>> m_NormalMaps;
		std::vector<Ref<MaterialInstance>> m_Materials;

		std::unordered_map<u32, std::vector<Triangle>> m_TriangleCache;

		// Animation
		bool m_IsAnimated = false;
		float m_AnimationTime = 0.0f;
		float m_WorldTime = 0.0f;
		float m_TimeMultiplier = 1.0f;
		bool m_AnimationPlaying = true;

		std::string m_FilePath;

		friend class Renderer;
		friend class SceneHierarchyPanel;
	};

}
