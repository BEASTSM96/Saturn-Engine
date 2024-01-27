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

// Game client main.
// DO NOT MODIFY

#include <Windows.h>

#include <Saturn/Core/App.h>
#include <Saturn/Runtime/RuntimeLayer.h>
#include <Saturn/Project/Project.h>
#include <Saturn/Vulkan/ShaderBundle.h>
#include <Saturn/Serialisation/EngineSettingsSerialiser.h>
#include <Saturn/Serialisation/ProjectSerialiser.h>

static std::string s_ProjectPath = "";

// Saturn client main:
extern int _main( int, char** );

int main( int count, char** args )
{
	// Hand it off to Saturn:
	return _main( count, args );
}

#if defined ( _WIN32 )

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	return _main( __argc, __argv );
}

#endif // _WIN32

// Client default app.
class __GameApplication : public Saturn::Application
{
public:
	__GameApplication( const Saturn::ApplicationSpecification& spec )
		: Saturn::Application( spec )
	{
		Saturn::EngineSettingsSerialiser uss;
		uss.Deserialise();

		Saturn::EngineSettings& rSettings = Saturn::EngineSettings::Get();
		rSettings.StartupProject = s_ProjectPath;

		size_t found = s_ProjectPath.find_last_of( "/\\" );
		rSettings.StartupProjectName = s_ProjectPath.substr( found + 1 );

		rSettings.FullStartupProjPath = s_ProjectPath;

		m_RootContentPath = std::filesystem::current_path() / "content";

		// Load the project really early on because we still need to load the shader bundle and create the scene renderer.
		Saturn::ProjectSerialiser ps;
		ps.Deserialise( rSettings.FullStartupProjPath.string() );

		SAT_CORE_ASSERT( Saturn::Project::GetActiveProject(), "No project was given." );

		// Load the shader bundle.
		Saturn::ShaderBundle::Get().ReadBundle();

		Saturn::SceneRendererFlags flags = Saturn::SceneRendererFlag_MasterInstance | Saturn::SceneRendererFlag_SwapchainTarget;

		m_SceneRenderer = new Saturn::SceneRenderer( flags );
	}

	virtual void OnInit() override
	{
		m_RuntimeLayer = new Saturn::RuntimeLayer();
		PushLayer( m_RuntimeLayer );
	}

	virtual void OnShutdown() override
	{
		Saturn::EngineSettingsSerialiser uss;
		uss.Serialise();

		PopLayer( m_RuntimeLayer );
		delete m_RuntimeLayer;
	}

private:
	Saturn::RuntimeLayer* m_RuntimeLayer = nullptr;
};

Saturn::Application* Saturn::CreateApplication( int argc, char** argv )
{
	std::filesystem::path WorkingDir = argv[ 0 ];
	std::filesystem::current_path( WorkingDir.parent_path() );

	Saturn::ApplicationSpecification spec{};
	spec.Flags = Saturn::ApplicationFlag_CreateSceneRenderer | Saturn::ApplicationFlag_GameDist | Saturn::ApplicationFlag_Titlebar;

	s_ProjectPath = Saturn::Project::FindProjectDir( "%PROJECT_NAME%" );

	return new __GameApplication( spec );
}