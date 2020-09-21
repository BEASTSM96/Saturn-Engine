#pragma once

#include "World.h"

#include "Saturn/Application.h"

namespace Saturn {

	class GameObject;
	class GameLayer;

	class SATURN_API Level : World
	{
	public:
		Level();
		~Level();

		// Inherited via World
		virtual void* GetLevel() override;

		virtual std::string GetLevelName() override;

		virtual uint64_t GetAllGameObjects() override;

		GameLayer* CreateGameLayer();

		virtual void * GetGameLayer() override;

		virtual float GetID() override;

	private:
		float m_id = 0;

		std::string m_name = "testlevel1";
		std::vector<GameObject*> gameObjects;

		GameLayer* m_Gamelayer;

	};
}