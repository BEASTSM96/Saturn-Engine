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
#include "Base.h"

#include "Saturn/Log.h"
#include "Saturn/Physics/PhysX/PhysXFnd.h"
#include "Saturn/Script/ScriptEngine.h"

namespace Saturn {

	static bool s_AppRestart = true;
	static bool s_AppRestartInProg = false;

	void InitCore( void )
	{
		Saturn::Log::Init();

		SAT_CORE_TRACE("View License at https://github.com/BEASTSM96/Saturn-Engine/blob/master/LICENSE");
		SAT_CORE_TRACE("API Docs (https://beastsm96.github.io/Projects/Saturn-Engine/api/v0.a01)");
	}

	void EndCore( void )
	{
		SAT_CORE_WARN( "Shuting Down!" );
		Saturn::Log::Clear();
	}

	bool CheckRestart()
	{
		return s_AppRestart;
	}

	void StartRestart()
	{
		s_AppRestartInProg = true;

		Saturn::PhysXFnd::Clear();
		Saturn::ScriptEngine::Shutdown();
	}

	bool RestartInProg()
	{
		if( s_AppRestart )
			return true;
		return false;
	}

	void SetRestartOnAppClose( bool restart )
	{
		s_AppRestart = restart;
	}

}