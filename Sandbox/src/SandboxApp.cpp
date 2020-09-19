#include <Sparky.h>

#include <Saturn/GameBase/GameObject.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

#ifndef SPARKY_SANDBOX
#define SPARKY_SANDBOX
#include <Saturn/Core/Serialisation/Serialiser.h>
#endif // SPARKY_SANDBOX

class ExampleLayer : public Saturn::Layer
{
public:

	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f), m_SquarePosition(0.0f)
	{
	}

	void OnUpdate(Saturn::Timestep ts) override
	{
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");

		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));

		ImGui::End();

	}

	void OnEvent(Saturn::Event& event) override
	{
		Saturn::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Saturn::KeyPressedEvent>(SAT_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
	}

	bool OnKeyPressed(Saturn::KeyPressedEvent& event) {
	
		

		return false;

	}

private:

	Saturn::Ref<Saturn::Shader> m_Shader;
	Saturn::Ref<Saturn::VertexArray> m_VertexArray;

	Saturn::Ref<Saturn::Shader> m_flatShader, m_TextureShader;
	Saturn::Ref<Saturn::VertexArray> m_SquareVA;

	Saturn::Ref<Saturn::Texture2D> m_Texture, m_beastlogo;

	Saturn::OrthographicCamera m_Camera;

	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 5.0f;


	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;

	glm::vec3 m_SquarePosition;
public:
	glm::vec3 m_SquareColor = {0.2f, 0.3f, 0.8f};
};


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