#include "sppch.h"
#include "SceneMananger.h"

namespace Saturn {

	SceneMananger* SceneMananger::s_Instance = nullptr;

	SceneMananger::SceneMananger()
	{
		SAT_CORE_ASSERT(!s_Instance, "SceneMananger already exists!");
	}

	SceneMananger::~SceneMananger()
	{
		for (int i = 0; i < m_Scenes.size(); i++)
		{
			delete m_Scenes.at(i).Raw();
		}
	}

	void SceneMananger::AddScene(const Ref<Scene>& scene)
	{
		SAT_CORE_ASSERT(scene, "Scene was not vaild (0xFFFFF(NULL))");
		m_Scenes.push_back(scene);
	}
}
