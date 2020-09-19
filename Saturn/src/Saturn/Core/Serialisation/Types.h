#pragma once


#include "Saturn/Core.h"
#include <string>
#include <json/json.h>



#define CONSTRUCT Serialisable(const std::string& keyName, void* dataAddress) : GenericSerialisable(keyName, dataAddress) {  }

namespace Saturn {

	struct GenericSerialisable
	{
		GenericSerialisable(const std::string& memberName, void* dataAddress)
			: memberKey(memberName), dataAddress(dataAddress) {  }

		virtual void serialiseMember(Json::Value& member) = 0;
		virtual void deserialiseMember(Json::Value& member) = 0;

		std::string memberKey = "";
		void* dataAddress = nullptr;
	};

	template<typename T>
	struct Serialisable : GenericSerialisable
	{
		CONSTRUCT

		virtual void serialiseMember(Json::Value& member) override {
			member = *((T*)dataAddress);
		}

		virtual void deserialiseMember(Json::Value& member) override {
			*((T*)dataAddress) = member.asInt();
		}
	};

	/*
	template<> //INT
	struct Serialisable<int> : GenericSerialisable
	{
		CONSTRUCT

		virtual void serialiseMember(const Json::Value& member) override {
			SAT_CORE_ASSERT(member.isInt(), "Serialisation data must have be a type Int!");
			member = *((int*)dataAddress);
		}

		virtual void deserialiseMember(Json::Value& member) override {
			SAT_CORE_ASSERT(member.isInt(), "Deserialisation data must have be a type Int!");
			*((int*)dataAddress) = member.asInt();
		}

	};
	*/

	template<> //BOOL
	struct Serialisable<bool> : GenericSerialisable
	{
		CONSTRUCT

		virtual void serialiseMember(Json::Value& member) override {
			if(member.isBool())
				member = *((bool*)dataAddress);
		}

		virtual void deserialiseMember(Json::Value& member) override {
			bool x = false;
			//SAT_ASSERT(x, "test");
			*((bool*)dataAddress) = member.asBool();
		}

	};

	template<> //STRING
	struct Serialisable<std::string> : GenericSerialisable
	{
		CONSTRUCT

		virtual void serialiseMember(Json::Value& member) override {
			if (member.isString())
				member = *((std::string*)dataAddress);
		}

		virtual void deserialiseMember(Json::Value& member) override {
			*((std::string*)dataAddress) = member.asString();
		}

	};
}