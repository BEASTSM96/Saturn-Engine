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