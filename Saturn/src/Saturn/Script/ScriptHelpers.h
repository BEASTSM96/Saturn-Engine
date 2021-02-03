#pragma once

#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"

namespace Saturn {

	class ScriptHelpers
	{
	public:
		static EntityMap GetEntityMap( Ref<Scene>& scene, uint32_t entityID )
		{
			auto& map = scene->GetEntityMap();
			SAT_CORE_ASSERT( map.find( entityID ) != map.end() );
			return map;
		}

		template<typename Ty>
		static Ty& GetCompFromScene( Ref<Scene>& scene, uint32_t entityID )
		{
			auto& map = scene->GetEntityMap();
			SAT_CORE_ASSERT( map.find( entityID ) != map.end() );
			Entity e = map.at( entityID );
			return e.GetComponent<Ty>();
		}
	protected:
	private:
	};


}