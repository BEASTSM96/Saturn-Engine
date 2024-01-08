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
#include "Process.h"

namespace Saturn {

	Process::Process( const std::wstring& rCommandLine, const std::wstring& rWorkingDir )
		: m_CommandLine( rCommandLine )
	{
		Create( rWorkingDir );
	}

	Process::~Process()
	{
		Terminate();
	}

	void Process::Create( const std::wstring& rWorkingDir )
	{
#if defined( _WIN32 )
		STARTUPINFOW StartupInfo = {};
		StartupInfo.cb = sizeof( StartupInfo );
		StartupInfo.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
		StartupInfo.dwFlags = STARTF_USESTDHANDLES;

		PROCESS_INFORMATION ProcessInfo;
		bool result = CreateProcessW( 
			nullptr, m_CommandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, 
			rWorkingDir.empty() ? nullptr : rWorkingDir.data(), &StartupInfo, &ProcessInfo );

		CloseHandle( ProcessInfo.hThread );
		m_Handle = ProcessInfo.hProcess;
#endif
	}

	void Process::Terminate()
	{
#if defined( _WIN32 )
		if( m_Handle ) 
		{
			TerminateProcess( m_Handle, 0 );

			m_Handle = nullptr;
		}
#endif
	}

	void Process::WaitForExit()
	{
#if defined( _WIN32 )
		bool Result;
		DWORD ExitCode;

		while( Result = GetExitCodeProcess( m_Handle, &ExitCode ) && ExitCode == STATUS_PENDING )
		{
			Sleep( 1 );
		}

		// Process exited somehow... cleanup.
		CloseHandle( m_Handle );

		m_Handle = nullptr;
		m_ExitCode = ExitCode;
#endif
	}

	int Process::ResultOfProcess()
	{
		WaitForExit();

		return m_ExitCode;
	}

}