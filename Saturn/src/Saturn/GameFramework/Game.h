#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Core/Assets/Asset.h"

namespace Saturn {

	class GameContext;

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

		bool Compile( GameContext* gameContext );
		void Start( void );
		void End( void );

		std::string& GetName() { return m_Name; }

	protected:
		std::string m_Name = "MyGame";

	private:
		friend class GameContext;
	};

	class GameContext : public RefCounted
	{
	public:
		GameContext();
		~GameContext();

		void CompileAllGames( void );

		template<class T>
		Ref< T > LookFor() 
		{
			SAT_CORE_ASSERT( false, "Not added" );
		}

		bool CompileGame( char name )
		{
			for ( auto& cn : m_Games )
			{
				//if( cn.GetName().c_str() == ( const char* )name )
					return cn.Compile( this );
			}
		}

		std::vector<Game> m_Games;

	private:
		friend class Game;
	};

}