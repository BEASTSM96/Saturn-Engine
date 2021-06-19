/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include "sppch.h"
#include "EngineSettings.h"

#include "Saturn/Core/Serialisation/Serialiser.h"

namespace Saturn {

	static std::string s_StartupSceneName;
	static std::string s_ProjectName;
	static std::string s_AssetPath;
	static Ref<Project> s_CurrentProject;
	static bool s_MakeCurrentProjectStartup;
	static std::string s_StartupProjectName;
	static std::string s_StartupProjectFolder;

	void ProjectSettings::SetStartupSceneName( std::string scenename )
	{
		s_StartupSceneName = scenename;
	}

	std::string ProjectSettings::GetStartupSceneName()
	{
		return s_StartupSceneName;
	}

	void ProjectSettings::SetCurrentProject( Ref<Project>& project )
	{
		s_CurrentProject = project;
	}

	Ref<Project>& ProjectSettings::GetCurrentProject()
	{
		return s_CurrentProject;
	}

	void ProjectSettings::SetStartupNameFolder( std::string name, std::string folder )
	{
		if( !name.empty() )
			s_StartupProjectName = name;
		if( !folder.empty() )
			s_StartupProjectFolder = folder;
	}

	void ProjectSettings::SetStartupName( std::string name )
	{
		s_StartupProjectName = name;
	}

	void ProjectSettings::SetStartupFolder( std::string folder )
	{
		s_StartupProjectFolder = folder;
	}

	std::string& ProjectSettings::GetStartupProjectName()
	{
		return s_StartupProjectName;
	}

	std::string& ProjectSettings::GetStartupProjectFolder()
	{
		return s_StartupProjectFolder;
	}

	bool ProjectSettings::HasStartupProject()
	{
		if( !s_StartupProjectName.empty() && !s_StartupProjectFolder.empty() )
			return true;
	}

	void ProjectSettings::Save()
	{
		Serialiser s;
		s.SerialiseProjectSettings( "assets\\EngineSettings.eng" );
	}

	void ProjectSettings::Load()
	{
		Serialiser s;
		s.DeserialiseProjectSettings( "assets\\EngineSettings.eng" );
	}
}
