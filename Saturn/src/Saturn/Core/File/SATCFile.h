#pragma once

#include "Saturn/Core.h"
#include <fstream>

namespace Saturn {

#define _SATCFIETYPEVOID() SATC
#define _OFSTREAMTYPE(name, path) std::ofstream(name + path)
#define _IFSTREAMTYPE(name, path) std::ifstream(name + path)
#define _TYPEVOID() void


	class SATURN_API SATC //Scene File
	{
	public:

		_SATCFIETYPEVOID()(std::string name, std::string path);
		~_SATCFIETYPEVOID()();

		std::ofstream NewFile(std::string name, std::string path);

	private:

	};
}