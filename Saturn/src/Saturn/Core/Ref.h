/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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

#include <type_traits>

namespace Saturn {

	template<typename T>
	class Ref
	{
	public:
		Ref() : m_Pointer( nullptr ) {}
		Ref( std::nullptr_t nullptrr ) : m_Pointer( nullptr ) {}
		Ref( T* pointer ) : m_Pointer( pointer ) {}

		template<typename T2>
		Ref( const Ref<T2>& other ) { m_Pointer = ( T* )other.m_Pointer; }

		Ref( const Ref<T>& other ) { m_Pointer = ( T* )other.m_Pointer; }

		template<typename T2>
		Ref( Ref<T2>&& other )
		{
			m_Pointer = ( T* )other.m_Pointer;  
			other.m_Pointer = nullptr;
		}

		~Ref() 
		{ 
			/*
			if( m_Pointer ) 
				delete m_Pointer; 

			m_Pointer = nullptr; 
			*/
		}

		void Delete() 
		{
			delete m_Pointer;
			m_Pointer = nullptr;
		}

		void Reset()
		{
			m_Pointer = nullptr;
		}

		template<typename... VaArgs>
		static Ref<T> Create( VaArgs&&... args )
		{
			return Ref<T>( new T( std::forward<VaArgs>( args )... ) );
		}

	public:
	
		Ref& operator=( std::nullptr_t ) 
		{
			delete m_Pointer;
			m_Pointer = nullptr;
			return *this;
		}

		//////////////////////////////////////////////////////////////////////////

		Ref& operator=( Ref<T>& other )
		{
			//delete m_Pointer;
			//m_Pointer = nullptr;

			m_Pointer = other.m_Pointer;

			return *this;
		}

		template<typename T2>
		Ref& operator=( Ref<T2>& other )
		{
			//delete m_Pointer;
			//m_Pointer = nullptr;

			m_Pointer = other.m_Pointer;

			return *this;
		}

		//////////////////////////////////////////////////////////////////////////
		// CONST
		//////////////////////////////////////////////////////////////////////////

		Ref& operator=( const Ref<T>& other )
		{
			//if( m_Pointer )
			//	delete m_Pointer;

			m_Pointer = other.m_Pointer;

			return *this;
		}

		template<typename T2>
		Ref& operator=( const Ref<T2>& other )
		{
			if( m_Pointer )
				delete m_Pointer;

			m_Pointer = other.m_Pointer;

			return *this;
		}

		//////////////////////////////////////////////////////////////////////////

		template<typename T2>
		Ref& operator=( Ref<T2>&& other )
		{
			delete m_Pointer;
			m_Pointer = nullptr;

			m_Pointer = other.m_Pointer;
			other.m_Pointer = nullptr;

			return *this;
		}

		//////////////////////////////////////////////////////////////////////////

		operator bool()       { return m_Pointer != nullptr; }
		operator bool() const { return m_Pointer != nullptr; }

		T* operator->()             { return m_Pointer; }
		const T* operator->() const { return m_Pointer; }

		T& operator*()             { return *m_Pointer; }
		const T& operator*() const { return *m_Pointer; }

		T* Pointer()             { return m_Pointer; }
		const T* Pointer() const { return m_Pointer; }

	public:

		//T* Null = nullptr;

	private:

		T* m_Pointer;

	private:
		// Fix cannot access private member declared in class
		friend class Ref;
	};

}