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

#include "sppch.h"
#include "Project.h"

#include "Saturn/Serialisation/EngineSettingsSerialiser.h"
#include "Saturn/Serialisation/ProjectSerialiser.h"
#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/AssetManager.h"

#include "Saturn/Core/EngineSettings.h"
#include "Saturn/Core/EnvironmentVariables.h"
#include "Saturn/Core/StringAuxiliary.h"
#include "Saturn/Core/Process.h"

#include "SharedGlobals.h"

namespace Saturn {
	
	static const std::vector<std::string> s_AllowedAssetExtentions
	{
		{ ".png"          },
		{ ".tga"          },
		{ ".jpg"          },
		{ ".jpeg"         },
		{ ".hdr"          },
		{ ".msnd"         },
		{ ".gsnd"         },
		{ ".scene"        },
		{ ".smaterial"    },
		{ ".cpp"          },
		{ ".h"            },
		{ ".prefab"       },
		{ ".sphymaterial" }
	};

	Project::Project()
	{
	}

	Project::Project( const ProjectConfig& rConfig )
		: m_Config( rConfig )
	{
		m_RootPath = m_Config.Path.parent_path();
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
		s_ActiveProject = rProject;
	}

	std::filesystem::path Project::FindProjectDir( const std::string& rName )
	{
		std::string fullName = rName + ".sproject";
		std::filesystem::path res;

		for( auto& rEntry : std::filesystem::directory_iterator( std::filesystem::current_path() ) )
		{
			if( rEntry.is_directory() )
				continue;

			std::filesystem::path filepath = rEntry.path();

			if( filepath.extension() == ".sproject" )
			{
				if( filepath.filename() == fullName ) 
				{
					res = filepath;
					break;
				}
			}
		}

		return res;
	}

	void Project::CheckNewAssets()
	{
		bool FileChanged = false;
		auto AssetPath = GetFullAssetPath();

		for( auto& rEntry : std::filesystem::recursive_directory_iterator( AssetPath ) )
		{
			if( rEntry.is_directory() )
				continue;

			std::filesystem::path filepath = std::filesystem::relative( rEntry.path(), GetRootDir() );
			auto filepathString = filepath.extension().string();

			if( filepath.extension() == ".sreg" )
				continue;

			Ref<Asset> asset = AssetManager::Get().FindAsset( filepath );

			if( std::find( s_AllowedAssetExtentions.begin(), s_AllowedAssetExtentions.end(), filepathString ) == s_AllowedAssetExtentions.end() )
				continue; // Extension is forbidden.

			const auto& assetReg = AssetManager::Get().GetAssetRegistry()->GetAssetMap();
			if( asset == nullptr )
			{
				SAT_CORE_INFO( "Found an asset that exists in the system filesystem, however not in the asset registry, creating new asset." );

				// Add to pending file list.
				// Editor will show dialog and handle the rest.

				auto type = ExtensionToAssetType( filepathString );
				auto id = AssetManager::Get().CreateAsset( type );
				asset = AssetManager::Get().FindAsset( id );

				asset->SetAbsolutePath( rEntry.path() );

				FileChanged = true;
			}
		}

		if( FileChanged )
		{
			AssetRegistrySerialiser ars;
			ars.Serialise( AssetManager::Get().GetAssetRegistry() );
		}
	}

	void Project::CheckOfflineAssets()
	{
		bool FileChanged = false;

		auto& assetReg = AssetManager::Get().GetAssetRegistry()->GetAssetMap();
		for( auto& [id, rAsset] : assetReg )
		{
			if( !rAsset )
				continue;

			if( std::filesystem::exists( FilepathAbs( rAsset->Path ) ) )
				continue;

			SAT_CORE_WARN( "Found an asset that is present in the Asset Registry however no longer exists in the filesystem, removing from Asset Registry..." );

			AssetManager::Get().RemoveAsset( id );
		}

		if( FileChanged )
		{
			AssetRegistrySerialiser ars;
			ars.Serialise( AssetManager::Get().GetAssetRegistry() );
		}
	}

	void Project::CheckMissingAssetRefs()
	{
		CheckOfflineAssets();
		CheckNewAssets();
	}

	std::filesystem::path Project::GetAssetPath()
	{
		std::filesystem::path asset = GetActiveProject()->GetConfig().AssetPath;
		return std::filesystem::relative( asset, asset.parent_path() );
	}

