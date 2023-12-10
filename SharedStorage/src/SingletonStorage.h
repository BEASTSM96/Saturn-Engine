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

#include <typeindex>
#include <unordered_map>
#include <thread>
#include <mutex>

#include "SSBase.h"

// Not the best way. 
// "needs to have dll-interface to be used by clients of class 'Saturn::SingletonStorage'"
#if defined(_MSC_VER)
#pragma warning(disable:4251)
#endif

namespace Saturn {

	class SS_API SingletonStorage
	{
	public:
		SingletonStorage();
		~SingletonStorage();

		template<typename Ty>
		Ty* GetOrCreateSingleton()
		{
			std::type_index Index = typeid( Ty );

			if( m_Singletons.find( Index ) == m_Singletons.end() )
			{
				m_Singletons[ Index ] = new Ty();
			}

			return static_cast< Ty* >( m_Singletons.at( Index ) );
		}

		template<typename Ty>
		Ty* GetSingleton()
		{
			std::type_index Index = typeid( Ty );

			return static_cast< Ty* >( m_Singletons.at( Index ) );
		}

		template<typename Ty>
		void AddSingleton( Ty* type )
		{
			std::type_index Index = typeid( Ty );

			if( m_Singletons.find( Index ) == m_Singletons.end() )
				m_Singletons[ Index ] = type;
		}

		template<typename Ty>
		void RemoveSingleton( Ty* type ) 
		{
			std::type_index Index = typeid( Ty );
			m_Singletons.erase( Index );
		}

		static inline SingletonStorage& Get()
		{
			return *s_Instance;
		}

	private:
		std::unordered_map<std::type_index, void*> m_Singletons;
		std::mutex m_Mutex;

		static SingletonStorage* s_Instance;
	};
}
