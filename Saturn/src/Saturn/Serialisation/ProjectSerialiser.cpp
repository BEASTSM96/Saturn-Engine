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
#include "ProjectSerialiser.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

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

	inline Emitter& operator<<( Emitter& emitter, const std::filesystem::path& v ) 
	{
		return emitter.Write( v.string() );
	}
}

namespace Saturn {

	ProjectSerialiser::ProjectSerialiser( const Ref< Project >& rProject )
		: m_Project( rProject )
	{

	}

	ProjectSerialiser::~ProjectSerialiser()
	{
	}

	void ProjectSerialiser::Serialise( const std::string& rFilePath )
	{
		Ref<Project> rProject = Project::GetActiveProject();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "Project" << YAML::Value;

		out << YAML::BeginMap;

		{
			out << YAML::Key << "Name" << YAML::Value << rProject->GetName();
			out << YAML::Key << "AssetPath" << YAML::Value << rProject->GetAssetPath();
			out << YAML::Key << "AssetBase" << YAML::Value << "Assets"; // TODO
			out << YAML::Key << "StartupScene" << YAML::Value << rProject->m_Config.StartupScenePath;
		}

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream file( rFilePath + ".sproject" );
		file << out.c_str();
	}

	void ProjectSerialiser::Deserialise( const std::string& rFilePath )
	{
		std::ifstream FileIn( rFilePath );

		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;	

		auto project = data[ "Project" ];

		Ref<Project> newProject = Ref<Project>::Create();
		{
			newProject->m_Config.Name = project[ "Name" ].as<std::string>();
			newProject->m_Config.Path = project[ "AssetPath" ].as<std::string>();
			newProject->m_Config.StartupScenePath = project[ "StartupScene" ].as<std::string>();
		}

		Project::SetActiveProject( newProject );
	}

}