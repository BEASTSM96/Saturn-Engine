
#pragma once

#ifdef SP_PLATFORM_WINDOWS

extern Sparky::Application* Sparky::CreateApplication();

int main(int argc, char** argv)
{
	Sparky::Log::Init();
	auto app = Sparky::CreateApplication();
	app->Run();
	delete app;
}

#endif 