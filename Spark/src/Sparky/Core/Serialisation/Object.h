#pragma once

#include <string>

#include "Serialiser.h"

namespace Sparky {
	class Object : virtual public Serialiser
	{
	public:
		Object();
		Object(const std::string& objectname, Json::Value& reconstructionValue);
		virtual ~Object() {};

		uint32_t testval1 = 192;
		std::string testvalstring = "Hello";
		bool testvalbool = true;


	protected:
		virtual void archive() override {
		
			SerialisationData(new Serialisable<int>("Gjenstand", &testval1));
			//SerialisationData(new Serialisable<std::string>("testVal2", &testvalstring));
			//SerialisationData(new Serialisable<bool>("testVal3", &testvalbool));
		}
		
	};
}