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

#pragma once

#include "SingletonStorage.h"
#include "Saturn/GameFramework/SClass.h"

#include <string>
#include <vector>

namespace Saturn {

	class ClassMetadataHandler : public RefTarget
	{
	public:
		static inline ClassMetadataHandler& Get() { return *SingletonStorage::Get().GetOrCreateSingleton<ClassMetadataHandler>(); }
	public:
		ClassMetadataHandler();
		~ClassMetadataHandler();

		template<typename Fn> 
		void Each( Fn Function ) 
		{
			for( const auto& data : m_Metadata )
				Function( data );
		}

		template<typename Fn>
		void EachTreeNode( Fn Function )
		{
			for( auto&& [name, data] : m_MetadataTree )
				Function( data );
		}

		void Add( const SClassMetadata& rData );

	private:
		void ConstructTree();

	private:
		std::vector<SClassMetadata> m_Metadata;
		std::unordered_map<std::string, SClassMetadata> m_MetadataTree;
	};
}