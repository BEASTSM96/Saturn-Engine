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

#include "Saturn/Core/App.h"

#include "Saturn/Core/UserSettings.h"

#include "Saturn/Runtime/RuntimeLayer.h"

#include "EditorLayer.h"

#include "Saturn/Serialisation/UserSettingsSerialiser.h"

#include "Saturn/GameFramework/Core/GameModule.h"

class EditorApplication : public Saturn::Application
{
public:
	EditorApplication( const Saturn::ApplicationSpecification& spec, const std::string& rProjectPath )
		: Application( spec ), m_ProjectPath( rProjectPath )
	{
		// Setup user settings and find the project path.
		auto& settings = Saturn::GetUserSettings();
		Saturn::UserSettingsSerialiser uss;
		uss.Deserialise( settings );

		settings.StartupProject = m_ProjectPath;

		size_t found = m_ProjectPath.find_last_of( "/\\" );
		settings.StartupProjectName = m_ProjectPath.substr( found + 1 );

		settings.FullStartupProjPath = m_ProjectPath + "\\" + settings.StartupProjectName + ".sproject";

		settings = Saturn::GetUserSettings();

		// Check if the editor asset registry exists.
		if( !std::filesystem::exists( "content/AssetRegistry.sreg" ) )
		{
			// Create file.
			std::ofstream stream( "content/AssetRegistry.sreg" );
			stream.close();
		}

		// Set our root content path.
		m_RootContentPath = std::filesystem::current_path() / "content";
	}

	virtual void OnInit() override
	{
		m_EditorLayer = new Saturn::EditorLayer();

		PushLayer( m_EditorLayer );
	}

	virtual void OnShutdown() override
	{
		Saturn::UserSettingsSerialiser uss;
		uss.Serialise( Saturn::GetUserSettings() );

		PopLayer( m_EditorLayer );
		delete m_EditorLayer;
	}

private:
	Saturn::EditorLayer* m_EditorLayer = nullptr;

	std::string m_ProjectPath = "";
};

Saturn::Application* Saturn::CreateApplication( int argc, char** argv ) 
{
	std::string projectPath = "";

	if( argc > 1 )
		projectPath = argv[1];
	else
		projectPath = "D:\\Saturn\\Projects\\barn_blew_up";

	ApplicationSpecification spec;
	spec.Flags = (uint32_t)ApplicationFlags::CreateSceneRenderer;

	return new EditorApplication( spec, projectPath );
}