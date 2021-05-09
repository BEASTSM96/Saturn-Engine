#pragma once

#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"

namespace Saturn {

	class ScriptHelpers
	{
	public:
		static EntityMap GetEntityMap( Ref<Scene>& scene, uint64_t entityID )
		{
			UUID id = entityID;
			const auto& map = scene->GetEntityMap();
			SAT_CORE_ASSERT( map.find( id ) != map.end() );
			return map;
		}

		template<typename Ty>
		static Ty& GetCompFromScene( Ref<Scene>& scene, uint64_t entityID )
		{
			UUID id = entityID;
			const auto& map = scene->GetEntityMap();
			SAT_CORE_ASSERT( map.find( id ) != map.end() );
			Entity e = map.at( id );
			return e.GetComponent<Ty>();
		}
	protected:
	private:
	};


}