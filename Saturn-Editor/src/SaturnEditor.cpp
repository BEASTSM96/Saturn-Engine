/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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
#include "Saturn/Core/ErrorDialog.h"

#include "Saturn/Core/EngineSettings.h"

#include "Saturn/Runtime/RuntimeLayer.h"

#include "EditorLayer.h"

#include "Saturn/Serialisation/EngineSettingsSerialiser.h"

#include "Saturn/GameFramework/Core/GameModule.h"

class EditorApplication final : public Saturn::Application
{
public:
	explicit EditorApplication( const Saturn::ApplicationSpecification& spec, const std::filesystem::path& rProjectPath )
		: Application( spec ), m_ProjectPath( rProjectPath )
	{
		// Setup user settings and find the project path.
		Saturn::EngineSettingsSerialiser uss;
		uss.Deserialise();

		// Set our root content path.
		// TODO: Remove this
		// We can't remove until we no longer need editor assets for Dist.
		m_RootContentPath = std::filesystem::current_path() / "content";

		if( m_ProjectPath.empty() )
		{
			// TODO: if there is not startup project, then go to recent projects, if none, show dialog telling user to go to project browser.
			m_ProjectPath = Saturn::EngineSettings::Get().StartupProject;
		}
	}

	virtual void OnInit() override
	{
		m_EditorLayer = new Saturn::EditorLayer();

		PushLayer( m_EditorLayer );
	}

	virtual void OnShutdown() override
	{
		Saturn::EngineSettingsSerialiser uss;
		uss.Serialise();

		PopLayer( m_EditorLayer );
		delete m_EditorLayer;
	}

private:
	Saturn::EditorLayer* m_EditorLayer = nullptr;
	std::filesystem::path m_ProjectPath;
};

Saturn::Application* Saturn::CreateApplication( int argc, char** argv ) 
{	
	std::string projectPath = "";
	if( argc > 1 )
		projectPath = argv[1];

	ApplicationSpecification spec;
	spec.Flags = ApplicationFlag_CreateSceneRenderer;

	return new EditorApplication( spec, projectPath );
}