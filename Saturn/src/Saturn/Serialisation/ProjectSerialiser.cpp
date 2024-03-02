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
#include "ProjectSerialiser.h"

#include "YamlAux.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

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
			out << YAML::Key << "Name" << YAML::Value << rProject->GetConfig().Name;
			out << YAML::Key << "AssetPath" << YAML::Value << rProject->GetAssetPath();
			out << YAML::Key << "StartupScene" << YAML::Value << rProject->GetConfig().StartupSceneID;

			out << YAML::Key << "ActionBindings";
			out << YAML::BeginSeq;

			for( const auto& rBinding : rProject->GetActionBindings() )
			{
				out << YAML::BeginMap;

				out << YAML::Key << "ActionBinding" << YAML::Value << rBinding.Name;
				
				out << YAML::Key << "ActionName" << YAML::Value << rBinding.ActionName;
				out << YAML::Key << "Key" << YAML::Value << (int)rBinding.Key;
				out << YAML::Key << "MouseButton" << YAML::Value << (int)rBinding.MouseButton;
				out << YAML::Key << "Type" << YAML::Value << (int)rBinding.Type;
				out << YAML::Key << "ID" << YAML::Value << (uint64_t)rBinding.ID;

				out << YAML::EndMap;
			}

			out << YAML::EndSeq;
		}

		out << YAML::EndMap;

		out << YAML::EndMap;

		std::ofstream file( rProject->GetConfig().Path );
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
		ProjectConfig& rConfig = newProject->GetConfig();

		{
			rConfig.Name      = project[ "Name" ].as<std::string>();
			rConfig.AssetPath = project[ "AssetPath" ].as<std::string>();
			rConfig.StartupSceneID = project[ "StartupScene" ].as<uint64_t>( 0 );

			auto actionBindings = project[ "ActionBindings" ];

			if( actionBindings ) 
			{
				for( const auto& binding : actionBindings )
				{
					ActionBinding ab;
					ab.Name = binding[ "ActionBinding" ].as<std::string>();
					ab.ActionName = binding[ "ActionName" ].as<std::string>();

					ab.Key = ( RubyKey ) binding[ "Key" ].as<int>( 0 );
					ab.MouseButton = ( RubyMouseButton ) binding[ "Key" ].as<int>( 6 );

					ab.Type = ( ActionBindingType ) binding[ "Type" ].as<int>( 0 );
					ab.ID = ( UUID ) binding[ "ID" ].as<uint64_t>();

					newProject->AddActionBinding( ab );
				}
			}
		}

		rConfig.Path = rFilePath;

		Project::SetActiveProject( newProject );
	}

}