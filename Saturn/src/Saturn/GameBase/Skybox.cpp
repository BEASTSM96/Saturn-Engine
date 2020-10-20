#include "sppch.h"
#include "Skybox.h"

#include "Saturn/Application.h"
#include "GameObject.h"

namespace Saturn {

	Skybox::Skybox() : Serialiser::OBJ_NAME("Skybox")
	{
	}

	Skybox::Skybox(const std::string& objectname, Json::Value& reconstructionValue) : Serialiser::OBJ_NAME("Skybox")
	{
	}

	Skybox::Skybox(entt::entity handle, Scene* scene) : Serialiser::OBJ_NAME("Skybox")
	{
	}

	void Skybox::Init()
	{
		SAT_PROFILE_FUNCTION();


		// tell stb_image.h to flip loaded texture's on the y-axis
		stbi_set_flip_vertically_on_load(true);

		GetComponent<TransformComponent>().Transform = glm::translate(transform, glm::vec3(1.0f, 10.0f, 10.0f));
		GetComponent<TransformComponent>().Transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));

		GameLayer* gl = static_cast<GameLayer*>(Application::Get().GetCurrentScene().GetLevel().GetGameLayer());

		gl->AddGameObjects(*this);

		SAT_CORE_WARN("GameObject Size {0} ", gl->GetGameObjectsSize());
	}

	void Skybox::Render()
	{
		SAT_PROFILE_SCOPE("SkyboxRenderLoop");

		GameLayer* gl = Application::Get().m_gameLayer;
		if (HasComponent<MeshComponent>())
		{
			GetComponent<MeshComponent>().GetModel()->GetShader()->Bind();

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(Application::Get().m_gameLayer->Get3DCamera().Zoom), (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), 0.1f, 100.0f);
			glm::mat4 view = Application::Get().m_gameLayer->Get3DCamera().GetViewMatrix();
			GetComponent<MeshComponent>().GetModel()->GetShader()->UploadMat4("projection", projection);
			GetComponent<MeshComponent>().GetModel()->GetShader()->UploadMat4("view", view);

			GetComponent<MeshComponent>().GetModel()->GetShader()->UploadMat4(
				"model",
				GetComponent<TransformComponent>().Transform
			);

			GetComponent<MeshComponent>().GetModel()->Draw(*GetComponent<MeshComponent>().GetModel()->GetShader());

			/////////////////////////////////////////////////////////////////////SKYBOX-TMP//////////////////////////////
		}
	}

	void Skybox::OnUpdate(Timestep ts)
	{
	}
}