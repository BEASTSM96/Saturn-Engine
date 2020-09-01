#include "sppch.h"
#include "GameObject.h"

#include "GameLayer.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "Sparky/Core/Serialisation/Object.h"
#include "Sparky/Application.h"

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#ifdef SPARKY_GAME_BASE

namespace Sparky {

	GameObject* GameObject::s_Instance = nullptr;

	GameObject::GameObject() : Serialiser::OBJ_NAME("GameObject")
	{
		m_SquareVA.reset(Sparky::VertexArray::Create());

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		};

		Sparky::Ref<Sparky::VertexBuffer> squareVB;
		squareVB.reset(Sparky::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Sparky::ShaderDataType::Float3, "a_Position" },
			{ Sparky::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Sparky::Ref<Sparky::IndexBuffer> squareIB;
		squareIB.reset(Sparky::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string textureShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec2 v_TexCoord;
			void main()
			{
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string textureShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;
			in vec2 v_TexCoord;
			
			uniform sampler2D u_Texture;
			void main()
			{
				color = texture(u_Texture, v_TexCoord);
			}
		)";

		m_playerShader.reset(Sparky::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));


		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_playerShader)->Bind();
		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_playerShader)->UploadUniformInt("u_Texture", 0);

	}

	GameObject::GameObject(const std::string& objectname, Json::Value& reconstructionValue) : Serialiser::OBJ_NAME("GameObject")
	{
		m_SquareVA.reset(Sparky::VertexArray::Create());

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		};

		Sparky::Ref<Sparky::VertexBuffer> squareVB;
		squareVB.reset(Sparky::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Sparky::ShaderDataType::Float3, "a_Position" },
			{ Sparky::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Sparky::Ref<Sparky::IndexBuffer> squareIB;
		squareIB.reset(Sparky::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string textureShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec2 v_TexCoord;
			void main()
			{
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

		std::string textureShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;
			in vec2 v_TexCoord;
			
			uniform sampler2D u_Texture;
			void main()
			{
				color = texture(u_Texture, v_TexCoord);
			}
		)";

		m_playerShader.reset(Sparky::Shader::Create(textureShaderVertexSrc, textureShaderFragmentSrc));


		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_playerShader)->Bind();
		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_playerShader)->UploadUniformInt("u_Texture", 0);
	}

	void GameObject::Init() {

		s_Instance = this;

		GameLayer* gl = Application::Get().m_gameLayer;

		gl->gameObjects.push_back(*this);

		SP_CORE_WARN("GameObject Size {0} " , gl->gameObjects.size());

	}

	void GameObject::OnKeyInput(KeyPressedEvent& InEvent)
	{
	}

	void GameObject::OnPos()
	{
		m_PlayerPosition.x += 50.0f * 0.01f;
	}

	void GameObject::OnUpdate(Timestep ts) {
	
		if (Sparky::Input::IsKeyPressed(SP_KEY_LEFT))
		{
			//m_PlayerPosition.x += m_PlayerMoveSpeed *ts;

			SP_CORE_INFO("PLAYER POS {0} ", m_PlayerPosition.x);

			(Application::Get().m_gameLayer->m_CameraPosition.x += Application::Get().m_gameLayer->m_CameraMoveSpeed * ts);
		}

		else if (Sparky::Input::IsKeyPressed(SP_KEY_RIGHT))
		{
			m_PlayerPosition.x -= m_PlayerMoveSpeed * ts;

			(Application::Get().m_gameLayer->m_CameraPosition.x -= Application::Get().m_gameLayer->m_CameraMoveSpeed * ts);
		}

		if (Sparky::Input::IsKeyPressed(SP_KEY_DOWN))
		{
			m_PlayerPosition.y += m_PlayerMoveSpeed * ts;

			(Application::Get().m_gameLayer->m_CameraPosition.y += Application::Get().m_gameLayer->m_CameraMoveSpeed * ts);
		}

		else if (Sparky::Input::IsKeyPressed(SP_KEY_UP))
		{
			m_PlayerPosition.y -= m_PlayerMoveSpeed * ts;

			(Application::Get().m_gameLayer->m_CameraPosition.y -= Application::Get().m_gameLayer->m_CameraMoveSpeed * ts);
		}

		if (Sparky::Input::IsKeyPressed(SP_KEY_A))
		{
			m_PlayerRotation += m_PlayerRotationSpeed * ts;
		}

		if (Sparky::Input::IsKeyPressed(SP_KEY_D))
		{
			m_PlayerRotation -= m_PlayerRotationSpeed * ts;
		}
	
	}

	void GameObject::Render()
	{	
		GameLayer* gl = Application::Get().m_gameLayer;

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.1f));

		FTransform tras = FTransform(m_PlayerPosition, scale, 0.0f);

		gl->Sumbit(m_playerShader, m_SquareVA, tras);
		//m_playerTexture->Bind();
	}

}

#endif