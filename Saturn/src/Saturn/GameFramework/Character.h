#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Scene/ScriptableEntity.h"

namespace Saturn {

	class Character : public ScriptableEntity
	{
	public:
		Character();
		~Character();
	
		void OnCreate() override;
		void OnDestroy() override;
		void OnUpdate( Timestep ts ) override;
		void BeginPlay() override;
	private:
		void ProcessInput( Timestep ts );

	private:
		friend class SceneHierarchyPanel;
	};

}