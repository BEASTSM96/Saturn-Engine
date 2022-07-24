/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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
#include "Project.h"

namespace Saturn {
	
	static Ref<Project> s_ActiveProject;

	Project::Project()
	{
	}

	Project::~Project()
	{
	}

	Ref<Project> Project::GetActiveProject()
	{
		return s_ActiveProject;
	}

	void Project::SetActiveProject( const Ref<Project>& rProject )
	{
		SAT_CORE_ASSERT( rProject, "Project must be not be null!" );
		s_ActiveProject = rProject;
	}

	std::filesystem::path Project::GetAssetPath()
	{
		return std::filesystem::path( GetActiveProject()->GetConfig().Path ) / "Content" / "Assets";
	}

	//////////////////////////////////////////////////////////////////////////

	void CreateProjectResources( Ref<Project>& rProject, const std::string& Name, const std::string& Path )
	{
		std::filesystem::path ProjectPath = Path;
		std::filesystem::path ProjectContentPath = ProjectPath / "Content";
		
		if( !std::filesystem::exists( ProjectContentPath ) )
			std::filesystem::create_directories( ProjectContentPath );

		std::filesystem::create_directory( ProjectContentPath );
		std::filesystem::create_directory( ProjectContentPath / "Assets" );

		std::filesystem::create_directories( ProjectContentPath / "Assets" / "Shaders" );
		std::filesystem::create_directories( ProjectContentPath / "Assets" / "Textures" );
		std::filesystem::create_directories( ProjectContentPath / "Assets" / "Meshes" );
		std::filesystem::create_directories( ProjectContentPath / "Assets" / "Materials" );
		std::filesystem::create_directories( ProjectContentPath / "Assets" / "Scenes" );
		std::filesystem::create_directories( ProjectContentPath / "Assets" / "Sound" );
		std::filesystem::create_directories( ProjectContentPath / "Assets" / "Sound" / "Source" );
		std::filesystem::create_directory( ProjectContentPath / "Scripts" );

		rProject->m_Config.Name = Name;
		rProject->m_Config.Path = Path;
	}

}