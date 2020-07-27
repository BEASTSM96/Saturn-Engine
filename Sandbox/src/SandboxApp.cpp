#include <Sparky.h>

#include "Sparky\ImGui\ImGuiLayer.h"


class ExampleLayer : public Sparky::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		//SP_INFO("ExampleLayer::Update");

	}

	void OnImGuiRender() override
	{

	}

	void OnEvent(Sparky::Event& event) override
	{
		if (event.GetEventType() == Sparky::EventType::KeyPressed) 
		{
			Sparky::KeyPressedEvent& e = (Sparky::KeyPressedEvent&)event;

			SP_TRACE("{0}", (char)e.GetKeyCode());
		}
		
	}

};


class Sandbox : public Sparky::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());

		//PushOverlay(new Sparky::ImGuiLayer());
	}

	~Sandbox()
	{

	}

};

Sparky::Application* Sparky::CreateApplication()
{
	return new Sandbox();
}