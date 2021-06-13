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

#include <stdint.h>

namespace Saturn {

#ifdef SAT_PLATFORM_LINUX
	class RefCounted
	{
	public:
		void IncRefCount( void )  const
		{
			m_RefCount++;
		}

		void DecRefCount( void )  const
		{
			m_RefCount--;
		}

		uint32_t GetRefCount() const { return m_RefCount; }
	private:
		mutable uint32_t m_RefCount = 0; // TODO: atomic
	};

#endif

	class RefCounter
	{
	public:
		virtual void IncRefCount( void )  const = 0;
		virtual void DecRefCount( void )  const = 0;
		virtual void SetRefCount( uint32_t Count )  const = 0;
		virtual uint32_t GetRefCount( void )  const = 0;
	};

	class RefCounted : public RefCounter
	{
	public:
		void IncRefCount( void )  const override
		{
			m_RefCount++;
		}

		void DecRefCount( void )  const override
		{
			m_RefCount--;
		}

		void SetRefCount( uint32_t Count ) const override
		{
			if( GetRefCount() != 0 )
			{
				m_RefCount = Count;
			}
			else
				m_RefCount = 0;
		}

		uint32_t GetRefCount() const override { return m_RefCount; }
	private:
		mutable uint32_t m_RefCount = 0; // TODO: atomic
	};

	template<typename T>
	class Ref
	{
	public:
		Ref()
			: m_Instance( nullptr )
		{
			static_assert( std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!" );
		}

		Ref( std::nullptr_t n )
			: m_Instance( nullptr )
		{
		}

		Ref( T* instance )
			: m_Instance( instance )
		{
			static_assert( std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!" );

			IncRef();
		}

		template<typename T2>
		Ref( const Ref<T2>& other )
		{
			m_Instance = ( T* )other.m_Instance;
			IncRef();
		}

		template<typename T2>
		Ref( Ref<T2>&& other )
		{
			m_Instance = ( T* )other.m_Instance;
			other.m_Instance = nullptr;
		}

		~Ref()
		{
			DecRef();
		}

		Ref( const Ref<T>& other )
			: m_Instance( other.m_Instance )
		{
			IncRef();
		}

		Ref& operator=( std::nullptr_t )
		{
			DecRef();
			m_Instance = nullptr;
			return *this;
		}

		Ref& operator=( const Ref<T>& other )
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=( const Ref<T2>& other )
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=( Ref<T2>&& other )
		{
			DecRef();

			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		operator bool() { return m_Instance != nullptr; }
		operator bool() const { return m_Instance != nullptr; }

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		T* Raw() { return  m_Instance; }
		const T* Raw() const { return  m_Instance; }

		void Reset( T* instance = nullptr )
		{
			DecRef();
			m_Instance = instance;
		}

		void Destory()
		{
			m_Instance->SetRefCount( 0 );
		}

		template<typename... Args>
		static Ref<T> Create( Args&&... args )
		{
		#ifdef SAT_PLATFORM_LINUX
			return Ref<T>( new T( std::move<Args>( args )... ) );
		#endif

		#ifdef SAT_PLATFORM_WINDOWS
			return Ref<T>( new T( std::forward<Args>( args )... ) );
		#endif
		}
	private:
		void IncRef( void )  const
		{
			static_assert( std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!" );

			if( m_Instance )
				m_Instance->IncRefCount();
		}

		void DecRef( void )  const
		{

			static_assert( std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!" );

			if( m_Instance )
			{
				m_Instance->DecRefCount();
				if( m_Instance->GetRefCount() == 0 )
				{
					delete m_Instance;
				}
			}
		}

		template<class T2>
		friend class Ref;
		T* m_Instance;
	};

}
