#include "sppch.h"
#include "EngineSettings.h"

namespace Saturn {

	static std::string m_StartupSceneName;
	static std::string m_ProjectName;
	static std::string m_AssetPath;

	void ProjectSettings::SetStartupSceneName( std::string scenename )
	{
		m_StartupSceneName = scenename;
	}

	std::string ProjectSettings::GetStartupSceneName()
	{
		return m_StartupSceneName;
	}

}
