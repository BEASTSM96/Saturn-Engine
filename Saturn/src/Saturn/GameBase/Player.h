#pragma once


/*
* Sparky BasePlayer
*/


#include "Saturn/GameBase/GameObject.h"
#include "Saturn/GameBase/GameLayer.h"


#include "Saturn/Core/Serialisation/Object.h"
#include "Saturn/Core/Serialisation/Serialiser.h"

#include "Saturn/Renderer/OrthographicCamera.h"

#include "Saturn/Renderer/Texture.h"
#include "Saturn/Renderer/OrthographicCamera.h"

#include "Saturn/Renderer/VertexArray.h"
#include "Saturn/Renderer/Shader.h"

#include "Saturn/Renderer/Renderer.h"


#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "Saturn/Application.h"

#include "Saturn/Core.h"

#include<fstream>
#include<string>
#include<iterator>

#ifdef PLAYER_INCLUDE




namespace Saturn { 

	struct STexture
	{
		STexture(std::string name, std::string path) : name(name), path(path) {  }
		std::string name;
		std::string path;
	};

	class SATURN_API Player : public Layer
	{
	public:
		Player();
		~Player();

		void OnSumbit();

		void OnUpdate(Timestep ts) override;
		void OnImGuiRender() override;
		void OnEvent(Event& event) override;

		bool OnKeyPressed(KeyPressedEvent& event);

	private:

		bool m_HasBeenSerialised = false;

		Saturn::Ref<Saturn::Shader> m_playerShader;
		Saturn::Ref<Saturn::VertexArray> m_playerVA;

		Saturn::Ref<Saturn::Texture2D> m_playerTexture;

		Saturn::Ref<Saturn::Shader> m_Shader;
		Saturn::Ref<Saturn::VertexArray> m_VertexArray;

		Saturn::Ref<Saturn::Shader> m_flatShader, m_TextureShader;
		Saturn::Ref<Saturn::VertexArray> m_SquareVA;

		Saturn::Ref<Saturn::Texture2D> m_Texture, m_beastlogo;

		OrthographicCamera m_Camera;

		glm::vec3 m_CameraPosition;
		float m_CameraMoveSpeed = 5.0f;


		float m_CameraRotation = 0.0f;
		float m_CameraRotationSpeed = 180.0f;


		float m_PlayerRotation = 0.0f;
		float m_PlayerRotationSpeed = 180.0f;

		glm::vec3 m_PlayerPosition;
		float m_PlayerMoveSpeed = 5.0f;

		std::vector<STexture> textures{
			STexture("Beastpic", "assets/tex/beastpic_pfp_1.png"),
						STexture("Beastpic2", "assets/tex/sunilde.png"),
			STexture("Checkerboard", "assets/tex/Checkerboard.png")
		};

	protected:
		virtual void archive() override {

			SerialisationData(new Serialisable<int>("PlayerPos", &m_PlayerPosition));
		}

	public:
		glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

	private:
		static Player* s_Instance;

		Json::Value serialiser;

	};
	
}
#endif // PLAYER_INCLUDE