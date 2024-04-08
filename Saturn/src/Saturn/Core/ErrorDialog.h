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

#pragma once

#if defined( SAT_PLATFORM_WINDOWS )
#include <Windows.h>
#endif

#include <string>

namespace Saturn::Core {

	inline int ShowErrorDialogBox( const std::string& rTitle, const std::string& rText ) 
	{
#if defined(SAT_PLATFORM_WINDOWS)
		return MessageBoxA( nullptr, rText.data(), rTitle.data(), MB_ICONSTOP | MB_OK );
#else
		return 0;
#endif
	}

	[[noreturn]] inline void ShowErrorDialogBox( const std::string& rTitle, const std::string& rText, bool Terminate )
	{
#if defined(SAT_PLATFORM_WINDOWS)
		MessageBoxA( nullptr, rText.data(), rTitle.data(), MB_ICONSTOP | MB_OK );
#else
#endif
		std::exit( 1 );
		std::unreachable();
	}
}