	std::filesystem::path Project::GetFullAssetPath()
	{
		// Root dir
		std::filesystem::path rootDir = m_RootPath;
		rootDir /= "Assets";

		return rootDir;
	}

	std::filesystem::path Project::GetPremakeFile()
	{
		return GetAssetPath().parent_path() / "premake5.lua";
	}

	std::filesystem::path Project::GetRootDir()
	{
		return m_RootPath;
	}

	std::filesystem::path Project::GetTempDir()
	{
		return m_RootPath / "Temp";
	}

	std::filesystem::path Project::GetBinDir()
	{
		auto rootDir = GetRootDir();
		rootDir /= "bin";

#if defined(SAT_WINDOWS)

# if defined( SAT_DEBUG )
		rootDir /= "Debug-windows-x86_64";
#  elif defined( SAT_RELEASE )
		rootDir /= "Release-windows-x86_64";
#  else // SAT_DIST
		rootDir /= "Dist-windows-x86_64";
# endif
#else // SAT_WINDOWS

#endif

		rootDir /= m_Config.Name;

		return rootDir;
	}

	std::filesystem::path Project::GetProjectPath()
	{
		return GetActiveProject()->GetConfig().Path;
	}

	std::filesystem::path Project::FilepathAbs( const std::filesystem::path& rPath )
	{
		std::filesystem::path rootDir = m_RootPath;
		rootDir /= rPath;

		return rootDir;
	}

	std::filesystem::path Project::GetFullCachePath()
	{
		return m_RootPath / "Cache";
	}

	std::filesystem::path Project::GetAppDataFolder()
	{
		std::filesystem::path appData = Application::Get().GetAppDataFolder();

		return appData /= m_Config.Name;
	}

	void Project::RemoveActionBinding( const ActionBinding& rBinding )
	{
		m_ActionBindings.erase( std::remove( m_ActionBindings.begin(), m_ActionBindings.end(), rBinding ), m_ActionBindings.end() );
	}

	void Project::RemoveSoundGroup( const Ref<SoundGroup>& rGrp )
	{
		m_SoundGroups.erase( std::remove( m_SoundGroups.begin(), m_SoundGroups.end(), rGrp ), m_SoundGroups.end() );
	}

	void Project::UpgradeAssets()
	{
		// TODO: For now this function will just update the "Version" variable in the asset
		//       In the future we will want to actually upgrade the asset.

		AssetManager::Get().BumpAssetVersion( SAT_CURRENT_VERSION );
		
		AssetRegistrySerialiser ars;
		ars.Serialise( AssetManager::Get().GetAssetRegistry() );
	}

	bool Project::HasPremakeFile()
	{
		return std::filesystem::exists( GetAssetPath().parent_path() / "premake5.lua" );
	}

	void Project::CreatePremakeFile()
	{
		auto PremakePath = GetAssetPath().parent_path() / "premake5.lua";

		if( std::filesystem::exists( PremakePath ) )
			std::filesystem::remove( PremakePath );

		std::filesystem::copy( "content/Templates/premake5.lua", PremakePath );

		std::ifstream ifs( PremakePath );

		std::string fileData;

		if( ifs )
		{
			ifs.seekg( 0, std::ios_base::end );
			auto size = static_cast< size_t >( ifs.tellg() );
			ifs.seekg( 0, std::ios_base::beg );

			fileData.reserve( size );
			fileData.assign( std::istreambuf_iterator<char>( ifs ), std::istreambuf_iterator<char>() );
		}

		size_t pos = fileData.find( "__SATURN_BIN_DIR__" );

		while( pos != std::string::npos )
		{
			std::filesystem::path rootDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );
			rootDir /= "bin/";

#if defined( SAT_DEBUG )
			rootDir /= "Debug-windows-x86_64/Saturn";
#elif defined( SAT_RELEASE )
			rootDir /= "Release-windows-x86_64/Saturn";
#else
			rootDir /= "Dist-windows-x86_64/Saturn";
#endif

			auto rootDirString = rootDir.string();

			std::replace( rootDirString.begin(), rootDirString.end(), '\\', '/' );

			fileData.replace( pos, 18, rootDirString );

			pos = fileData.find( "__SATURN_BIN_DIR__" );
		}

