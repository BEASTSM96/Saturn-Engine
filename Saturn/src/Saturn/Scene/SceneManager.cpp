#include "sppch.h"
#include "SceneManager.h"

namespace Saturn {

	SceneManager* SceneManager::s_Instance = nullptr;

	SceneManager::SceneManager()
	{
		SAT_CORE_ASSERT(!s_Instance, "SceneMananger already exists!");
	}

	SceneManager::~SceneManager()
	{
		for (int i = 0; i < m_Scenes.size(); i++)
		{
			//delete m_Scenes.at(i).Raw();
		}
	}

	void SceneManager::AddScene(const Ref<Scene>& scene)
	{
		SAT_CORE_ASSERT(scene, "Scene was not vaild (0xFFFFF(NULL))");
		m_Scenes.push_back(scene);
	}
}
