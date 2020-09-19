#include "sppch.h"
#include "Serialiser.h"

#include "Object.h"
#include <iostream>

#include <fcntl.h>
#include <io.h>
#include <string.h>

namespace Saturn
{
	Serialiser::Serialiser(const std::string& objectname, bool shouldSerialise) : m_shouldSerialise(true), m_ObjectName(objectname)
	{
	}

	Serialiser::Serialiser(const std::string& objectname, Json::Value& reconstructionValue) : m_shouldSerialise(true), m_ObjectName(objectname)
	{


	}

	void Serialiser::Init()
	{
		SAT_CORE_WARN("Serialiser inited! ");

	}

	Serialiser::~Serialiser()
	{
		//error here

		//for (uint32_t i = 0; i < m_Serialisables.size(); i++)
			//delete m_Serialisables[i];
	}

	void Serialiser::Serialise(Json::Value& members)
	{
		if (m_shouldSerialise)
		{
			for (GenericSerialisable* i : m_Serialisables)
			{
				i->serialiseMember(members);
			}
		}
		else
			SAT_CORE_WARN("SERIALISER : {0} is false while you are trying to call the 'Serialise' func!", m_shouldSerialise);
	}

	void Serialiser::Deserialise(Json::Value& members)
	{

		if (m_shouldSerialise)
		{
			archive();


			uint32_t count = 0;
			bool found = false;
			for (std::string& memberName : members.getMemberNames())
			{
				for (GenericSerialisable* i : m_Serialisables)
				{
					if (i->memberKey == memberName)
					{
						found = true;

						i->deserialiseMember(members[memberName]);

						break;
					}
				}

				if (!found)
					SAT_CORE_WARN("SERIALISER : Couldn't find {0} during serialisation of {1}. It will just use it's default value", memberName, m_ObjectName);
				count++;
			}
		}
		else
			SAT_CORE_WARN("SERIALISER : {0} is false while you are trying to call the 'Deserialise' func!", m_shouldSerialise);

	}
}
