#pragma once

#include <string>

#include "Serialiser.h"

namespace Saturn {
	class SATURN_API Object : virtual public Serialiser
	{
	public:
		Object();
		Object(const std::string& objectname, Json::Value& reconstructionValue);
		virtual ~Object() {};

		uint32_t testval1 = 192;
	protected:
		virtual void archive() override {
		
			SerialisationData(new Serialisable<int>("Gjenstand", &testval1));
		}
		
	};
}