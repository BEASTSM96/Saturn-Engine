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

#include "Saturn/Core/UserSettings.h"

#include "Saturn/Serialisation/UserSettingsSerialiser.h"
#include "Saturn/Serialisation/ProjectSerialiser.h"
#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include "Saturn/Asset/AssetRegistry.h"

#include "Saturn/Core/EnvironmentVariables.h"

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
		//SAT_CORE_ASSERT( rProject, "Project must be not be null!" );
		s_ActiveProject = rProject;
	}

	void Project::CheckMissingAssetRefs()
	{
		auto AssetPath = GetAssetPath();

		bool FileChanged = false;

		for( auto& rEntry : std::filesystem::recursive_directory_iterator( AssetPath ) ) 
		{
			if( rEntry.is_directory() )
				continue;

			std::filesystem::path filepath = rEntry.path();

			if( filepath.extension() == ".sreg" )
				continue;

			Ref<Asset> asset = AssetRegistry::Get().FindAsset( filepath );

			const auto& assetReg = AssetRegistry::Get().GetAssetMap();
			if( asset == nullptr ) 
			{
				// Search the asset asset registry
				//if( assetReg.at( asset->GetAssetID() )->GetPath() == filepath )
				//	continue;

				SAT_CORE_INFO( "Found an asset that exists in the system filesystem, however not in the asset registry, creating new asset." );

				auto id = AssetRegistry::Get().CreateAsset( AssetTypeFromExtension( filepath.extension().string() ) );
				asset = AssetRegistry::Get().FindAsset( id );

				asset->SetPath( filepath );

				FileChanged = true;
			}
		}

		if( FileChanged )
		{
			AssetRegistrySerialiser ars;
			ars.Serialise();
		}
	}

	void Project::LoadAssetRegistry()
	{
		AssetRegistrySerialiser ars;
		ars.Deserialise();
	}

	std::filesystem::path Project::GetAssetPath()
	{
		return GetActiveProject()->GetConfig().Path;
	}

	const std::string& Project::GetName() const
	{
		return GetActiveProject()->GetConfig().Name;
	}

	std::filesystem::path Project::GetPremakeFile()
	{
		return GetAssetPath().parent_path() / "premake5.lua";
	}

	std::filesystem::path Project::GetRootDir()
	{
		return GetAssetPath().parent_path();
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

		rootDir /= GetName();

		return rootDir;
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

		std::filesystem::copy( "assets/Templates/premake5.lua", PremakePath );

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

		size_t pos = fileData.find( "__SATURN_DIR__" );

		while( pos != std::string::npos )
		{
			std::filesystem::path rootDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );
			rootDir /= "Saturn/src/";

			auto rootDirString = rootDir.string();

			std::replace( rootDirString.begin(), rootDirString.end(), '\\', '/' );

			fileData.replace( pos, 14, rootDirString);

			pos = fileData.find( "__SATURN_DIR__" );
		}

		pos = fileData.find( "__SATURN_VENDOR__" );

		while( pos != std::string::npos )
		{
			std::filesystem::path rootDir = Auxiliary::GetEnvironmentVariable( "SATURN_DIR" );
			rootDir /= "Saturn/vendor/";

			auto rootDirString = rootDir.string();

			std::replace( rootDirString.begin(), rootDirString.end(), '\\', '/' );

			fileData.replace( pos, 17, rootDirString );

			pos = fileData.find( "__SATURN_VENDOR__" );
		}

		pos = fileData.find( "__SATURN_BIN_DIR__" );

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
			fileData.replace( pos, 16, GetName().c_str() );

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
		auto BuildFilePath = GetAssetPath().parent_path() / "Scripts";
		BuildFilePath /= GetName() + ".Build.cs";

		if( !std::filesystem::exists( BuildFilePath ) )
			std::filesystem::copy( "assets/Templates/%PROJECT_NAME%.Build.cs", BuildFilePath );
	}

}