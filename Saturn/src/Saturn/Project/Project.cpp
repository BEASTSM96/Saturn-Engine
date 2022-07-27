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

#include "Saturn/Serialisation/ProjectSerialiser.h"

namespace Saturn {
	
	static Ref<Project> s_ActiveProject;

	static std::vector< Ref< Project > > s_RecentProjects;

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
		return std::filesystem::path( GetActiveProject()->GetConfig().Path ) / "Assets";
	}

	const std::string& Project::GetName() const
	{
		return GetActiveProject()->GetConfig().Name;
	}
}