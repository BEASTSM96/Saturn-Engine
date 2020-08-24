#include "sppch.h"
#include "Player.h"



#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "Platform/OpenGL/OpenGLShader.h"

#include "Sparky/Core/Serialisation/Serialiser.h"

#include <json/json.h>

#include "Sparky/Layer.h"

#include "Sparky/Core.h"

#include "Sparky/Input.h"

#include "Sparky/KeyCodes.h"

namespace Sparky {
	
	Player::Player(  ): m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), Layer("Player")
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

		
		Json::Value serialiser;
		for (STexture i : textures)
		{
			serialiser["Textures"][i.name]["Path"] = i.path;
		}

		{
			std::ofstream texFile("assets/tex/player/json/player.json");

			texFile << serialiser;
		}

		//Deserialise
		std::vector<STexture*> deserialiseObjects;
		uint32_t count = 0;
		for (const Json::Value& object : serialiser["Textures"])
		{
			deserialiseObjects.push_back(new STexture(serialiser["Textures"].getMemberNames()[count], object.get("Path", 0).asString()));
			count++;
		}

		deserialiseObjects;

		m_playerTexture = Sparky::Texture2D::Create(deserialiseObjects.at(0)->path);

		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_playerShader)->Bind();
		std::dynamic_pointer_cast<Sparky::OpenGLShader>(m_playerShader)->UploadUniformInt("u_Texture", 0);

	}


	Player::~Player()
	{
	}

	void Player::OnUpdate(Timestep ts)
	{

		if (Input::IsKeyPressed(SP_KEY_LEFT))
		{
			m_CameraPosition.x += m_CameraMoveSpeed * ts;

			Json::Value serialiser;

			for (STexture i : textures)
			{
				serialiser["Textures"][i.name]["Path"] = i.path;
			}

			//Deserialise
			std::vector<STexture*> deserialiseObjects;
			uint32_t count = 0;
			for (const Json::Value& object : serialiser["Textures"])
			{
				deserialiseObjects.push_back(new STexture(serialiser["Textures"].getMemberNames()[count], object.get("Path", 0).asString()));
				count++;
			}


			m_playerTexture = Sparky::Texture2D::Create(deserialiseObjects.at(1)->path);

		}

		else if (Input::IsKeyPressed(SP_KEY_RIGHT))
		{
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		}

		if (Input::IsKeyPressed(SP_KEY_DOWN))
		{
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		}

		else if (Input::IsKeyPressed(SP_KEY_UP))
		{
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
		}

		RenderCommand::SetClearColor({ 1, 1, 0, 0 });
		RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		m_playerTexture->Bind();
		Renderer::Submit(m_playerShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		Renderer::EndScene();

	}

	void Player::OnImGuiRender()
	{
	}

	void Player::OnEvent(Event& event)
	{

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Sparky::KeyPressedEvent>(SP_BIND_EVENT_FN(Player::OnKeyPressed));
	
	}

	bool Player::OnKeyPressed(KeyPressedEvent& event)
	{
		return false;
	}

}
