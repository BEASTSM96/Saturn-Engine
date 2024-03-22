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
#include "EngineSettingsSerialiser.h"

#include <fstream>
#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace YAML {

	template <>
	struct convert<std::filesystem::path> 
	{
		static Node encode( std::filesystem::path rhs ) 
		{ 
			return Node( rhs.string() ); 
		}

		static bool decode( const Node& node, std::filesystem::path& rhs )
		{
			rhs = node.as<std::string>();
			
			return true;
		}
	};

}

namespace Saturn {

	EngineSettingsSerialiser::EngineSettingsSerialiser()
	{
	}

	EngineSettingsSerialiser::~EngineSettingsSerialiser()
	{
	}

	void EngineSettingsSerialiser::Serialise()
	{
		auto AppDataPath = Application::Get().GetAppDataFolder();
		auto& rSettings = EngineSettings::Get();

		YAML::Emitter out;

		out << YAML::BeginMap;

		if( !rSettings.StartupProject.empty() )
			out << YAML::Key << "Startup Project" << YAML::Value << rSettings.StartupProject.string();
		
		out << YAML::Key << "Recent Projects";
		
		out << YAML::BeginSeq;
		
		for( auto& rPath : rSettings.RecentProjects )
		{
			auto p = rPath.string();

			std::replace( p.begin(), p.end(), '\\', '/' );

			out << YAML::Key << YAML::Value << p;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		auto userSettingsPath = AppDataPath / "EngineSettings.yaml";

		std::ofstream file( userSettingsPath );
		file << out.c_str();
	}

	void EngineSettingsSerialiser::Deserialise()
	{
		auto AppDataPath = Application::Get().GetAppDataFolder();
		
		auto userSettingsPath = AppDataPath / "EngineSettings.yaml";

		std::ifstream FileIn( userSettingsPath );
		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;

		auto p = data[ "Recent Projects" ];
		auto startup = data[ "Startup Project" ].as<std::string>( "" );

		auto& rSettings = EngineSettings::Get();

		if( !startup.empty() )
		{
			size_t found = startup.find_last_of( "/\\" );
			rSettings.StartupProjectName = startup.substr( found + 1 );

			rSettings.StartupProject = startup;
			rSettings.FullStartupProjPath = fmt::format( "{0}\\{1}.sproject", startup, rSettings.StartupProjectName );
		}

		for ( auto project : p )
		{
			auto path = project.as<std::filesystem::path>();
			
			if( std::filesystem::exists( path ) )
				rSettings.RecentProjects.push_back( path );
		}
	}

}