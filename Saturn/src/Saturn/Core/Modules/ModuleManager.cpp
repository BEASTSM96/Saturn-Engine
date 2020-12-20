#include "sppch.h"
#include "ModuleManager.h"
#include "Module.h"

namespace Saturn {

	ModuleManager::ModuleManager()
	{
	}

	ModuleManager::~ModuleManager()
	{
		for( int i = 0; i < m_Modules.size(); i++ )
		{
			delete m_Modules.at( i ).Raw();
		}

		m_Modules.clear();
	}

	void ModuleManager::InitNewModule( Ref<Module>& module, std::string name, std::string path )
	{
		module.Raw()->m_Manager = this;
		m_Modules.push_back( module );
	}

	void ModuleManager::AddGameModule( Ref<Module>& gamemodule )
	{
		gamemodule.Raw()->m_Manager = this;
		m_Modules.push_back( gamemodule );
	}

	Ref<Module> ModuleManager::CopyModuleFrom( Ref<Module> moudule )
	{
		Ref<Module>NewModule = moudule;
		NewModule.Raw()->m_Manager = moudule.Raw()->m_Manager;
		return NewModule;
	}

}
