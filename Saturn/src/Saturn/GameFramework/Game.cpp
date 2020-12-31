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

#include "sppch.h"
#include "Game.h"

#include "HotReload.h"

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

	bool Game::Compile( Saturn::GameContext* gameContext )
	{
		std::string MSBuildPath = "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe";

		//TODO: Somehow create the msbuild

		//system("cd ../Game\\");
		//system("cd C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin");
		//system("MSBuild.exe");

		return true;
	}

	void Game::Start( void )
	{

	}

	void Game::End( void )
	{

	}

	GameContext::GameContext( HotReload* hotReload )
	{
		m_Scene = hotReload->m_Scece.Raw();
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

	void GameContext::ConfigAllGames( void )
	{
		for( auto& cn : m_Games )
		{
			cn.ConfigGame( m_Scene );
		}
	}

}