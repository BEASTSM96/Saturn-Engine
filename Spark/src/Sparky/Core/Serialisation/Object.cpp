#include "sppch.h"
#include "Object.h"

namespace Sparky
{
	Object::Object() : OBJ_NAME("Object")
	{
		archive();
	}

	Object::Object(const std::string& objectname, Json::Value& reconstructionValue) 
		: Serialiser(objectname, reconstructionValue)
	{
		archive();
		Deserialise(reconstructionValue);
	}
}