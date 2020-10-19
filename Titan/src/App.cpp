#include <Sparky.h>

class Sandbox : public Saturn::Application
{
public:
	Sandbox()
	{
	}

	~Sandbox()
	{
	}

private:
	//std::string m_GameName = "";
	//std::vector<Saturn::GameObject*> m_NewGameObject;
};

Saturn::Application* Saturn::CreateApplication()
{
	return new Sandbox();
}