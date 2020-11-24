#pragma once

#include "entt.hpp"

#include "Saturn/Core/Ref.h"
#include "Saturn/Log.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Renderer/Material.h"
#include "Saturn/Editor/EditorCamera.h"
#include "Saturn/Core/UUID.h"

#include "Saturn/Physics/PhysicsScene.h"

#include <reactphysics3d/reactphysics3d.h>

namespace Saturn {

	struct SceneData {
		UUID SceneID;
		std::string name;
		RefSR<Texture2D> m_SkyboxTexture;
		RefSR<Material> m_SkyboxMaterial;
	};

	struct Environment
	{
		std::string FilePath;
		Ref<TextureCube> RadianceMap;
		Ref<TextureCube> IrradianceMap;

		static Environment Load(const std::string& filepath);
	};

	struct Light
	{
		glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Radiance = { 0.0f, -1.0f, 0.0f };

		float Multiplier = 1.0f;
	};

	class Level;
	class Entity;
	class GameObject;

	using EntityMap = std::unordered_map<UUID, Entity>;

	class Scene : public RefCounted
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithID(UUID uuid, const std::string& name = "");
		GameObject CreateEntityGameObject(const std::string& name = std::string());
		GameObject * CreateEntityGameObjectprt(const std::string& name, const std::vector<std::string> ShaderPaths, std::string ObjectPath = std::string());
		void DestroyEntity(Entity entity);
		void DestroyGameObject(GameObject entity);
		void DestroyGameObject(GameObject * entity);

		void OnRenderEditor(Timestep ts, const EditorCamera& editorCamera);

		template<typename T>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<T>();
		}

		PhysicsScene* GetPhysicsScene() {
			return &*m_physicsScene;
		}

		SceneData& GetData() { return m_data; }
		Level& GetLevel() { return *m_CurrentLevel; }
		entt::registry& GetRegistry() { return m_Registry; }

		void OnUpdate(Timestep ts);
		void SetViewportSize(uint32_t width, uint32_t height);

		void SetEnvironment(const Environment& environment);
		const Environment& GetEnvironment() const { return m_Environment; }
		void SetSkybox(const Ref<TextureCube>& skybox);

		Light& GetLight() { return m_Light; }
		const Light& GetLight() const { return m_Light; }

		Entity GetMainCameraEntity();

		float& GetSkyboxLod() { return m_SkyboxLod; }

		void DuplicateEntity(Entity entity);

		// Editor-specific
		void SetSelectedEntity(entt::entity entity) { m_SelectedEntity = entity; }

		//phys
		void PhysicsUpdate(float delta);
		void ContactStay(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);
		void ContactEnter(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);
		void ContactExit(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other);

		void PhysicsComponentCreate(entt::registry& r, entt::entity ent);

		void Contact(rp3d::CollisionBody* body);

		std::shared_ptr<PhysicsScene> m_physicsScene;
	private:
		UUID m_SceneID;
		entt::entity m_SceneEntity;
		entt::registry m_Registry;

		EntityMap m_EntityIDMap;

		std::string m_DebugName;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		Environment m_Environment;
		Ref<TextureCube> m_SkyboxTexture;
		Ref<MaterialInstance> m_SkyboxMaterial;

		entt::entity m_SelectedEntity;

		float m_SkyboxLod = 1.0f;

		Light m_Light;
		float m_LightMultiplier = 0.3f;

		SceneData m_data;

		Level* m_CurrentLevel;

		friend class  Entity;
		friend class  GameObject;
		friend class  SceneRenderer;
		friend class  Serialiser;
		friend class  SceneHierarchyPanel;

	};
}