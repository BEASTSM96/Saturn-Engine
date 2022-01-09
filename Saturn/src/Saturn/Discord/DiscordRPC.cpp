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

#include "sppch.h"
#include "DiscordRPC.h"

#define DISCORD_APP_ID "898878512990134283"

namespace Saturn {

	static const char* GetImgFromStr( const std::string& str )
	{
		if( str == ".cpp" || str == ".hpp" || str == ".h" /* Assume .h is c++ */ || str == ".cxx" )
			return "img_l_cpp";
		else if( str == ".c" )
			return "img_l_c";
		else if( str == ".cs" )
			return "img_l_cs";
		else if( str == "saturninrt" )
			return "img_s_staurn";
		else
			return "default";
	}

	static void HandleDiscordReady( const DiscordUser* connectedUser )
	{
		// TODO: Print to status bar
		printf( "\nDiscord: connected to user %s#%s - %s\n", connectedUser->username, connectedUser->discriminator, connectedUser->userId );
	}

	static void HandleDiscordDisconnected( int errcode, const char* message )
	{
		// TODO: Print to status bar
		printf( "\nDiscord: disconnected (%d: %s)\n", errcode, message );
	}

	static void HandleDiscordError( int errcode, const char* message )
	{
		// TODO: Print to status bar
		printf( "\nDiscord: error (%d: %s)\n", errcode, message );
	}

	void DiscordRPC::Init()
	{
		DiscordEventHandlers handlers{};

		handlers.ready        = HandleDiscordReady;
		handlers.disconnected = HandleDiscordDisconnected;
		handlers.errored      = HandleDiscordError;

		Discord_Initialize( DISCORD_APP_ID, &handlers, 1, NULL );
	}

	void DiscordRPC::Update()
	{
		m_StartInUnixTime = ( int64_t )std::chrono::duration_cast< std::chrono::seconds >( m_StartTime.time_since_epoch() ).count();

		DiscordRichPresence DiscordPresence{};

		if( m_SceneName == "" || m_SceneName == "No Scene" )
		{
			DiscordPresence.state = "No Scene";
		}
		else if( m_SceneName != ( const char* )"No Scene" )
		{
			DiscordPresence.state = m_SceneName.c_str();
		}

		DiscordPresence.startTimestamp  = ( int64_t )m_StartInUnixTime;

		DiscordPresence.instance = 0;

		DiscordPresence.largeImageKey = GetImgFromStr( "saturninrt" );
		DiscordPresence.smallImageKey = GetImgFromStr( "saturninrt" );
		DiscordPresence.smallImageText = "Saturn";

		m_CurrentRPC = DiscordPresence;

		DiscordPresence ={};

		Discord_UpdatePresence( &m_CurrentRPC );

		Discord_RunCallbacks();

	}

	void DiscordRPC::Shutdown()
	{
		Discord_Shutdown();
	}

}
