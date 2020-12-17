#pragma once

#include "Core.h"
#include "PhysicsObject.h"

#include <vector>

#include "Saturn/Core/Ref.h"
#include "Saturn/Core/Timestep.h"

namespace PHYSICNAMESPACE {

	class PhysicsWorld
	{
	public:

		void AddObject( Saturn::Ref<Object>* Object ) 
		{
			m_Objects.push_back( Object );
		}

		void RemoveObject( Saturn::Ref<Object>* Object ) 
		{
			if( !Object ) return;
			auto itr = std::find( m_Objects.begin(), m_Objects.end(), Object );
			if( itr == m_Objects.end() ) return;
			m_Objects.erase( itr );
		}

		void Step( Saturn::Timestep ts ) 
		{
			for( Saturn::Ref<Object>* object : m_Objects )
			{
				object->Raw()->Force += object->Raw()->Mass * m_Gravity;

				object->Raw()->Velocity += object->Raw()->Force / object->Raw()->Mass * ts.GetSeconds();

				object->Raw()->Position += object->Raw()->Velocity;

				object->Raw()->Force = glm::vec3(0, 0, 0);

			}

		}

	protected:
	private:
		std::vector<Saturn::Ref<Object>*> m_Objects;
		glm::vec3 m_Gravity = glm::vec3( 0, -9.81f, 0 );
	};


}