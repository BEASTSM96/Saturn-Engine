/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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
#include "FileCollection.h"

namespace Saturn {

	static std::vector<Ref<File>> s_Files;

	void FileCollection::AddFileToCollection( Ref<File>& file )
	{
		s_Files.push_back( file.Raw() );
	}

	void FileCollection::RemoveFileFromCollection( File* file )
	{
	}

	Ref<File> FileCollection::GetFile( std::string name )
	{
		for( size_t i = 0; i < s_Files.size(); i++ )
		{
			Ref<File> file = s_Files[ i ];
			if( file->GetName() == name )
				return file;
		}
		abort();
		return nullptr;
	}

	bool FileCollection::DoesFileExistInCollection( std::string name )
	{
		for( size_t i = 0; i < s_Files.size(); i++ )
		{
			Ref<File> file = s_Files[ i ];
			if( file->GetName() == name )
				return true;
		}
		return false;
	}

	int FileCollection::GetCollectionSize()
	{
		return s_Files.size();
	}

}
