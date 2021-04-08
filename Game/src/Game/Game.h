#pragma once

#include "Saturn/Log.h"
#include "Saturn/Core/Base.h"
#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Scene.h"

extern "C" {

	class TestClass
	{
	public:
	protected:
	private:
	};

	__declspec( dllexport ) void test();
	__declspec( dllexport ) void SceneInit( Saturn::Ref<Saturn::Scene> scene );
};