#pragma once

#include "Core.h"

#include <vector>

#include "Saturn/Core/Ref.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Components.h"

namespace Saturn {

	class PhysicsWorld
	{
	public:
		void Step( Timestep ts ) 
		{

			auto view = m_Scene->GetRegistry().view<TransformComponent, PhysicsComponent>();

			for( const auto& entity : view )
			{
				auto [tc, pc] = view.get<TransformComponent, PhysicsComponent>( entity );


				pc.Force += pc.Mass * m_Gravity;

				pc.Velocity += pc.Force / pc.Mass * ts.GetSeconds();

				pc.Position += pc.Velocity;

				pc.Force = glm::vec3( 0, 0, 0 );

			}
		}

	protected:
		Ref<Scene> m_Scene;

	private:
		std::vector<PhysicsComponent> m_PhysicsComponents;
		glm::vec3 m_Gravity = glm::vec3( 0, -9.81f, 0 );
	};


}