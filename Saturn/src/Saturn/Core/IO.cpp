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
#include "IO.h"

#if defined ( SAT_WINDOWS )
#define freopen freopen_s
#endif

namespace Saturn {

	void IO::Init()
	{
		//m_OldStdStringBuffer = std::cout.rdbuf();
	}

	void IO::Shutdown()
	{
		//std::cout.rdbuf( m_OldStdStringBuffer );
	}

	void IO::StdStreamRedirect()
	{
		//ConsoleStreamRedirect();

		//std::cout.rdbuf( &m_StdStringBuffer );
	}

	void IO::ConsoleStreamRedirect()
	{
	#if defined( SAT_WINDOWS )

		bool res = true;
		FILE* fp;

		if( GetStdHandle( STD_INPUT_HANDLE ) != INVALID_HANDLE_VALUE )
			if( freopen( &fp, "CONIN$", "r", stdin ) != 0 )
				res = false;
			else
				setvbuf( stdin, NULL, _IONBF, 0 );

		if( GetStdHandle( STD_OUTPUT_HANDLE ) != INVALID_HANDLE_VALUE )
			if( freopen( &fp, "CONOUT$", "w", stdout ) != 0 )
				res = false;
			else
				setvbuf( stdout, NULL, _IONBF, 0 );

		if( GetStdHandle( STD_ERROR_HANDLE ) != INVALID_HANDLE_VALUE )
			if( freopen( &fp, "CONOUT$", "w", stderr ) != 0 )
				res = false;
			else
				setvbuf( stderr, NULL, _IONBF, 0 );

		std::ios::sync_with_stdio( true );

		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();
	#endif
	}

}