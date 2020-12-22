/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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
#include "ModuleManager.h"
#include "Module.h"

namespace Saturn {

	ModuleManager::ModuleManager()
	{
	}

	ModuleManager::~ModuleManager()
	{
		for( int i = 0; i < m_Modules.size(); i++ )
		{
			delete m_Modules.at( i ).Raw();
		}

		m_Modules.clear();
	}

	void ModuleManager::InitNewModule( Ref<Module>& module, std::string name, std::string path )
	{
		module.Raw()->m_Manager = this;
		m_Modules.push_back( module );
	}

	void ModuleManager::AddGameModule( Ref<Module>& gamemodule )
	{
		gamemodule.Raw()->m_Manager = this;
		m_Modules.push_back( gamemodule );
	}

	Ref<Module> ModuleManager::CopyModuleFrom( Ref<Module> moudule )
	{
		Ref<Module>NewModule = moudule;
		NewModule.Raw()->m_Manager = moudule.Raw()->m_Manager;
		return NewModule;
	}

}
