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

#include "Saturn/Core/Base.h"
#include "Saturn/Core/Assets/Asset.h"
#include "Saturn/Scene/Scene.h"

#include <vector>
#include <string>

namespace Saturn {

	class GameContext;
	class HotReload;

	class Game : public RefCounted
	{
	public:
		Game();
		~Game();

		template<class T>
		T LookFor( char name )
		{
			SAT_CORE_ASSERT( std::is_base_of<Asset, T>::value, "Class is not Asset!" );
		}

		template<class T>
		T* LookFor( char name )
		{
			SAT_CORE_ASSERT( std::is_base_of<Asset, T>::value, "Class is not Asset!" );
		}

		template<class T>
		Ref< T > LookFor( char name )
		{
			SAT_CORE_ASSERT( std::is_base_of<Asset, T>::value, "Class is not Asset!" );
		}


		bool Compile( Ref< GameContext > gameContext ) 
		{
			return Compile( gameContext.Raw() );
		}

		public:
			virtual void ConfigGame( Ref<Scene> runtimeScece )  = 0;
		public:
			
		virtual bool Compile( Saturn::GameContext* gameContext );
		void Start( void );
		void End( void );

		std::string& GetName() { return m_Name; }

	protected:
		std::string m_Name = "MyGame";

	private:
		friend class GameContext;
	};

	class GAMEFRAMEWORK_API GameContext : public RefCounted
	{
	public:
		GameContext( HotReload* hotReload );
		~GameContext();

		void CompileAllGames( void );
		void ConfigAllGames( void );

		template<class T>
		Ref< T > LookFor()
		{
			SAT_CORE_ASSERT( false, "Not added" );
		}

		bool CompileGame( char name )
		{
			for( auto& cn : m_Games )
			{
				//if( cn.GetName().c_str() == ( const char* )name )
				return cn.Compile( this );
			}
		}

		std::vector<Game> m_Games;
	private:
		Scene* m_Scene;
	private:
		friend class Game;
	};

}