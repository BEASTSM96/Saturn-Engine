#pragma once

#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <json/json.h>
#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Scene.h"
#include "Types.h"

#pragma warning(disable: 4005)

namespace Saturn {
	class Serialiser
	{
	public:
		Serialiser(const std::string& objectname, bool shouldSerialise = true);
		Serialiser(const std::string& objectname, Json::Value& reconstructionValue);
		Serialiser(const Ref<Scene>& scene);

		static void Init( void );

		virtual ~Serialiser();

		virtual void Serialise(const std::string& filepath);
		virtual void Deserialise(const std::string& filepath);

		virtual void SerialiseEntity(YAML::Emitter& out, Entity entity);

		std::string m_ObjectName;
		bool m_shouldSerialise;
	private:
		Ref<Scene> m_Scene;

		friend class Scene;

	};
}