		pos = 0;
		pos = fileData.find( "__PROJECT_NAME__" );

		while( pos != std::string::npos )
		{
			fileData.replace( pos, 16, m_Config.Name.c_str() );

			pos = fileData.find( "__PROJECT_NAME__" );
		}

		pos = 0;
		pos = fileData.find( "__SATURN_BT_DIR__" );

		while( pos != std::string::npos )
		{
			std::filesystem::path rootDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );
			rootDir /= "bin/";

#if defined( SAT_DEBUG )
			rootDir /= "Debug-windows-x86_64/SaturnBuildTool";
#elif defined( SAT_RELEASE )
			rootDir /= "Release-windows-x86_64/SaturnBuildTool";
#else
			rootDir /= "Dist-windows-x86_64/SaturnBuildTool";
#endif
			auto rootDirString = rootDir.string();
			std::replace( rootDirString.begin(), rootDirString.end(), '\\', '/' );

			fileData.replace( pos, 17, rootDirString );

			pos = fileData.find( "__SATURN_BT_DIR__" );
		}

		std::ofstream fout( PremakePath );
		fout << fileData;
	}

	void Project::CreateBuildFile()
	{
		auto BuildFilePath = GetRootDir() / "Source";
		BuildFilePath /= m_Config.Name + ".Build.cs";

		if( !std::filesystem::exists( BuildFilePath ) )
			std::filesystem::copy( "content/Templates/%PROJECT_NAME%.Build.cs", BuildFilePath );
	}

	void Project::PrepForDist()
	{
		// Copy over the runtime build file.
		auto BuildFilePath = GetRootDir() / "Source";
		BuildFilePath /= m_Config.Name + ".RT_Build.cs";

		if( !std::filesystem::exists( BuildFilePath ) )
			std::filesystem::copy( "content/Templates/%PROJECT_NAME%.RT_Build.cs", BuildFilePath );

		// Copy over the client main file
		auto BuildPath = GetFullAssetPath().parent_path() / "Build";
		BuildPath /= m_Config.Name + ".Entry.cpp";

		if( std::filesystem::exists( BuildPath ) )
			std::filesystem::remove( BuildPath );

		std::filesystem::create_directory( GetFullAssetPath().parent_path() / "Build" );

		std::filesystem::copy( "content/Templates/%PROJECT_NAME%.Entry.cpp", BuildPath );

		std::ifstream ifs( BuildPath );

		std::string fileData;

		if( ifs )
		{
			ifs.seekg( 0, std::ios_base::end );
			auto size = static_cast< size_t >( ifs.tellg() );
			ifs.seekg( 0, std::ios_base::beg );

			fileData.reserve( size );
			fileData.assign( std::istreambuf_iterator<char>( ifs ), std::istreambuf_iterator<char>() );
		}

		size_t pos = fileData.find( "%PROJECT_NAME%" );

		while( pos != std::string::npos )
		{
			std::string projectPath = m_Config.Name;
			std::replace( projectPath.begin(), projectPath.end(), '\\', '/' );

			fileData.replace( pos, 14, projectPath );

			pos = fileData.find( "%PROJECT_NAME%" );
		}

		std::ofstream fout( BuildPath );
		fout << fileData;
	}

	std::filesystem::path Project::FindBuildTool()
	{
		std::filesystem::path SaturnRootDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );
		std::filesystem::path BuildToolDir = SaturnRootDir;

		BuildToolDir /= "bin";

		// Should we make this be our current config OR should we set this to the target project config?
#if defined( SAT_DEBUG )
		BuildToolDir /= "Debug-windows-x86_64";
#elif defined( SAT_RELEASE )
		BuildToolDir /= "Release-windows-x86_64";
#else
		BuildToolDir /= "Dist-windows-x86_64";
#endif
		BuildToolDir /= "SaturnBuildTool";

#if defined( SAT_WINDOWS )
		BuildToolDir /= "SaturnBuildTool.exe";
#else
		BuildToolDir /= "SaturnBuildTool";
