#pragma once


#include "Saturn/Core.h"
#include "GameObject.h"

namespace Saturn {

	class Skybox : public GameObject
	{
	public:
		Skybox();
		//////////////////////////////////////////////////////////////////////////////

		Skybox(entt::entity handle, Scene* scene);
		Skybox(const Skybox& other) = default;

		virtual ~Skybox() = default;
		//////////////////////////////////////////////////////////////////////////////////


		void Render() override;
		void Init() override;

		void OnUpdate(Timestep ts) override;

	private:
		glm::mat4& transform = glm::mat4(1.0f);

		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
}