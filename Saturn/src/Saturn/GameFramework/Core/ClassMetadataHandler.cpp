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
#include "ClassMetadataHandler.h"

namespace Saturn {

	ClassMetadataHandler::ClassMetadataHandler()
	{
		// Push some default classes.
		// TODO: This should be done by the Build Tool however we are not using it for the Engine.
		SClassMetadata SClassData{ .Name = "SClass", .ParentClassName = "" };
		m_Metadata.push_back( SClassData );

		SClassData = { .Name = "Entity", .ParentClassName = "SClass" };
		m_Metadata.push_back( SClassData );

		SClassData = { .Name = "Character", .ParentClassName = "Entity" };
		m_Metadata.push_back( SClassData );

		ConstructTree();
	}

	ClassMetadataHandler::~ClassMetadataHandler()
	{
		m_Metadata.clear();
	}

	void ClassMetadataHandler::Add( const SClassMetadata& rData )
	{
		m_Metadata.push_back( rData );
		
		ConstructTree();
	}

	void ClassMetadataHandler::ConstructTree()
	{
		for( const auto& data : m_Metadata )
		{
			m_MetadataTree[ data.Name ] = data;
		}
	}

}