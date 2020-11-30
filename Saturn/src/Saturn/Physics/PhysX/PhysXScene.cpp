#include "sppch.h"
#include "PhysXScene.h"

namespace Saturn {

	PhysXScene::PhysXScene(Scene* scene) : m_Scene(scene)
	{
		m_PhysXScene = new physx::PxScene();
	}

	PhysXScene::~PhysXScene()
	{

	}

}