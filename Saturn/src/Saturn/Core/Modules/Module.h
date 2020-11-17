#pragma once

#include "Saturn/Core.h"
#include "Saturn/Core/Ref.h"

namespace Saturn {

	class ModuleManager;

	enum class ModuleMode
	{
		None	=		0x0,
		Engine	=		0x1,
		Editor	=		0x2,
		Game	=		0x4,
		Other	=		0x8
	};

	class Module : public RefCounted
	{
	public:
		Module(std::string path, std::string name);
		~Module();

		void Reload() {}

		std::string& GetName() { return m_Name; }


		Ref<ModuleManager> m_Manager;
	private:
		std::string m_Path;
		std::string m_Name;
	};

}