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
#include "SceneSerialiser.h"

#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Components.h"
#include "Saturn/Vulkan/Mesh.h"

#include "YamlAux.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace Saturn {

	SceneSerialiser::SceneSerialiser( const Ref< Scene >& rScene )
		: m_Scene( rScene )
	{
	}

	SceneSerialiser::~SceneSerialiser()
	{

	}

	void SceneSerialiser::Serialise( const std::string& rFilePath )
	{
		YAML::Emitter out;
		
		out << YAML::BeginMap;

		out << YAML::Key << "Scene" << YAML::Value << "Untitled Scene";

		out << YAML::Key << "FilePath" << YAML::Value << rFilePath;

		m_Scene->m_Filepath = rFilePath;

		out << YAML::Key << "Entities";

		out << YAML::BeginSeq;
		
		m_Scene->Each( [&]( Ref<Entity> entity ) 
			{
				SerialiseEntity( out, entity );
			} );

		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		std::ofstream FileOut( rFilePath );
		FileOut << out.c_str();
	}

	void SceneSerialiser::Deserialise( const std::string& rFilePath )
	{
		std::ifstream FileIn( rFilePath );
		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );
		
		if( data.IsNull() )
			return;

		if( !data["Scene"] )
			return;

		std::string sceneName = data["Scene"].as< std::string >();
		SAT_CORE_INFO( "Deserialising scene '{0}'", sceneName );

		m_Scene->m_Filepath = rFilePath;

		auto entities = data["Entities"];

		DeserialiseEntities( entities, m_Scene );
	}

}