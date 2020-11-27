#pragma once

#ifdef SAT_PLATFORM_WINDOWS

extern Saturn::Application* Saturn::CreateApplication();

int main(int argc, char** argv)
{
	//Saturn::Log::Init();

	Saturn::InitCore();

	SAT_PROFILE_BEGIN_SESSION("Startup", "SaturnProfile-Startup.json");
	Saturn::Application* app = Saturn::CreateApplication();
	SAT_PROFILE_END_SESSION();

	SAT_PROFILE_BEGIN_SESSION("Runtime", "SaturnProfile-Runtime.json");
	app->Run();
	SAT_PROFILE_END_SESSION();

	SAT_PROFILE_BEGIN_SESSION("Shutdown", "SaturnProfile-Shutdown.json");
	delete app;
	SAT_PROFILE_END_SESSION();

	Saturn::EndCore();
}

#endif 