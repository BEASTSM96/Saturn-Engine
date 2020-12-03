#include "sppch.h"
#include "Game.h"

#ifdef SAT_PLATFORM_WINDOWS
#include <Windows.h>
#endif // SAT_PLATFORM_WINDOWS


namespace Saturn {

	Game::Game()
	{
	}

	Game::~Game()
	{

	}

	bool Game::Compile( GameContext* gameContext )
	{
		std::string MSBuildPath = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe";

		//TODO: Somehow create the msbuild

		return true;
	}

	void Game::Start( void )
	{

	}

	void Game::End( void )
	{

	}

	GameContext::GameContext()
	{

	}

	GameContext::~GameContext()
	{

	}

	void GameContext::CompileAllGames( void )
	{
		for( auto& cn : m_Games )
		{
			cn.Compile(this);
		}
	}

}