#include "sppch.h"
#include "Module.h"
#include "ModuleManager.h"

namespace Saturn {

	Module::Module(std::string path, std::string name)
	{
		m_Path = path;
		m_Name = name;
	}

	Module::~Module()
	{
	}

}
