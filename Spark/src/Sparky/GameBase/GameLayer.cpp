#include "sppch.h"
#include "GameLayer.h"

#include "Sparky/Layer.h"

#include "Sparky/Input.h"

#include "Sparky/KeyCodes.h"


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
		}

		Renderer::EndScene();
	}


	void GameLayer::Sumbit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, FTransform Intransform) {
		Renderer::Submit(shader, vertexArray, Intransform);
	}

	void GameLayer::OnEvent(Event& event)
	{
	}

	void GameLayer::AddGameObjects(GameObject gameObject)
	{
		gameObjects.emplace_back(gameObject);
	}
}
#endif // SPARKY_GAME_BASE