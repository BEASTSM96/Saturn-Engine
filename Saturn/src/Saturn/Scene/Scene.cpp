#include "sppch.h"
#include "Scene.h"

#include "Entity.h"

#include "Components.h"

#include "Saturn/Renderer/SceneRenderer.h"

#include "SceneManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Saturn {

	static const std::string DefaultEntityName = "Entity";

	std::unordered_map<UUID, Scene*> s_ActiveScenes;

	struct SceneComponent
	{
		UUID SceneID;
	};

	Scene::Scene( void )
	{
		SAT_PROFILE_FUNCTION();

		auto skyboxShader = Shader::Create("assets/shaders/Skybox.glsl");
		m_SkyboxMaterial = MaterialInstance::Create(Material::Create(skyboxShader));
		m_SkyboxMaterial->SetFlag(MaterialFlag::DepthTest, false);

		m_physicsScene = std::make_shared<PhysicsScene>(this);
	}

	Scene::~Scene( void )
	{
		//s_ActiveScenes.erase(m_SceneID);
		m_Registry.clear();
	}

	static std::tuple<glm::vec3, glm::quat, glm::vec3> GetTransformDecomposition(const glm::mat4& transform)
	{
		glm::vec3 scale, translation, skew;
		glm::vec4 perspective;
		glm::quat orientation;
		glm::decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	void Scene::OnUpdate(Timestep ts)
	{
		m_physicsScene->Update(ts);


		{	
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if( !nsc.Instance )
				{
					nsc.InstantiateFunc();
					nsc.OnCreateFunc( nsc.Instance );

					nsc.OnBeignPlayFunc( nsc.Instance );
				}

				nsc.OnUpdateFunc( nsc.Instance, ts );

			} );
		}
	}

	void Scene::OnRenderEditor(Timestep ts, const EditorCamera& editorCamera)
	{

		/////////////////////////////////////////////////////////////////////
		// RENDER 3D SCENE
		/////////////////////////////////////////////////////////////////////
		m_SkyboxMaterial->Set("u_TextureLod", m_SkyboxLod);

		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		SceneRenderer::BeginScene(this, { editorCamera, editorCamera.GetViewMatrix() });
		for (auto entity : group)
		{
			auto& [meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>(entity);
			if (meshComponent.Mesh)
			{
				meshComponent.Mesh->OnUpdate(ts);
		
				// TODO: Should we render (logically)
		
				if (m_SelectedEntity == entity)
					SceneRenderer::SubmitSelectedMesh(meshComponent, transformComponent.GetTransform());
				else
					SceneRenderer::SubmitMesh(meshComponent, transformComponent.GetTransform());
			}
		}

		SceneRenderer::EndScene();

	}

	void Scene::OnRenderRuntime(Timestep ts)
	{
		/////////////////////////////////////////////////////////////////////
		// RENDER 3D SCENE
		/////////////////////////////////////////////////////////////////////
		//TODO: ADD!
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		SAT_PROFILE_FUNCTION();

		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned Entity" : name;

		auto& ID = entity.AddComponent<IdComponent>();

		return entity;
	}

	Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name, bool runtimeMap)
	{
		auto entity = Entity{ m_Registry.create(), this };
		auto& idComponent = entity.AddComponent<IdComponent>();
		idComponent.ID = uuid;

		entity.AddComponent<TransformComponent>();
		if (!name.empty())
			entity.AddComponent<TagComponent>(name);

		//SAT_CORE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end());
		m_EntityIDMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity.m_EntityHandle);
	}

	void Scene::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	void Scene::SetEnvironment(const Environment& environment)
	{
		m_Environment = environment;
		SetSkybox(environment.RadianceMap);
	}

	void Scene::SetSkybox(const Ref<TextureCube>& skybox)
	{
		m_SkyboxTexture = skybox;
		m_SkyboxMaterial->Set("u_Texture", skybox);
	}

	Entity Scene::GetMainCameraEntity( void )
	{
		//todo: add
		return {};
	}

	Environment Environment::Load(const std::string& filepath)
	{
		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return { filepath, radiance, irradiance };
	}

	void Scene::PhysicsUpdate(float delta)
	{
		auto view = m_Registry.view<TransformComponent, PhysicsComponent>();
		for (auto ent : view) {
			auto [transform, physics] = view.get<TransformComponent, PhysicsComponent>(ent);

			transform.Position = physics.rigidbody->GetPosition();
			transform.Rotation = physics.rigidbody->GetRotation();
		}
	}

	void Scene::ContactStay(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other) {
		auto view = m_Registry.view<TransformComponent, PhysicsComponent>();

		SAT_CORE_WARN("ContactStay");

		Entity curr;
		Entity otherEntity;

		for (auto entity : view)
		{
			auto [tc, pc] = view.get<TransformComponent, PhysicsComponent>(entity);

			if (pc.rigidbody->m_body == body)
			{
				curr = Entity(entity, this);
			}
			else if(pc.rigidbody->m_body == other)
			{
				otherEntity = Entity(entity, this);
			}
		}

	}

	void Scene::ContactEnter(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other) {
		auto view = m_Registry.view<TransformComponent, PhysicsComponent>();

		SAT_CORE_WARN("ContactEnter");

		Entity curr;
		Entity otherEntity;

		for (auto entity : view)
		{
			auto [tc, pc] = view.get<TransformComponent, PhysicsComponent>(entity);

			if (pc.rigidbody->m_body == body)
			{
				curr = Entity(entity, this);
			}
			else if (pc.rigidbody->m_body == other)
			{
				otherEntity = Entity(entity, this);
			}
		}

	}

	void Scene::ContactExit(reactphysics3d::CollisionBody* body, reactphysics3d::CollisionBody* other) {
		auto view = m_Registry.view<TransformComponent, PhysicsComponent>();

		SAT_CORE_WARN("ContactExit");

		Entity curr;
		Entity otherEntity;

		for (auto entity : view)
		{
			auto [tc, pc] = view.get<TransformComponent, PhysicsComponent>(entity);

			if (pc.rigidbody->m_body == body)
			{
				curr = Entity(entity, this);
			}
			else if (pc.rigidbody->m_body == other)
			{
				otherEntity = Entity(entity, this);
			}
		}

	}

	void Scene::Contact(rp3d::CollisionBody* body) {
		auto view = m_Registry.view<TransformComponent, PhysicsComponent>();
		for (auto ent : view) {
			auto [transform, physics] = view.get<TransformComponent, PhysicsComponent>(ent);

			if (physics.rigidbody->m_body == body) {
				SAT_CORE_INFO("collision");
				break;
			}
		}
	}

	/*------------------------ Runtime helpers ------------------------ */

	template<typename T>
	static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto components = srcRegistry.view<T>();
		for (auto srcEntity : components)
		{
			entt::entity destEntity = enttMap.at(srcRegistry.get<IdComponent>(srcEntity).ID);

			auto& srcComponent = srcRegistry.get<T>(srcEntity);
			auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
		}
	}

	template<typename T>
	static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
	{
		if (registry.has<T>(src))
		{
			auto& srcComponent = registry.get<T>(src);
			registry.emplace_or_replace<T>(dst, srcComponent);
		}
	}

	/**
	 * Copies the scene and returns the new scene
	*/
	Ref<Scene> Scene::CopyScene(const Ref<Scene>& CurrentScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		Ref<Scene> CScene = Ref<Scene>::Create();
		CScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		CScene->m_data = CurrentScene->m_data;
		CScene->m_DebugName = CurrentScene->m_DebugName;
		CScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		CScene->m_Environment = CurrentScene->m_Environment;
		CScene->m_Light = CurrentScene->m_Light;
		CScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		CScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		CScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		CScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		CScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		CScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		CScene->m_physicsScene = CurrentScene->m_physicsScene;
		CScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = CScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(CScene->m_Registry, m_Registry, enttMap);

		return CScene;
	}

	/**
	 * Copies the scene and returns the new scene
	*/
	Scene* Scene::CopyScene(const Scene*& CurrentScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		Scene* CScene = new Scene();
		CScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		CScene->m_data = CurrentScene->m_data;
		CScene->m_DebugName = CurrentScene->m_DebugName;
		CScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		CScene->m_Environment = CurrentScene->m_Environment;
		CScene->m_Light = CurrentScene->m_Light;
		CScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		CScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		CScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		CScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		CScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		CScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		CScene->m_physicsScene = CurrentScene->m_physicsScene;
		CScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = CScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(CScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(CScene->m_Registry, m_Registry, enttMap);

		return CScene;
	}

	/**
	 * Copies the scene and returns the new scene, and fills the 'NewScece' value
	*/
	Ref<Scene> Scene::CopyScene(const Ref<Scene>& CurrentScene, Ref<Scene> NewScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		NewScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		NewScene->m_data = CurrentScene->m_data;
		NewScene->m_DebugName = CurrentScene->m_DebugName;
		NewScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		NewScene->m_Environment = CurrentScene->m_Environment;
		NewScene->m_Light = CurrentScene->m_Light;
		NewScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		NewScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		NewScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		NewScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		NewScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		NewScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		NewScene->m_physicsScene = CurrentScene->m_physicsScene;
		NewScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = NewScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(NewScene->m_Registry, m_Registry, enttMap);

		return NewScene;
	}

	/**
	 * Copies the scene and returns the new scene, and fills the 'NewScece' value
	*/
	Scene* Scene::CopyScene(const Scene*& CurrentScene, Scene* NewScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		NewScene->m_CurrentLevel = CurrentScene->m_CurrentLevel;
		NewScene->m_data = CurrentScene->m_data;
		NewScene->m_DebugName = CurrentScene->m_DebugName;
		NewScene->m_EntityIDMap = CurrentScene->m_EntityIDMap;
		NewScene->m_Environment = CurrentScene->m_Environment;
		NewScene->m_Light = CurrentScene->m_Light;
		NewScene->m_LightMultiplier = CurrentScene->m_LightMultiplier;
		NewScene->m_SkyboxLod = CurrentScene->m_SkyboxLod;
		NewScene->m_SkyboxMaterial = CurrentScene->m_SkyboxMaterial;
		NewScene->m_SkyboxTexture = CurrentScene->m_SkyboxTexture;
		NewScene->m_ViewportHeight = CurrentScene->m_ViewportHeight;
		NewScene->m_ViewportWidth = CurrentScene->m_ViewportWidth;
		NewScene->m_physicsScene = CurrentScene->m_physicsScene;
		NewScene->m_RuntimeData = CurrentScene->m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = NewScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(NewScene->m_Registry, m_Registry, enttMap);

		return NewScene;
	}

	/**
	 * Copies the scene and fills the 'NewScece' value
	*/
	void Scene::CopyScene(Ref<Scene>& NewScene)
	{
		SAT_CORE_WARN("Copying Scene!");
		NewScene->m_CurrentLevel = m_CurrentLevel;
		NewScene->m_data = m_data;
		NewScene->m_DebugName = m_DebugName;
		NewScene->m_EntityIDMap = m_EntityIDMap;
		NewScene->m_Environment = m_Environment;
		NewScene->m_Light = m_Light;
		NewScene->m_LightMultiplier = m_LightMultiplier;
		NewScene->m_SkyboxLod = m_SkyboxLod;
		NewScene->m_SkyboxMaterial = m_SkyboxMaterial;
		NewScene->m_SkyboxTexture = m_SkyboxTexture;
		NewScene->m_ViewportHeight = m_ViewportHeight;
		NewScene->m_ViewportWidth = m_ViewportWidth;
		NewScene->m_physicsScene = m_physicsScene;
		NewScene->m_RuntimeData = m_RuntimeData;

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IdComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IdComponent>(entity).ID;
			Entity e = NewScene->CreateEntityWithID(uuid, "", true);
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<RelationshipComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(NewScene->m_Registry, m_Registry, enttMap);
		CopyComponent<SpriteRendererComponent>(NewScene->m_Registry, m_Registry, enttMap);

	}

	void Scene::BeginRuntime( void )
	{
		SAT_CORE_WARN("[Runtime] Begining!");
		StartRuntime();
	}

	void Scene::StartRuntime( void )
	{
		SAT_CORE_WARN("[Runtime] Starting!");
		m_RuntimeRunning = true;
	}

	void Scene::UpdateRuntime( void )
	{
		SAT_CORE_WARN("Updating Runtime!");
		auto view = m_Registry.view<TransformComponent, PhysicsComponent /*, TODO: When add other comps */>();

		for (auto entity : view)
		{
			auto [tc, pc] = view.get<TransformComponent, PhysicsComponent>(entity);

			tc.Position = pc.rigidbody->GetPosition();
			tc.Rotation = pc.rigidbody->GetRotation();

		}

	}

}