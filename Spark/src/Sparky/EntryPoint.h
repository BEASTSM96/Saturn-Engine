
#pragma once

#ifdef SP_PLATFORM_WINDOWS

extern Sparky::Application* Sparky::CreateApplication();

int main(int argc, char** argv)
{
	Sparky::Log::Init();
	SP_CORE_WARN("Log inited!");
	int a = 5;
	//SP_INFO("Hello Var={0}", a);
	auto app = Sparky::CreateApplication();
	app->Run();
	delete app;
}

#endif 