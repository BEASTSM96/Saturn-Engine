#pragma once

#include "Saturn/Core.h"
#include "Saturn/Core/Ref.h"
#include <vector>

namespace Saturn {

	class Module;

	class ModuleManager : public RefCounted
	{
	public:
		ModuleManager();
		~ModuleManager();

		void InitNewModule(Ref<Module>& module, std::string name, std::string path);
		void AddGameModule(Ref<Module>& gamemodule);
		Ref<Module> CopyModuleFrom(Ref<Module> moudule);

		Ref<Module> GetGameModule() { return m_GameModule; }
		std::vector<Ref<Module>> GetModules() { return m_Modules; }
	private:
		std::vector<Ref<Module>>m_Modules;
		Ref<Module>m_GameModule;
	};
}