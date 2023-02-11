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

#include "Base.h"

#include <sstream>
#include <iostream>

#if defined( SAT_WINDOWS )
#include <Windows.h>
#endif

namespace Saturn {

	using CharBuffer = std::basic_stringbuf<std::ostream::char_type>;

	class IO
	{
		SINGLETON( IO );

		IO() {}
		~IO() {}

	public:

		void Init();

		void Shutdown();

		void StdStreamRedirect();

		void ConsoleStreamRedirect();

		CharBuffer& StdStreamBuffer() { return m_StdStringBuffer; }
		const CharBuffer& StdStreamBuffer() const { return m_StdStringBuffer; }

	protected:
	private:

		CharBuffer m_StdStringBuffer;
		std::streambuf* m_OldStdStringBuffer;

		CharBuffer m_CStringBuffer;

		wchar_t m_ConsoleStringBuffer[ 2 ];
	};

}