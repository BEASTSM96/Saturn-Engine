/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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

#pragma once

#include "Saturn/Core/Base.h"

#include <string>
#include <filesystem>

namespace Saturn {
	
	struct ProjectConfig
	{
		std::string Name;
		std::string StartupScenePath;

		std::string AssetPath; // Relative path
		std::string Path; // Absolute path
	};

	class Project : public CountedObj
	{
	public:
		Project();
		~Project();

		const ProjectConfig& GetConfig() const { return m_Config; }

		static Ref<Project> GetActiveProject();
		static void SetActiveProject( const Ref<Project>& rProject );

		// Only to be used by the Game.
		static std::string FindProjectDir( const std::string& rName );

		void CheckMissingAssetRefs();
		void LoadAssetRegistry();

		std::filesystem::path GetAssetPath();
		std::filesystem::path GetFullAssetPath();
		const std::string& GetName() const;
	
		std::filesystem::path GetPremakeFile();
		std::filesystem::path GetRootDir();

		std::filesystem::path GetBinDir();

		std::filesystem::path GetPath();

		std::filesystem::path FilepathAbs( const std::filesystem::path& rPath );

		bool HasPremakeFile();
		void CreatePremakeFile();
		void CreateBuildFile();

		void PrepForDist();

		// TEMP
		//    Until we have a proper project system
		ProjectConfig m_Config;
	};
}