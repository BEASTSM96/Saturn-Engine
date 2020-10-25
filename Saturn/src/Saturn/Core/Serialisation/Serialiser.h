#pragma once

#include <filesystem>
#include <json/json.h>

#include "Saturn/Scene/Entity.h"

#define YAML

#ifdef YAML
#include <yaml-cpp/yaml.h>
#endif

#include "Types.h"

#pragma warning(disable: 4005)

#define OBJ_NAME(name) Serialiser(name)
#define MEMBER(name, var, type) new Serialiser<type>(#name, &var)
#define MEMBER(name, type) new Serialiser<type>(#name, &var)

namespace Saturn {
	class SATURN_API Serialiser
	{
	public:
		Serialiser(const std::string& objectname, bool shouldSerialise = true);
		Serialiser(const std::string& objectname, Json::Value& reconstructionValue);

		static void Init();

		virtual ~Serialiser();

#ifdef YAML
		virtual void Serialise(const std::string& filepath);
		virtual void Deserialise(const std::string& filepath);
#else
		virtual void Serialise(const std::string& filepath);
		virtual void Deserialise(const std::string& filepath);
#endif

#ifdef YAML
		virtual void SerialiseEntity(YAML::Emitter& out, Entity entity);
		virtual void SerialiseEntity(YAML::Emitter& out, GameObject entity);
#else
		void SerialiseEntity(Json::Value& members, Entity entity);
		void SerialiseEntity(Json::Value& members, GameObject entity);
#endif // YAML



		std::string m_ObjectName;
		bool m_shouldSerialise;
	protected:

		virtual void archive() {};
		void SerialisationData(GenericSerialisable* serialisable) {
			m_Serialisables.push_back(serialisable);
		}

		template<typename T, class ... Targs>
		void SerialisationData(const GenericSerialisable& serialisable, T value, Targs... Fargs) {
		
			m_Serialisables.push_back(serialisable);

			SerialisationData(value, Fargs...);
		
		}


		std::vector<GenericSerialisable*> m_Serialisables;
	};
}
