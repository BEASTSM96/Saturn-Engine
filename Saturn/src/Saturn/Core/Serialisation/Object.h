#pragma once

#include <string>

#include "Serialiser.h"

namespace Saturn {
	class Object
	{
	public:
		Object();
		Object(const std::string& objectname, Json::Value& reconstructionValue);
		virtual ~Object() {};
	};
}