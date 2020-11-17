#pragma once

#include <string>

#include "Serialiser.h"

namespace Saturn {
	class SATURN_API Object
	{
	public:
		Object();
		Object(const std::string& objectname, Json::Value& reconstructionValue);
		virtual ~Object() {};
	};
}