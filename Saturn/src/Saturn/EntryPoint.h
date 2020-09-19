
#pragma once

#ifdef SAT_PLATFORM_WINDOWS

extern Saturn::Application* Saturn::CreateApplication();

int main(int argc, char** argv)
{
	Saturn::Log::Init();
	auto app = Saturn::CreateApplication();
	app->Run();
	delete app;
}

#endif 