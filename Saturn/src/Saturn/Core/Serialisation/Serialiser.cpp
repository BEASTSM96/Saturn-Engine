#include "sppch.h"
#include "Serialiser.h"

#include <yaml-cpp/yaml.h>

#include "Saturn/Application.h"
#include "Saturn/Scene/Scene.h"

#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"

#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <string.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::quat> {
		static Node encode(const glm::quat& rhs) {
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs) {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();

			return true;
		}
	};

}


namespace Saturn {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}


	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}


	Serialiser::Serialiser(const std::string& objectname, bool shouldSerialise) : m_shouldSerialise(true), m_ObjectName(objectname)
	{
	}

	Serialiser::Serialiser(const Ref<Scene>& scene) 
		: m_Scene(scene), m_shouldSerialise(true)
	{
	}

	void Serialiser::Init( void )
	{
		SAT_PROFILE_FUNCTION();
		SAT_CORE_WARN("Serialiser inited! ");
	}

	Serialiser::~Serialiser() {}

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	void Serialiser::SerialiseEntity(YAML::Emitter& out, Entity entity)
	{
		SAT_CORE_ASSERT(
			entity.HasComponent<TagComponent>()
			&& entity.HasComponent<TransformComponent>()
			&& entity.HasComponent<IdComponent>(),
			"Entity dose not have a TagComponent, TransformComponent and a IdComponent!"
		);

		UUID uuid = entity.GetComponent<IdComponent>().ID;
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << uuid; // TODO: Entity ID goes here

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}
		else
			SAT_CORE_ASSERT(entity.HasComponent<TagComponent>(), "Entity dose not have a TagComponent!");

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << tc.Position;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}
		else
			SAT_CORE_ASSERT(entity.HasComponent<TransformComponent>(), "Entity dose not have a TransformComponent!");


		if (entity.HasComponent<MeshComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; // MeshComponent

			auto mesh = entity.GetComponent<MeshComponent>().Mesh;
			out << YAML::Key << "AssetPath" << YAML::Value << mesh->GetFilePath();

			out << YAML::EndMap; // MeshComponent
		}
		out << YAML::EndMap; // Entity
	}

	static void SerialiseEnvironment(YAML::Emitter& out, const Ref<Scene>& scene)
	{
		out << YAML::Key << "Environment";
		out << YAML::Value;
		out << YAML::BeginMap; // Environment
		out << YAML::Key << "AssetPath" << YAML::Value << scene->GetEnvironment().FilePath;
		const auto& light = scene->GetLight();
		out << YAML::Key << "Light" << YAML::Value;
		out << YAML::BeginMap; // Light
		out << YAML::Key << "Direction" << YAML::Value << light.Direction;
		out << YAML::Key << "Radiance" << YAML::Value << light.Radiance;
		out << YAML::Key << "Multiplier" << YAML::Value << light.Multiplier;
		out << YAML::EndMap; // Light
		out << YAML::EndMap; // Environment
	}
	
	void Serialiser::Serialise(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << "Scene Name";
		SerialiseEnvironment(out, m_Scene);
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.Raw() };
			if (!entity || !entity.HasComponent<IdComponent>())
				return;

			SerialiseEntity(out, entity);

		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();

	}

	void Serialiser::Deserialise(const std::string& filepath)
	{

		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return;

		std::string sceneName = data["Scene"].as<std::string>();
		SAT_CORE_INFO("Deserializing scene '{0}'", sceneName);

		auto environment = data["Environment"];
		if (environment)
		{
			std::string envPath = environment["AssetPath"].as<std::string>();
			m_Scene->SetEnvironment(Environment::Load(envPath));

			auto lightNode = environment["Light"];
			if (lightNode)
			{
				auto& light = m_Scene->GetLight();
				light.Direction = lightNode["Direction"].as<glm::vec3>();
				light.Radiance = lightNode["Radiance"].as<glm::vec3>();
				light.Multiplier = lightNode["Multiplier"].as<float>();
			}
		}

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				SAT_CORE_INFO("Deserialized entity with ID ({0}), name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& transform = deserializedEntity.GetComponent<TransformComponent>().GetTransform();
					glm::vec3 translation = transformComponent["Position"].as<glm::vec3>();
					glm::quat rotation = transformComponent["Rotation"].as<glm::quat>();
					glm::vec3 scale = transformComponent["Scale"].as<glm::vec3>();

					transform = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);

					deserializedEntity.GetComponent<TransformComponent>().Position = translation;
					deserializedEntity.GetComponent<TransformComponent>().Rotation = rotation;
					deserializedEntity.GetComponent<TransformComponent>().Scale = scale;

					SAT_CORE_INFO("  Entity Transform:");
					SAT_CORE_INFO("    Translation: {0}, {1}, {2}", translation.x, translation.y, translation.z);
					SAT_CORE_INFO("    Rotation: {0}, {1}, {2}, {3}", rotation.w, rotation.x, rotation.y, rotation.z);
					SAT_CORE_INFO("    Scale: {0}, {1}, {2}", scale.x, scale.y, scale.z);
				}

				auto meshComponent = entity["MeshComponent"];
				if (meshComponent)
				{
					std::string meshPath = meshComponent["AssetPath"].as<std::string>();
					if (!deserializedEntity.HasComponent<MeshComponent>())
						deserializedEntity.AddComponent<MeshComponent>(Ref<Mesh>::Create(meshPath));

					SAT_CORE_INFO("  Mesh Asset Path: {0}", meshPath);
				}

			}
		}

	}

}
