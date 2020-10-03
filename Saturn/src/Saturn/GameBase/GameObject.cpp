#include "sppch.h"
#include "GameObject.h"
#include "Saturn/Core/Serialisation/Object.h"

#include "Saturn/Renderer/3D/3dShader.h"
#include "GameLayer.h"

#include "Saturn/Application.h"
#include "Saturn/Scene/Components.h"

#include "Platform/OpenGL/OpenGLShader.h"


#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtc/type_ptr.hpp>

#pragma warning(disable: 26812)


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

		SAT_PROFILE_FUNCTION();

		s_Instance = this;

		// tell stb_image.h to flip loaded texture's on the y-axis
		stbi_set_flip_vertically_on_load(true);

		GetComponent<TransformComponent>().Transform = glm::translate(transform, glm::vec3(10.0f, 10.0f, 10.0f));
		GetComponent<TransformComponent>().Transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));

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
		SAT_PROFILE_FUNCTION();

		if (HasComponent<MeshComponent>())
		{
			ourModel->SetTransform(GetComponent<TransformComponent>().Transform);
			ourModel->Update(ts, *ourShader);
		}
	}

	void GameObject::Render()
	{	
		SAT_PROFILE_FUNCTION();

		if(m_3D)
		{
			SAT_PROFILE_SCOPE("GameObjectRenderLoop");

			GameLayer* gl = Application::Get().m_gameLayer;
			if (HasComponent<MeshComponent>())
			{


				GetComponent<MeshComponent>().GetModel()->GetShader()->Bind();

				// view/projection transformations
				glm::mat4 projection = glm::perspective(glm::radians(Application::Get().m_gameLayer->m_3DCamera.Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);
				glm::mat4 view = Application::Get().m_gameLayer->m_3DCamera.GetViewMatrix();
				GetComponent<MeshComponent>().GetModel()->GetShader()->UploadMat4("projection", projection);
				GetComponent<MeshComponent>().GetModel()->GetShader()->UploadMat4("view", view);

				GetComponent<MeshComponent>().GetModel()->GetShader()->UploadMat4(
					"model",
					GetComponent<TransformComponent>().Transform
				);

				// material properties

				//Load the texture(s)
				unsigned int diffuseMap = loadTexture("assets/meshes/red.png");

				GetComponent<MeshComponent>().GetModel()->GetShader()->UploadInt("material.diffuse", diffuseMap);
				// bind diffuse map
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, diffuseMap);

				GetComponent<MeshComponent>().GetModel()->Draw(*GetComponent<MeshComponent>().GetModel()->GetShader());

			}
		}
	}


	GameObject* GameObject::SpawnGameObject()
	{
		return Application::Get().GetCurrentScene().CreateEntityGameObjectprt("");
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