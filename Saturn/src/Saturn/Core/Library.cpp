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
#include "Library.h"

namespace Saturn {

	Library::Library()
	{
	}

	Library::~Library()
	{
		Free();
	}

	bool Library::Load( const std::string& rPath )
	{
		bool result = false;

#if defined(_WIN32)
		m_Handle = LoadLibraryA( rPath.data() );
		result = ( bool ) m_Handle;
#else
		m_Handle = dlopen( rPath.data(), RTLD_LAZY );
		result = ( bool ) m_Handle;
#endif

		return result;
	}

	void Library::Free()
	{
		if( !m_Handle )
			return;

#if defined(_WIN32)
		FreeLibrary( m_Handle );
		m_Handle = nullptr;
#else
		dlclose( m_Handle );
		m_Handle = nullptr;
#endif
	}

#if defined(_WIN32)
	FARPROC Library::GetSymbol( const char* pName )
	{
		return GetProcAddress( m_Handle, pName );
	}
#else
	void* Library::GetSymbol( const char* pName )
	{
		return dlsym( m_Handle, pName );
	}
#endif

	void Library::SetExisting( LibraryHandle NewHandle )
	{
		Free();
		m_Handle = NewHandle;
	}

}