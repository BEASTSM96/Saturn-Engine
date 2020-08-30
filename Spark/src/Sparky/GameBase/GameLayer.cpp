#include "sppch.h"
#include "GameLayer.h"

#include "Sparky/Layer.h"

#include "Sparky/Input.h"

#include "Sparky/KeyCodes.h"

#include "Sparky/Core.h"


#ifdef SPARKY_GAME_BASE
/*  THIS CLASS WILL PROB BE THE BATCH RENDERER!*/

namespace Sparky {
	

	GameLayer* GameLayer::s_Instance = nullptr;


	Sparky::GameLayer::GameLayer() : Layer("GameLayer") ,  m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		SP_CORE_ASSERT(!s_Instance, "More than one Game Layer already exists!");

		s_Instance = this;
	}

	GameLayer::~GameLayer()
	{
	}

	void GameLayer::OnUpdate(Timestep ts)
	{
		RenderCommand::SetClearColor({ 1, 1, 0, 0 });
		RenderCommand::Clear();

		Renderer::BeginScene(m_Camera);

		for (GameObject gb : gameObjects)
		{
			gb.Render();
			gb.OnUpdate(ts);
		}

		//OnGameObjectMove(gameObjects.at(0).m_PlayerPosition);

		//m_Camera.SetPosition(gameObjects.at(0).m_PlayerPosition);

		Renderer::EndScene();
	}

	void GameLayer::Sumbit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, FTransform Intransform) {
		Renderer::Submit(shader, vertexArray, Intransform);
	}

	void GameLayer::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(SP_BIND_EVENT_FN(GameLayer::OnKeyPressed));
	}

	bool GameLayer::OnKeyPressed(KeyPressedEvent& event)
	{

		for (GameObject i : gameObjects)
		{
			i.OnKeyInput(event);

			return true;
		}

		return false;

	}

	void GameLayer::OnGameObjectMove(glm::vec3& position)
	{
		m_Camera.SetPosition(position);
	}

	void GameLayer::AddGameObjects(GameObject gameObject)
	{
		gameObjects.push_back(gameObject);
	}
}
#endif // SPARKY_GAME_BASE