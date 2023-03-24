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

// Game client main.
// DO NOT MODIFY

#include <Saturn/Core/App.h>
#include <Saturn/ImGui/RuntimeLayer.h>
#include <Saturn/Serialisation/UserSettingsSerialiser.h>

static std::string s_ProjectPath = "%PROJECT_PATH%";

// Saturn client main:
extern int _main( int, char** );

int main()
{
	// Hand it off to Saturn:
	return _main( count, args );
}

#if defined ( _WIN32 )

int WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd ) 
{
	return main( __argc, __argv );
}

#endif // _WIN32


// Client default app.
class __GameApplication : public Saturn::Application
{
	__GameApplication(const Saturn::ApplicationSpecification& spec, const std::string& rProjectPath )
		: Saturn::Application( spec ), m_ProjectPath( rProjectPath )
	{
		auto& settings = Saturn::GetUserSettings();
		settings.StartupProject = m_ProjectPath;

		size_t found = m_ProjectPath.find_last_of( "/\\" );
		settings.StartupProjectName = m_ProjectPath.substr( found + 1 );

		settings.FullStartupProjPath = m_ProjectPath + "\\" + settings.StartupProjectName + ".sproject";

		settings = Saturn::GetUserSettings();

		Saturn::UserSettingsSerialiser uss;
		uss.Deserialise( settings );
	}

	virtual void OnInit() override 
	{
		m_RuntimeLayer = new Saturn::RuntimeLayer();
		PushLayer( m_RuntimeLayer );
	}

	virtual void OnShutdown() override
	{
		Saturn::UserSettingsSerialiser uss;
		uss.Serialise( Saturn::GetUserSettings() );

		PopLayer( m_RuntimeLayer );
		delete m_RuntimeLayer;
	}
private:
	Saturn::RuntimeLayer* m_RuntimeLayer = nullptr;
	std::string m_ProjectPath = "";
}

Saturn::Application* Saturn::CreateApplication( int argc, char** argv ) 
{
	ApplicationSpecification spec {};
	spec.Titlebar = true;	

	return new __GameApplication( spec, s_ProjectPath );
}