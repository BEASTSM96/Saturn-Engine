#include <Saturn.h>

class Sandbox : public Saturn::Application
{
public:
	Sandbox()
	{
	}

	~Sandbox()
	{

	}

};

Saturn::Application* Saturn::CreateApplication()
{
	return new Sandbox();
}