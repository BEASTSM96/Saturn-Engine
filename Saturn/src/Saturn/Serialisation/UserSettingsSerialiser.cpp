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
#include "UserSettingsSerialiser.h"

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

	UserSettingsSerialiser::UserSettingsSerialiser()
	{
	}

	UserSettingsSerialiser::~UserSettingsSerialiser()
	{
	}

	void UserSettingsSerialiser::Serialise( const UserSettings& rSettings )
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		//out << YAML::Key << "Startup Project" << YAML::Value << rSettings.StartupProject;
		
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

		std::ofstream file( "assets/UserSettings.yaml" );
		file << out.c_str();
	}

	void UserSettingsSerialiser::Deserialise( UserSettings& rSettings )
	{
		std::string FilePath = "assets/UserSettings.yaml";

		std::ifstream FileIn( FilePath );
		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;

		auto p = data[ "Recent Projects" ];

		for ( auto project : p )
		{
			auto path = project.as<std::filesystem::path>();
			
			rSettings.RecentProjects.push_back( path );
		}
	}

}