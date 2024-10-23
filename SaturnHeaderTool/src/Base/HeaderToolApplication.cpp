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

#include "HeaderToolApplication.h"

#include <string>
#include <vector>

static std::vector<std::string> s_ArgumentsMap 
{
	"/SRC",
	"/OUT",
	"/MT",
	"/VERBOSE"
};

namespace Saturn {

	HeaderToolApplication::HeaderToolApplication( std::span<char*> args )
		: m_Args( args )
	{
		ValidateArgs();
	}

	HeaderToolApplication::~HeaderToolApplication()
	{
	}

	bool HeaderToolApplication::ValidateArgs()
	{
		// 2 args are needed, SourcePath, OutputPath
		if( m_Args.size() < 2 ) return false;

		// Start Parsing
		std::string outPath = m_Args[ 0 ];
		{
			std::string prefix = "/OUT=";
			auto pos = outPath.find( prefix );

			if( pos != std::string::npos )
			{
				std::string path = outPath.substr( pos + prefix.length() );

				m_SourcePath = path;
			}
			else
			{
				return false;
			}
		}

		std::string srcPath = m_Args[ 1 ];
		{
			std::string prefix = "/SRC=";
			auto pos = srcPath.find( prefix );

			if( pos != std::string::npos )
			{
				std::string path = srcPath.substr( pos + prefix.length() );

				m_OutputPath = path;
			}
			else
			{
				return false;
			}
		}

		m_HeaderTool.SetWorkingDir( m_OutputPath );

		return std::filesystem::exists( m_OutputPath ) && std::filesystem::exists( m_SourcePath );
	}

	void HeaderToolApplication::Run()
	{
		{
			m_SourcePath = "D:\\Saturn\\Projects\\Rush\\Source\\Rush";
			m_OutputPath = "D:\\Saturn\\Projects\\Rush\\Build";

			m_HeaderTool.SetWorkingDir( m_OutputPath );
		}

		std::vector<std::filesystem::path> headerFiles;

		// Search source path for all header files.
		for( const auto& rEntry : std::filesystem::recursive_directory_iterator( m_SourcePath ) )
		{
			if( rEntry.is_directory() ) continue;

			auto& rPath = rEntry.path();
			if( rPath.extension() == ".h" || rPath.extension() == ".hpp" )
				headerFiles.push_back( rPath );
		}

		m_HeaderTool.SubmitWorkList( headerFiles );

		m_HeaderTool.StartGeneration();
	}
}