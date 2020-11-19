#include "sppch.h"
#include "Scene.h"

#include "Components.h"

#include <glm/glm.hpp>
#include "Entity.h"
#include "Saturn/Core/Math/Math.h"
#include "Saturn/GameBase/GameObject.h"
#include "Saturn/Core/World/Level.h"
#include "Saturn/Renderer/SceneRenderer.h"

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

	Scene::Scene()
	{
		SAT_PROFILE_FUNCTION();

		auto skyboxShader = Shader::Create("assets/shaders/Skybox.glsl");
		m_SkyboxMaterial = MaterialInstance::Create(Material::Create(skyboxShader));
		m_SkyboxMaterial->SetFlag(MaterialFlag::DepthTest, false);

	}

	Scene::~Scene()
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

	GameObject Scene::CreateEntityGameObject(const std::string& name)
	{
		SAT_PROFILE_FUNCTION();

		GameObject entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned GameObject" : name;

		auto& ID = entity.AddComponent<IdComponent>();

		return entity;
	}

	GameObject* Scene::CreateEntityGameObjectprt(const std::string& name, const std::vector<std::string> ShaderPaths, std::string ObjectPath)
	{
		SAT_PROFILE_FUNCTION();

		GameObject* entity = new GameObject(m_Registry.create(), this);

		entity->AddComponent<TransformComponent>();

		auto& tag = entity->AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Unmanned GameObject" : name;

		auto& ID = entity->AddComponent<IdComponent>();

		if (ObjectPath.empty()) {
			std::string name = "assets/meshes/CUBE.fbx";
			ObjectPath = name;
		}

		entity->Init();

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity.m_EntityHandle);
	}

	void Scene::DestroyGameObject(GameObject entity)
	{
		m_Registry.destroy(entity.m_EntityHandle);
	}

	void Scene::DestroyGameObject(GameObject* entity)
	{
		m_Registry.destroy(entity->m_EntityHandle);
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

	Entity Scene::GetMainCameraEntity()
	{
		//todo: add
		return {};
	}

	Environment Environment::Load(const std::string& filepath)
	{
		auto [radiance, irradiance] = SceneRenderer::CreateEnvironmentMap(filepath);
		return { filepath, radiance, irradiance };
	}
}