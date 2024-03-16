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
#include "SceneSerialiser.h"

#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Components.h"
#include "Saturn/Vulkan/Mesh.h"

#include "YamlAux.h"

#include <fstream>

#include <yaml-cpp/yaml.h>
#include "yaml-cpp/node/node.h"

namespace Saturn {

	static std::filesystem::path GetFilepathAbs( const std::filesystem::path& rPath, bool IsEditorAsset )
	{
		if( IsEditorAsset )
		{
			std::filesystem::path basePath = Application::Get().GetRootContentDir();
			basePath = basePath.parent_path();
			basePath = basePath.parent_path();
			basePath /= rPath;

			return basePath;
		}
		else
		{
			return Project::GetActiveProject()->FilepathAbs( rPath );
		}
	}

	SceneSerialiser::SceneSerialiser( const Ref< Scene >& rScene )
		: m_Scene( rScene )
	{
	}

	SceneSerialiser::~SceneSerialiser()
	{
		m_Scene = nullptr;
	}

	void SceneSerialiser::Serialise()
	{
		auto& basePath = m_Scene->GetPath();
		auto fullPath = GetFilepathAbs( basePath, m_Scene->IsFlagSet( AssetFlag::Editor ) );

		YAML::Emitter out;
		
		out << YAML::BeginMap;

		out << YAML::Key << "Scene" << YAML::Value << "Untitled Scene";

		out << YAML::Key << "Entities";

		out << YAML::BeginSeq;
		
		m_Scene->Each( [&]( Ref<Entity> entity ) 
			{
				SerialiseEntity( out, entity );
			} );

		out << YAML::EndSeq;
		out << YAML::EndMap;
		
		std::ofstream FileOut( fullPath );
		FileOut << out.c_str();
	}

	void SceneSerialiser::Deserialise()
	{
		auto& basePath = m_Scene->GetPath();	
		Deserialise( basePath );
	}

	void SceneSerialiser::Deserialise( const std::filesystem::path& rPath )
	{
		auto fullPath = GetFilepathAbs( rPath, m_Scene->IsFlagSet( AssetFlag::Editor ) );

		std::ifstream FileIn( fullPath );
		std::stringstream ss;
		ss << FileIn.rdbuf();

		YAML::Node data = YAML::Load( ss.str() );

		if( data.IsNull() )
			return;

		if( !data[ "Scene" ] )
			return;

		SAT_CORE_INFO( "Deserialising scene '{0}'", m_Scene->Name );

		auto entities = data[ "Entities" ];
		DeserialiseEntities( entities, m_Scene );

		FileIn.close();
	}

}