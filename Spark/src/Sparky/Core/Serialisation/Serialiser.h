#pragma once

#include <filesystem>

#include <json/json.h>

#include "Types.h"

#define OBJ_NAME(name) Serialiser(name)
#define MEMBER(name, var, type) new Serialiser<type>(#name, &var)
#define MEMBER(name, type) new Serialiser<type>(#name, &var)

namespace Sparky {
	class Serialiser
	{
	public:
		Serialiser(const std::string& objectname, bool shouldSerialise = true);
		Serialiser(const std::string& objectname, Json::Value& reconstructionValue);

		static void Init();

		virtual ~Serialiser();
		virtual void Serialise(Json::Value& members);
		virtual void Deserialise(Json::Value& members);

		std::string m_ObjectName;
		bool m_shouldSerialise;
	protected:

		virtual void archive() {};
		void SerialisationData(GenericSerialisable* serialisable) {
		
			m_Serialisables.push_back(serialisable);
		
		}

		template<typename T, class... Targs>
		void SerialisationData(const GenericSerialisable& serialisable, T value, Targs... Fargs) {
		
			m_Serialisables.push_back(serialisable);

			SerialisationData(value, Fargs...);
		
		}


		std::vector<GenericSerialisable*> m_Serialisables;
	};
}
