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

#include "sppch.h"
#include "Project.h"

#include "Saturn/Core/UserSettings.h"

#include "Saturn/Serialisation/UserSettingsSerialiser.h"
#include "Saturn/Serialisation/ProjectSerialiser.h"
#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/AssetManager.h"

#include "Saturn/Core/EnvironmentVariables.h"

#include "SharedGlobals.h"

namespace Saturn {
	
	static std::vector<std::string> s_DisallowedAssetExtensions
	{
		{ ".fbx" },  // Already in the static mesh asset
		{ ".gltf" }, // Already in the static mesh asset
		{ ".bin" },  // Already in the static mesh asset
		{ ".glb" },  // Already in the static mesh asset
		{ ".wav" },  // Already in the sound 2d asset
	};

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
		s_ActiveProject = rProject;
	}

	std::string Project::FindProjectDir( const std::string& rName )
	{
		std::string fullName = rName + ".sproject";
		std::string res = "";

		for( auto& rEntry : std::filesystem::recursive_directory_iterator( std::filesystem::current_path() ) )
		{
			if( rEntry.is_directory() )
				continue;

			std::filesystem::path filepath = rEntry.path();

			if( filepath.extension() == ".sproject" )
			{
				if( filepath.filename() == fullName ) 
				{
					res = filepath.string();

					break;
				}
			}
		}

		return res;
	}

	void Project::CheckMissingAssetRefs()
	{
		auto AssetPath = GetFullAssetPath();

		bool FileChanged = false;

		for( auto& rEntry : std::filesystem::recursive_directory_iterator( AssetPath ) ) 
		{
			if( rEntry.is_directory() )
				continue;

			std::filesystem::path filepath = std::filesystem::relative( rEntry.path(), GetRootDir() );
			auto filepathString = filepath.extension().string();

			if( filepath.extension() == ".sreg" )
				continue;

			Ref<Asset> asset = AssetManager::Get().FindAsset( filepath );

			if( std::find( s_DisallowedAssetExtensions.begin(), s_DisallowedAssetExtensions.end(), filepathString ) != s_DisallowedAssetExtensions.end() )
				continue; // Extension is forbidden.

			const auto& assetReg = AssetManager::Get().GetAssetRegistry()->GetAssetMap();
			if( asset == nullptr ) 
			{
				SAT_CORE_INFO( "Found an asset that exists in the system filesystem, however not in the asset registry, creating new asset." );

				auto type = AssetTypeFromExtension( filepathString );
				auto id = AssetManager::Get().CreateAsset( type );
				asset = AssetManager::Get().FindAsset( id );

				asset->SetPath( rEntry.path() );

				FileChanged = true;
			}
		}

		if( FileChanged )
		{
			AssetRegistrySerialiser ars;
			ars.Serialise( AssetManager::Get().GetAssetRegistry() );
		}
	}

	std::filesystem::path Project::GetAssetPath()
	{
		std::filesystem::path asset = GetActiveProject()->GetConfig().AssetPath;
		return std::filesystem::relative( asset, asset.parent_path() );
	}

	std::filesystem::path Project::GetFullAssetPath()
	{
		// Root dir
		auto rootDir = std::filesystem::path( GetActiveProject()->GetConfig().Path ).parent_path();
		rootDir /= "Assets";

		return rootDir;
	}

	std::filesystem::path Project::GetPremakeFile()
	{
		return GetAssetPath().parent_path() / "premake5.lua";
	}

	std::filesystem::path Project::GetRootDir()
	{
		return GetFullAssetPath().parent_path();
	}

	std::filesystem::path Project::GetBinDir()
	{
		auto rootDir = GetRootDir();
		rootDir /= "bin";

#if defined( SAT_DEBUG )
		rootDir /= "Debug-windows-x86_64";
#elif defined( SAT_RELEASE )
		rootDir /= "Release-windows-x86_64";
#else // SAT_DIST
		rootDir /= "Dist-windows-x86_64";
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
		auto rootDir = std::filesystem::path( GetActiveProject()->GetConfig().Path ).parent_path();
		rootDir /= rPath;

		return rootDir;
	}

	std::filesystem::path Project::GetFullCachePath()
	{
		return GetRootDir() / "Cache";
	}

	void Project::RemoveActionBinding( const ActionBinding& rBinding )
	{
		//m_ActionBindings.erase( std::remove( m_ActionBindings.begin(), m_ActionBindings.end(), rBinding ), m_ActionBindings.end() );
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
		BuildPath /= m_Config.Name + "Main.cpp";

		if( std::filesystem::exists( BuildPath ) )
			std::filesystem::remove( BuildPath );

		std::filesystem::create_directory( GetFullAssetPath().parent_path() / "Build" );

		std::filesystem::copy( "content/Templates/%PROJECT_NAME%Main.cpp", BuildPath );

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

}