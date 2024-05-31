/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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
#include "Saturn/GameFramework/ActionBinding.h"
#include "Saturn/Audio/SoundGroup.h"

#include "Saturn/Core/UUID.h"

#include <string>
#include <filesystem>

namespace Saturn {
	
	struct ProjectConfig
	{
		std::string Name;
		UUID StartupSceneID;

		// Relative path
		// "Assets"
		std::string AssetPath; 
		
		// Path to the .sproject file
		std::filesystem::path Path;
	};

	enum class ConfigKind
	{
		Debug,
		Release,
		Dist
	};

	class Project : public RefTarget
	{
	public:
		Project();
		Project( const ProjectConfig& rConfig );
		~Project();

		ProjectConfig& GetConfig() { return m_Config; }
		static ProjectConfig& GetActiveConfig() { return s_ActiveProject->m_Config; }

		// Local per module (copied to game when running in editor.)
		static Ref<Project> GetActiveProject();
		static void SetActiveProject( const Ref<Project>& rProject );

		// Only to be used by the Game.
		static std::string FindProjectDir( const std::string& rName );

		void CheckMissingAssetRefs();

	public:
		//////////////////////////////////////////////////////////////////////////
		// File and Project folder helpers.

		std::filesystem::path FilepathAbs( const std::filesystem::path& rPath );

		// Relative Asset Path
		std::filesystem::path GetAssetPath();
		
		// Absolute Asset Path
		std::filesystem::path GetFullAssetPath();
		std::filesystem::path GetAbsoluteAssetPath() { return GetFullAssetPath(); }
	
		// Relative Premake file
		std::filesystem::path GetPremakeFile();
		
		// Absolute project dir
		std::filesystem::path GetRootDir();
		
		// Absolute Temp dir
		std::filesystem::path GetTempDir();

		// Absolute bin dir
		std::filesystem::path GetBinDir();
		static std::filesystem::path GetActiveBinDir() { return s_ActiveProject->GetBinDir(); }

		// Absolute project dir (uses the project config's "Path" variable)
		std::filesystem::path GetProjectPath();
		static std::filesystem::path GetActiveProjectPath() { return s_ActiveProject->GetProjectPath(); }

		static std::filesystem::path GetActiveProjectRootPath() { return s_ActiveProject->GetRootDir(); }

		// Absolute cache dir
		std::filesystem::path GetFullCachePath();

	public:
		//////////////////////////////////////////////////////////////////////////
		// Action Bindings

		std::vector<ActionBinding>& GetActionBindings() { return m_ActionBindings; }
		const std::vector<ActionBinding>& GetActionBindings() const { return m_ActionBindings; }
		
		void AddActionBinding( const ActionBinding& rBinding ) { m_ActionBindings.push_back( rBinding ); }
		void RemoveActionBinding( const ActionBinding& rBinding );

		//////////////////////////////////////////////////////////////////////////
		// Sound Group

		std::vector<Ref<SoundGroup>>& GetSoundGroups() { return m_SoundGroups; }
		const std::vector<Ref<SoundGroup>>& GetSoundGroups() const { return m_SoundGroups; }

		void AddSoundGroup( const Ref<SoundGroup>& rGrp ) { m_SoundGroups.push_back( rGrp ); }
		void RemoveSoundGroup( const Ref<SoundGroup>& rGrp );

	public:
		//////////////////////////////////////////////////////////////////////////
		// Premake, Building & Preparation for Distribution (Used in Editor)

		bool HasPremakeFile();
		void CreatePremakeFile();
		void CreateBuildFile();

		std::filesystem::path FindBuildTool();

		bool Build( ConfigKind kind );
		bool Rebuild( ConfigKind kind );
		void Distribute( ConfigKind kind );

		void PrepForDist();

	private:
		void CheckNewAssets();
		void CheckOfflineAssets();

	private:
		ProjectConfig m_Config;
		std::vector<ActionBinding> m_ActionBindings;
		std::vector<Ref<SoundGroup>> m_SoundGroups;

		// Absolute root path
		std::filesystem::path m_RootPath;

	private:
		inline static Ref<Project> s_ActiveProject;
	};
}