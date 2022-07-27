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

#include "Saturn/Core/App.h"

#include "EditorLayer.h"

class EditorApplication : public Saturn::Application
{
public:
	EditorApplication( const Saturn::ApplicationSpecification& spec, const std::string& rProjectPath )
		: Application( spec ), m_ProjectPath( rProjectPath )
	{
		m_UserSettings = {};

		m_UserSettings.StartupProject = rProjectPath;
	}

	virtual void OnInit() override
	{
		m_EditorLayer = new Saturn::EditorLayer();

		PushLayer( m_EditorLayer );
	}

	virtual void OnShutdown() override
	{
		PopLayer( m_EditorLayer );
		delete m_EditorLayer;
	}

private:
	Saturn::EditorLayer* m_EditorLayer;
	Saturn::UserSettings m_UserSettings;
	std::string m_ProjectPath;
};

Saturn::Application* Saturn::CreateApplication( int argc, char** argv ) 
{
	std::string projectPath = "";

	if( argc > 1 )
		projectPath = argv[1];

	return new EditorApplication( {}, projectPath );
}