#include "sppch.h"
#include "Object.h"

namespace Saturn
{
	Object::Object() : OBJ_NAME("Object")
	{
	}

	Object::Object(const std::string& objectname, Json::Value& reconstructionValue) 
		: Serialiser(objectname, reconstructionValue)
	{
	}
}