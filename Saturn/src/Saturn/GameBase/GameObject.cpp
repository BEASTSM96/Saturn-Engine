#include "sppch.h"
#include "GameObject.h"
#include "Saturn/Core/Serialisation/Object.h"

#include "GameLayer.h"

#include "Saturn/Application.h"
#include "Saturn/Scene/Components.h"

#include "Platform/OpenGL/OpenGLShader.h"


#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>



#ifdef SPARKY_GAME_BASE

namespace Saturn {

	GameObject* GameObject::s_Instance = nullptr;

	GameObject::GameObject() : Serialiser::OBJ_NAME("GameObject")
	{
	}

	GameObject::GameObject(const std::string& objectname, Json::Value& reconstructionValue) : Serialiser::OBJ_NAME("GameObject")
	{
	}

	GameObject::GameObject(entt::entity handle, Scene* scene) : Serialiser::OBJ_NAME("GameObject"), m_EntityHandle(handle), m_Scene(scene)
	{

	}

	void GameObject::Init() {

		s_Instance = this;

		// tell stb_image.h to flip loaded texture's on the y-axis
		stbi_set_flip_vertically_on_load(true);


		GetComponent<TransformComponent>().Transform = glm::translate(transform, glm::vec3(10.0f, 10.0f, 10.0f));
		GetComponent<TransformComponent>().Transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));

		if (m_3D)
		{																							//Will prob be GameObject::s_Instance->GetMeshComp()
			ourShader = new DShader("assets/shaders/3d_test.vs", "assets/shaders/3d_test.fs");

			ourModel = new Model("assets/meshes/base_cone.obj");

	
			shaders.push_back(ourShader);
		}

		GameObjectState = E_GameObjectState::Idle;

		GameLayer * gl = static_cast<GameLayer*>(Application::Get().GetCurrentScene().m_CurrentLevel->GetGameLayer());

		gl->gameObjects.push_back(*this);

		SAT_CORE_WARN("GameObject Size {0} " , gl->gameObjects.size());
	}

	void GameObject::OnKeyInput(KeyPressedEvent& InEvent)
	{
	}

	void GameObject::OnPos()
	{
		m_PlayerPosition.x += 50.0f * 0.01f;
	}

	void GameObject::OnUpdate(Timestep ts) {
	
		ourModel->SetTransform(GetComponent<TransformComponent>().Transform);
		ourModel->Update(ts, *ourShader);
	
	}

	void GameObject::Render()
	{	
		if (!m_3D)
		{
			GameLayer* gl = Application::Get().m_gameLayer;

			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.1f));

			FTransform tras = FTransform(m_PlayerPosition, scale, 0.0f);

			gl->Sumbit(m_playerShader, m_SquareVA, tras);
			//m_playerTexture->Bind();
		}
		else 
		{
			GameLayer* gl = Application::Get().m_gameLayer;

			ourShader->use();

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(Application::Get().m_gameLayer->m_3DCamera.Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);
			glm::mat4 view = Application::Get().m_gameLayer -> m_3DCamera.GetViewMatrix();
			ourShader -> setMat4("projection", projection);
			ourShader -> setMat4("view", view);

			ourShader->setMat4("model", GetComponent<TransformComponent>().Transform);

	
			ourModel -> Draw(*ourShader);

		}
	}

	// utility function for loading a 2D texture from file
	unsigned int  GameObject::loadTexture(char const* path)
	{
		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}
}

#endif