#include "sppch.h"
#include "Serialiser.h"

#ifdef _YAML
	#include <glm/glm.hpp>
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
#endif // YAML

	Serialiser::Serialiser(const std::string& objectname, bool shouldSerialise) : m_shouldSerialise(true), m_ObjectName(objectname)
	{
	}

	Serialiser::Serialiser(const std::string& objectname, Json::Value& reconstructionValue) : m_shouldSerialise(true), m_ObjectName(objectname)
	{


	}

	void Serialiser::Init()
	{
		SAT_PROFILE_FUNCTION();
		SAT_CORE_WARN("Serialiser inited! ");
	}

	Serialiser::~Serialiser() {}

#ifdef _YAML
	void Serialiser::Serialise(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		Application::Get().GetCurrentScene().GetRegistry().each([&](auto entityID)
		{
			Entity entity = { entityID, Application::Get().GetCurrentScene().GetRegistry().get() };
			if (!entity)
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
		for (int i = 0; i < Application::Get().m_gameLayer->GetGameObjects().size(); i++)
		{
			Json::Value x;
			for (GameObject gb : Application::Get().m_gameLayer->GetGameObjects())
			{
				SerialiseEntity(x, Application::Get().m_gameLayer->GetGameObjects().at(i));
			}
		}
	}
#endif
	
#ifdef _YAML
	void Serialiser::SerialiseEntity(YAML::Emitter& out, Entity entity)
	{

	}

	void Serialiser::SerialiseEntity(YAML::Emitter& out, GameObject entity)
	{

		SAT_CORE_ASSERT(
			entity.HasComponent<TagComponent>()
			&& entity.HasComponent<TransformComponent>()
			&& entity.HasComponent<IdComponent>(), 
			"Error! entity dose not have a TagComponent, TransformComponent and a IdComponent!"
		);

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<IdComponent>().ID; // TODO: Entity ID goes here

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

		out << YAML::EndMap; // Entity
	}
#else
	void Serialiser::SerialiseEntity(Json::Value& members, Entity entity)
	{
		for (int i = 0; i < Application::Get().m_gameLayer->GetGameObjects().size(); i++)
		{
			auto entitys = Application::Get().m_gameLayer->GetGameObjects().at(i);

			if (entitys)
			{
				/*

				members["Entitys"]
					[entitys.GetComponent<TagComponent>().Tag]
				["EntityName"] = entitys.GetComponent<TagComponent>().Tag;

				members["Entitys"]
					[entitys.GetComponent<TagComponent>().Tag]
				["ID"] = uint64_t(entitys.GetComponent<IdComponent>().ID);

				float* pos = glm::value_ptr(entitys.GetComponent<TransformComponent>().Position);
				float* rot = glm::value_ptr(entitys.GetComponent<TransformComponent>().Rotation);
				float* scale = glm::value_ptr(entitys.GetComponent<TransformComponent>().Scale);

				members["Entitys"]
					[entitys.GetComponent<TagComponent>().Tag]
				["Transform"]["Pos"] = float(int(pos));

				members["Entitys"]
					[entitys.GetComponent<TagComponent>().Tag]
				["Transform"]["Rot"] = float(int(rot));

				members["Entitys"]
					[entitys.GetComponent<TagComponent>().Tag]
				["Transform"]["Scale"] = float(int(scale));

				members["Entitys"]
					[entitys.GetComponent<TagComponent>().Tag]
				["Mesh"]["Model Name"] = entitys.GetComponent<MeshComponent>().GetModel()->GetName();

				members["Entitys"]
					[entitys.GetComponent<TagComponent>().Tag]
				["Mesh"]["Model Materials"] = entitys.GetComponent<MeshComponent>().GetModel()->GetMaterial().size();


				for (int i = 0; i < entitys.GetComponent<MeshComponent>().GetModel()->GetMaterial().size(); i++)
				{
					members["Entitys"]
						[entitys.GetComponent<TagComponent>().Tag]
					["Mesh"]["Model Material Name (At the Index)"] = entitys.GetComponent<MeshComponent>().GetModel()->GetMaterial().at(i)->GetName();

				}

				if (entitys.HasComponent<RelationshipComponent>())
				{
					members["Entitys"]
						[entitys.GetComponent<TagComponent>().Tag]
					["Relationship"]["RelativeTransform"] = entitys.GetComponent<RelationshipComponent>().RelativeTransform.length();

					for (int i = 0; i < entitys.GetComponent<RelationshipComponent>().Children.size(); i++)
					{
						members["Entitys"]
							[entitys.GetComponent<TagComponent>().Tag]
						["Relationship"]["Children"] = i;
					}
				}
				*/
			}
		}

		{
			std::ofstream fout("Scene1.json");
			fout << members;
		}
		
	}

	void Serialiser::SerialiseEntity(Json::Value& members, GameObject entity)
	{
		Json::Value x;

		for (int i = 0; i < Application::Get().m_gameLayer->GetGameObjects().size(); i++)
		{
			auto entitys = Application::Get().m_gameLayer->GetGameObjects().at(i);

			if (entitys)
			{

				/*

				//members["Entitys"] = Json::Value(Json::arrayValue);
				
				members.append(Json::Value::null);
				members.clear();

				x["Entitys"] = members;

				x["Entitys"][i][entitys.GetComponent<TagComponent>().Tag]["EntityName"] = entitys.GetComponent<TagComponent>().Tag;

				x["Entitys"][i]
					[entitys.GetComponent<TagComponent>().Tag]
				["ID"] = uint64_t(entitys.GetComponent<IdComponent>().ID);

				float* pos = glm::value_ptr(entitys.GetComponent<TransformComponent>().Position);
				float* rot = glm::value_ptr(entitys.GetComponent<TransformComponent>().Rotation);
				float* scale = glm::value_ptr(entitys.GetComponent<TransformComponent>().Scale);

				x["Entitys"][i]
					[entitys.GetComponent<TagComponent>().Tag]
				["Transform"]["Pos"] = float(int(pos));

				x["Entitys"][i]
					[entitys.GetComponent<TagComponent>().Tag]
				["Transform"]["Rot"] = float(int(rot));

				x["Entitys"][i]
					[entitys.GetComponent<TagComponent>().Tag]
				["Transform"]["Scale"] = float(int(scale));

				x["Entitys"][i]
					[entitys.GetComponent<TagComponent>().Tag]
				["Mesh"]["Model Name"] = entitys.GetComponent<MeshComponent>().GetModel()->GetName();

				x["Entitys"][i]
					[entitys.GetComponent<TagComponent>().Tag]
				["Mesh"]["Model Materials"] = entitys.GetComponent<MeshComponent>().GetModel()->GetMaterial().size();


				for (int i = 0; i < entitys.GetComponent<MeshComponent>().GetModel()->GetMaterial().size(); i++)
				{
					x["Entitys"][i]
						[entitys.GetComponent<TagComponent>().Tag]
					["Mesh"]["Model Material Name (At the Index)"] = entitys.GetComponent<MeshComponent>().GetModel()->GetMaterial().at(i)->GetName();
					
				}

				if (entitys.HasComponent<RelationshipComponent>())
				{
					x["Entitys"][i]
						[entitys.GetComponent<TagComponent>().Tag]
					["Relationship"]["RelativeTransform"] = entitys.GetComponent<RelationshipComponent>().RelativeTransform.length();

					for (int i = 0; i < entitys.GetComponent<RelationshipComponent>().Children.size(); i++)
					{
						x["Entitys"][i]
							[entitys.GetComponent<TagComponent>().Tag]
						["Relationship"]["Children"] = i;
					}
				}
				*/
			}
		}

		{
			std::ofstream fout("Scene1.json");
			fout << x;
		}
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
