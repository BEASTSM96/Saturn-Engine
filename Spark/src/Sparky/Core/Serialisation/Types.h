#pragma once

#include <string>


#include <json/json.h>

#define CONSTRUCT Serialisable(const std::string& keyName, void* dataAddress) : GenericSerialisable(keyName, dataAddress) {  }

namespace Sparky {

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
			SP_CORE_ASSERT(member.isInt(), "Serialisation data must have be a type Int!");
			member = *((int*)dataAddress);
		}

		virtual void deserialiseMember(Json::Value& member) override {
			SP_CORE_ASSERT(member.isInt(), "Deserialisation data must have be a type Int!");
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
			SP_CORE_ASSERT(member.isBool(), "Deserialisation data must have be a type bool!");
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
			else 
				SP_CORE_ASSERT(member.isString(), "Serialisation data must have be a type string!");
		}

		virtual void deserialiseMember(Json::Value& member) override {
			SP_CORE_ASSERT(member.isString(), "Deserialisation data must have be a type string!");
			*((std::string*)dataAddress) = member.asString();
		}

	};
}