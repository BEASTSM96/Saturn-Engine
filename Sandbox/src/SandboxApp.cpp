#include <Sparky.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"

class ExampleLayer : public Sparky::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f), m_SquarePosition(0.0f)
	{
		m_VertexArray.reset(Sparky::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		Sparky::Ref<Sparky::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Sparky::VertexBuffer::Create(vertices, sizeof(vertices)));
		Sparky::BufferLayout layout = {
			{ Sparky::ShaderDataType::Float3, "a_Position" },
			{ Sparky::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		Sparky::Ref<Sparky::IndexBuffer> indexBuffer;
		indexBuffer.reset(Sparky::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(Sparky::VertexArray::Create());

		float squareVertices[3 * 4] = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f
		};

		Sparky::Ref<Sparky::VertexBuffer> squareVB;
		squareVB.reset(Sparky::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Sparky::ShaderDataType::Float3, "a_Position" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Sparky::Ref<Sparky::IndexBuffer> squareIB;
		squareIB.reset(Sparky::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec3 v_Position;
			out vec4 v_Color;
			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform *vec4(a_Position, 1.0);	
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			in vec4 v_Color;
			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_Shader.reset(Sparky::Shader::Create(vertexSrc, fragmentSrc));

		std::string flatShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec3 v_Position;
			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string flastShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_flatShader.reset(Sparky::Shader::Create(flatShaderVertexSrc, flastShaderFragmentSrc));
	}

	void OnUpdate(Sparky::Timestep ts) override
	{
		if (Sparky::Input::IsKeyPressed(SP_KEY_LEFT))
		{
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		}

		else if (Sparky::Input::IsKeyPressed(SP_KEY_RIGHT))
		{
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		}

		if (Sparky::Input::IsKeyPressed(SP_KEY_DOWN))
		{
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		}

		else if (Sparky::Input::IsKeyPressed(SP_KEY_UP))
		{
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
		}

		if (Sparky::Input::IsKeyPressed(SP_KEY_A))
		{
			m_CameraRotation += m_CameraRotationSpeed * ts;
		}

		if (Sparky::Input::IsKeyPressed(SP_KEY_D))
		{
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}

		Sparky::RenderCommand::SetClearColor({ 1, 1, 0, 0 });
		Sparky::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Sparky::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_flatShader)->Bind();
		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_flatShader)->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 trasform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Sparky::Renderer::Submit(m_flatShader, m_SquareVA, trasform);
			}

		}
		
		Sparky::Renderer::Submit(m_Shader, m_VertexArray);

		Sparky::Renderer::EndScene();

	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");

		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));

		ImGui::End();

	}

	void OnEvent(Sparky::Event& event) override
	{
		Sparky::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Sparky::KeyPressedEvent>(SP_BIND_EVENT_FN(ExampleLayer::OnKeyPressed));
	}

	bool OnKeyPressed(Sparky::KeyPressedEvent& event) {
	
		

		return false;

	}

private:

	Sparky::Ref<Sparky::Shader> m_Shader;
	Sparky::Ref<Sparky::VertexArray> m_VertexArray;

	Sparky::Ref<Sparky::Shader> m_flatShader;
	Sparky::Ref<Sparky::VertexArray> m_SquareVA;

	Sparky::OrthographicCamera m_Camera;

	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 5.0f;


	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;

	glm::vec3 m_SquarePosition;
public:
	glm::vec3 m_SquareColor = {0.2f, 0.3f, 0.8f};
};


class Sandbox : public Sparky::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());

	}

	~Sandbox()
	{

	}

};

Sparky::Application* Sparky::CreateApplication()
{
	return new Sandbox();
}