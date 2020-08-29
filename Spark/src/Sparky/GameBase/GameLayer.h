#pragma once

/*
*					Sparky GameLayer
*********************************************************************************************************
*										Layer.
*/

#ifdef SPARKY_GAME_BASE

#include "Sparky/Layer.h"

#include "Sparky/Core/Serialisation/Object.h"

#include "Sparky/Renderer/OrthographicCamera.h"

#include "Sparky/Renderer/Renderer.h"

#include<string>

#include "Sparky/GameBase/GameObject.h"

#include "Sparky/Application.h"

namespace Sparky {
	class SPARKY_API GameLayer : public Layer
	{
	public:

		friend class GameObject;

		GameLayer();
		~GameLayer();

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& event) override;

		void Sumbit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, FTransform Intransform);

		void AddGameObjects(GameObject gameObject);

		inline const std::string& GetName() const { return m_DebugName; }

		inline std::vector<GameObject> GetGameObjects() { return gameObjects; }

		inline size_t GetGameObjectsSize() { return gameObjects.size(); }

		inline uint64_t GetGameObjectsSizeUint64() { return gameObjects.size(); }


	protected:
		std::string m_DebugName = "GameLayer";

		OrthographicCamera m_Camera;

	private:
		static GameLayer* s_Instance;

		std::vector<GameObject> gameObjects;

		Json::Value serialiser;

	};
}

//#else
//#error Error!, you are trying to call Player func's while 'SPARKY_GAME_BASE' is not defined!
#endif // SPARKY_GAME_BASE