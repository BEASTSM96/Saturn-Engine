#pragma once

#include "World.h"

#include "Saturn/Application.h"
#include "Saturn/GameBase/GameObject.h"

namespace Saturn {

	class SATURN_API GameObject;

	class SATURN_API Level : World
	{
	public:
		Level();
		~Level();

		// Inherited via World
		virtual void* GetLevel() override;

		virtual std::string GetLevelName() override;

		virtual uint64_t GetAllGameObjects() override;

		virtual void * GetGameLayer() override;

		virtual float GetID() override;




	private:
		float m_id = 0;

		std::string m_name = "testlevel1";
		std::vector<GameObject*> gameObjects;
	};
}