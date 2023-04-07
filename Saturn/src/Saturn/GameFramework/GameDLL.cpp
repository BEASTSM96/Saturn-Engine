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

#include "sppch.h"
#include "GameDLL.h"

#include "Saturn/Core/App.h"

#include "Saturn/Project/Project.h"

#include "GameScript.h"

#include "SourceManager.h"

namespace Saturn {

	GameDLL::GameDLL()
	{
		SingletonStorage::Get().AddSingleton( this );
	}

	void GameDLL::Load( bool reload /*=false*/ )
	{
		if( !Application::Get().GetSpecification().GameDist )
		{
			// We are the editor

			auto binDir = Project::GetActiveProject()->GetBinDir();

			auto& DllPath = binDir /= Project::GetActiveProject()->GetName() + ".dll";
			m_DLLInstance = LoadLibraryA( DllPath.string().c_str() );

			SourceManager::Get();

			SAT_CORE_INFO( "Loaded Game DLL!" );
		}
		else // We are the game
		{
			m_DLLInstance = GetModuleHandle( NULL );
			SourceManager::Get();
		}
	}

	void GameDLL::Unload()
	{
		if( !Application::Get().GetSpecification().GameDist )
		{
			FreeLibrary( m_DLLInstance );
		}
	}

	void GameDLL::Reload() 
	{
		Unload();
		Load(true);
	}
}