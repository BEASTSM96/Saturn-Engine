#include "sppch.h"
#include "Serialiser.h"

//#define  YAML

#ifdef YAML
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

#ifdef YAML

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
#endif // YAML



namespace Saturn
{
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

#ifdef YAML
	void Serialiser::Serialise(const std::string& filepath)
	{

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
	
#ifdef YAML
	void Serialiser::SerialiseEntity(YAML::Emitter& out, Entity entity)
	{

	}
#else
	void Serialiser::SerialiseEntity(Json::Value& members, Entity entity)
	{
		for (int i = 0; i < Application::Get().m_gameLayer->GetGameObjects().size(); i++)
		{
			auto entitys = Application::Get().m_gameLayer->GetGameObjects().at(i);

			if (entitys)
			{
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
			}
		}

		{
			std::ofstream fout("Scene1.json");
			fout << members;
		}
		
	}

	void Serialiser::SerialiseEntity(Json::Value& members, GameObject entity)
	{
		for (int i = 0; i < Application::Get().m_gameLayer->GetGameObjects().size(); i++)
		{
			auto entitys = Application::Get().m_gameLayer->GetGameObjects().at(i);

			if (entitys)
			{
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
			}
		}

		{
			std::ofstream fout("Scene1.json");
			fout << members;
		}
	}

#endif


#ifdef YAML

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
			//ID = d.get("ID", 0).asFloat();

			DeserialisedStringVales.push_back(x.get("Entitys", 0).asString());
			DeserialisedStringVales.push_back(x.get("Cube", 0).asString());
			DeserialisedStringVales.push_back(x.get("EntityName", 0).asString());

			DeserialisedIntVales.push_back(x.get("ID", 0).asInt());

			DeserialisedStringVales.push_back(x.get("Mesh", 0).asString());
			DeserialisedStringVales.push_back(x.get("Model Material Name (At the Index)", 0).asString());
			DeserialisedStringVales.push_back(x.get("Model Materials", 0).asString());

			DeserialisedStringVales.push_back(x.get("EntityName", 0).asString());
		}
	}

#endif // YAML

}
