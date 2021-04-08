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

#pragma once

typedef void __declspec( dllimport ) ( *func )( );

namespace Saturn {

	class ScriptLoader;

	class Library : public RefCounted
	{
	public:
		Library();

		std::string& GetName() { return m_Name; }
		std::string& GetPath() { return m_Path; }

		const std::string& GetName() const { return m_Name; }
		const std::string& GetPath() const { return m_Path; }

		void SetName( std::string name ) { m_Name = name; }

		template<typename T>
		void CallFunction( std::string name )
		{
			if( this == nullptr )
				return;

			if( m_Library == NULL )
				return;

			void ( *cfunc )( ) = ( func )GetProcAddress( m_Library, ( LPCSTR )name.c_str() );

			cfunc();
		}

	private:
		std::string m_Name;
		std::string m_Path;

		HINSTANCE m_Library;

	private:
		friend class ScriptLoader;
	};
}