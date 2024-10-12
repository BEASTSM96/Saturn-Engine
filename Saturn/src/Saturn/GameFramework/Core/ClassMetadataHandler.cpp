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
#include "ClassMetadataHandler.h"

namespace Saturn {

	ClassMetadataHandler::ClassMetadataHandler()
	{
		constexpr size_t Classes = 3;
		m_Metadata.reserve( Classes );

		// Push some default classes.
		// TODO: This should be done by the Build Tool however we are not using it for the Engine.
		m_Metadata.emplace_back( "SClass", "", "", "", false );
		m_Metadata.emplace_back( "Entity", "SClass", "", "", false );
		m_Metadata.emplace_back( "Character", "Entity", "", "", false );

		ConstructTree();
	}

	ClassMetadataHandler::~ClassMetadataHandler()
	{
		m_Metadata.clear();
	}

	void ClassMetadataHandler::AddMetadata( const SClassMetadata& rData )
	{
		m_Metadata.push_back( rData );
		
		ConstructTree();
	}

	SClassMetadata& ClassMetadataHandler::GetSClassMetadata()
	{
		return m_MetadataTree.at( "SClass" );
	}

	void ClassMetadataHandler::RegisterProperty( const std::string& rMetadataName, const SProperty& rProperty )
	{
		const auto Itr = m_MetadataTree.find( rMetadataName );

		if( Itr != m_MetadataTree.end() )
		{
			m_Properties[ rMetadataName ].push_back( rProperty );
		}
	}

	std::vector<SProperty>& ClassMetadataHandler::GetAllProperties( const std::string& rMetadataName )
	{
		const auto Itr = m_MetadataTree.find( rMetadataName );

		if( Itr != m_MetadataTree.end() )
		{
			auto& properties = m_Properties[ rMetadataName ];
			return properties;
		}
	}

	void ClassMetadataHandler::ConstructTree()
	{
		for( const auto& data : m_Metadata )
		{
			m_MetadataTree[ data.Name ] = data;
		}
	}

}