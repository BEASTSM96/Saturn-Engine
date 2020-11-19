#include "sppch.h"
#include "Serialiser.h"

#define _YAML

#ifdef _YAML
	#include <yaml-cpp/yaml.h>
#endif

#include "Object.h"

#include "Saturn/Application.h"
#include "Saturn/Core/World/Level.h"
#include "Saturn/GameBase/GameLayer.h"
#include "Saturn/Scene/Scene.h"

#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"

#include "Saturn/GameBase/GameObject.h"

#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <string.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _YAML

namespace YAML {

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

}

#endif // _YAML

namespace Saturn {

#ifdef _YAML
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

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

#endif // YAML

	Serialiser::Serialiser(const std::string& objectname, bool shouldSerialise) : m_shouldSerialise(true), m_ObjectName(objectname)
	{
	}

	Serialiser::Serialiser(const std::string& objectname, Json::Value& reconstructionValue) : m_shouldSerialise(true), m_ObjectName(objectname)
	{


	}

	Serialiser::Serialiser(const Ref<Scene>& scene) 
		: m_Scene(scene)
	{
	}

	void Serialiser::Init()
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

#ifdef _YAML
	void Serialiser::SerialiseEntity(YAML::Emitter& out, Entity entity)
	{
		SAT_CORE_ASSERT(
			entity.HasComponent<TagComponent>()
			&& entity.HasComponent<TransformComponent>()
			&& entity.HasComponent<IdComponent>(),
			"Error! entity dose not have a TagComponent, TransformComponent and a IdComponent!"
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
			SAT_CORE_ASSERT(entity.HasComponent<TagComponent>(), "Error! entity dose not have a TagComponent!");

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
			SAT_CORE_ASSERT(entity.HasComponent<TransformComponent>(), "Error! entity dose not have a TransformComponent!");


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

	void Serialiser::SerialiseEntity(YAML::Emitter& out, GameObject entity)
	{

		SAT_CORE_ASSERT(
			entity.HasComponent<TagComponent>()
			&& entity.HasComponent<TransformComponent>()
			&& entity.HasComponent<IdComponent>(), 
			"Error! entity dose not have a TagComponent, TransformComponent and a IdComponent!"
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
			SAT_CORE_ASSERT(entity.HasComponent<TagComponent>(), "Error! entity dose not have a TagComponent!");

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
			SAT_CORE_ASSERT(entity.HasComponent<TransformComponent>(), "Error! entity dose not have a TransformComponent!");


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
	
#endif

#ifdef _YAML
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
#else
	void Serialiser::Serialise(const std::string& filepath)
	{

	}
#endif



#ifdef _YAML

	void Serialiser::Deserialise(const std::string& filepath)
	{

	}

#else

	void Serialiser::Deserialise(const std::string& filepath)
	{
		Json::Value members;

		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		for (int i = 0; i < Application::Get().m_gameLayer->GetGameObjects().size(); i++)
		{
			auto entitys = Application::Get().m_gameLayer->GetGameObjects().at(i);

			std::string EntityName;
			float		ID;
			std::string Mesh;
			std::string ModelMaterialName;
			std::string ModelMaterials;
			std::string ModelName;
			float		Transform;
			float		Pos;
			float		Rot;
			float		Scale;

			std::vector<std::string>DeserialisedStringVales;
			std::vector<int>DeserialisedIntVales;

			Json::Value d = members["Entitys"];
			EntityName = d[0].get("EntityName", 0).asString();
			ID = d[0].get("ID", 0).asFloat();
		}
	}

#endif // YAML

}
