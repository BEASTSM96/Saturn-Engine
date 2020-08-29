#pragma once


/*
* Sparky BasePlayer
*/


#include "Sparky/GameBase/GameObject.h"
#include "Sparky/GameBase/GameLayer.h"


#include "Sparky/Core/Serialisation/Object.h"
#include "Sparky/Core/Serialisation/Serialiser.h"

#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Renderer/Texture.h"
#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Shader.h"

#include "Sparky/Renderer/Renderer.h"


#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "Sparky/Application.h"

#include "Sparky/Core.h"

#include<fstream>
#include<string>
#include<iterator>

#ifdef PLAYER_INCLUDE




namespace Sparky { 

	struct STexture
	{
		STexture(std::string name, std::string path) : name(name), path(path) {  }
		std::string name;
		std::string path;
	};

	class Player : public Layer
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

		Sparky::Ref<Sparky::Shader> m_playerShader;
		Sparky::Ref<Sparky::VertexArray> m_playerVA;

		Sparky::Ref<Sparky::Texture2D> m_playerTexture;

		Sparky::Ref<Sparky::Shader> m_Shader;
		Sparky::Ref<Sparky::VertexArray> m_VertexArray;

		Sparky::Ref<Sparky::Shader> m_flatShader, m_TextureShader;
		Sparky::Ref<Sparky::VertexArray> m_SquareVA;

		Sparky::Ref<Sparky::Texture2D> m_Texture, m_beastlogo;

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