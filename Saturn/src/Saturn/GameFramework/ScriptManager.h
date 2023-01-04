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

#pragma once

#include "GameScript.h"
#include <unordered_map>

namespace Saturn {

	class ScriptManager
	{
	public:
		ScriptManager();
		~ScriptManager();

		void RegisterScript( const std::string& rName );

		void BeginPlay();
		void UpdateAllScripts();
		void CreateAllScripts();

		static ScriptManager& Get() { return *s_Instance; }

	private:

		// The register function defined in the game dll. i.e. SATURN_REGISTER_SCRIPT( MyClass )
		std::unordered_map< std::string, SClass* ( __stdcall* )() > m_ScriptFunctions;

		// TODO: Maybe remove the raw ptr?
		std::unordered_map< std::string, SClass* > m_Scripts;

	private:
		static ScriptManager* s_Instance;
	};
}