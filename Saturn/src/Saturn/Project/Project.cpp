#include "sppch.h"
#include "Project.h"

#include <filesystem>

namespace Saturn {

	Project::Project( UUID& uuid )
	{
		m_UUID = uuid;
		m_WorkingDir = std::filesystem::current_path().string();
		m_WorkingDir += "assets";
	}

	Project::Project( std::string& filepath, std::string name )
	{
		m_Name = name;
		m_AssetsPath = filepath;
	}

	Project::~Project()
	{

	}

	void Project::CopyAssets()
	{
		if( std::filesystem::exists( "assets\\" + m_Name + "Project" ) )
		{
			std::filesystem::copy( m_WorkingDir, m_AssetsPath );
		}
	}
}
