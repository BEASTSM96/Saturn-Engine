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
#include "Premake.h"

#include "Saturn/Core/EnvironmentVariables.h"

namespace Saturn {

	bool Premake::Launch( const std::string& rWorkingDir )
	{
		std::string PremakePath = Auxiliary::GetEnvironmentVariable( "SATURN_PREMAKE_PATH" );

		STARTUPINFOA StartupInfo = {};
		StartupInfo.cb = sizeof( StartupInfo );
		StartupInfo.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
		StartupInfo.dwFlags = STARTF_USESTDHANDLES;

		PROCESS_INFORMATION ProcessInfo;

		std::replace( PremakePath.begin(), PremakePath.end(), '\\', '/' );

		PremakePath += " vs2022";

		bool res = CreateProcessA( nullptr, PremakePath.data(), nullptr, nullptr, FALSE, 0, nullptr, rWorkingDir.data(), &StartupInfo, &ProcessInfo );

		if( !res )
			SAT_CORE_ERROR( "Unable to start premake process" );

		WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

		CloseHandle( ProcessInfo.hThread );
		CloseHandle( ProcessInfo.hProcess );

		return res;
	}
}