#endif

		return BuildToolDir;
	}

	bool Project::Build( ConfigKind kind )
	{
		std::filesystem::path BuildToolDir = FindBuildTool();

		std::filesystem::path WorkingDir = BuildToolDir;

		BuildToolDir /= "SaturnBuildTool.exe";

		std::wstring Args = BuildToolDir.wstring();

		Args += L" /BUILD ";
		
		Args += Auxiliary::ConvertString( m_Config.Name );
		
		Args += L" /Win64";
		
		switch( kind )
		{
			case Saturn::ConfigKind::Debug:
				Args += L" /Debug ";
				break;
		
			case Saturn::ConfigKind::Release:
				Args += L" /Release ";
				break;
			
			case Saturn::ConfigKind::Dist:
				Args += L" /Dist ";
				break;
		}

		Args += GetRootDir().wstring();

		// Start the process
		Process buildTool( Args, WorkingDir );

		int exitCode = buildTool.ResultOfProcess();

		return exitCode == 0;
	}

	bool Project::Rebuild( ConfigKind kind )
	{
		std::filesystem::path BuildToolDir = FindBuildTool();
		
		// parent_path to remove file name
		std::filesystem::path WorkingDir = BuildToolDir.parent_path();

		std::string Args = BuildToolDir.string();

		Args += " /REBUILD /";

		Args += m_Config.Name;

		Args += " /Win64";

		switch( kind )
		{
			case Saturn::ConfigKind::Debug:
				Args += " /Debug /";
				break;

			case Saturn::ConfigKind::Release:
				Args += " /Release /";
				break;

			case Saturn::ConfigKind::Dist:
				Args += " /Dist /";
				break;
		}

		Args += GetRootDir().string();
		std::wstring wArgs = Auxiliary::ConvertString( Args );

		// Start the process
		Process buildTool( wArgs, WorkingDir, ProcessCreateFlags::RedirectedStreams );
		int exitCode = buildTool.ResultOfProcess();

		return exitCode == 0;
	}

	void Project::Distribute( ConfigKind kind )
	{
		std::filesystem::path SaturnBinDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );
		std::filesystem::path binDir = GetRootDir();

		SaturnBinDir /= "bin";
		binDir /= "bin";
		
		switch( kind )
		{
			case Saturn::ConfigKind::Debug: 
				SaturnBinDir /= "Debug-windows-x86_64";
				binDir /= "Debug-windows-x86_64";
				break;

			case Saturn::ConfigKind::Release:
				SaturnBinDir /= "Release-windows-x86_64";
				binDir /= "Release-windows-x86_64";
				break;

			case Saturn::ConfigKind::Dist:
				SaturnBinDir /= "Dist-windows-x86_64";
				binDir /= "Dist-windows-x86_64";
				break;
		}

		// We use the editor because the editor will have the DLLs that we need to copy over.
		SaturnBinDir /= "Saturn-Editor";
		binDir /= m_Config.Name;

		for( const auto& rEntry : std::filesystem::directory_iterator( SaturnBinDir ) )
		{
			auto& path = rEntry.path();

			// TODO: Change for other platforms.
			if( path.extension() != ".dll" ) 
				continue;

			std::filesystem::path dstPath = binDir / path.filename();

			if( std::filesystem::exists( dstPath ) )
				std::filesystem::remove( dstPath );

			std::filesystem::copy_file( path, dstPath );
		}

		std::filesystem::path dstAssets = binDir / "Assets";
		std::filesystem::path dstCache = binDir / "Cache";

		// Copy our assets folder into the bin dir
		if( std::filesystem::exists( dstAssets ) )
			std::filesystem::remove_all( dstAssets );

		if( std::filesystem::exists( dstCache ) )
			std::filesystem::remove_all( dstCache );

		std::filesystem::copy( GetFullAssetPath(), dstAssets, std::filesystem::copy_options::recursive );
		std::filesystem::copy( GetRootDir() / "Cache", dstCache, std::filesystem::copy_options::recursive );

		// Copy the project file over
		std::string projectName = std::format( "{0}\\{1}.sproject", binDir.string(), m_Config.Name );

		if( std::filesystem::exists( projectName ) )
			std::filesystem::remove( projectName );

		std::filesystem::copy_file( m_Config.Path, projectName );
		
		// TEMP: Copy over the editor assets
		std::filesystem::path contentDir = Application::Get().GetRootContentDir().parent_path();
		
		if( std::filesystem::exists( binDir / "content" ) )
			std::filesystem::remove_all( binDir / "content" );

		std::filesystem::copy( contentDir, binDir / "content", std::filesystem::copy_options::recursive );
	}
}