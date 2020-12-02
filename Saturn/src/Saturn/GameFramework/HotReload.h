#pragma once

#include "Saturn/Core/Base.h"
#include <string>
#include <vector>

#include "Game.h"

namespace Saturn {

	struct HotReloadContext {
		std::string ContextName;
		std::vector< GameContext > GameContexts;
		Ref < GameContext > GameContext;

	};

	struct HotReloadStruct {
		HotReloadContext m_HotReloadContext;
	};

	class HotReload : public RefCounted
	{
	public:
		HotReload();
		~HotReload();

		void OnHotReload();
		void OnHotReload( char name );

		Ref<GameContext> m_GameContext;

		HotReloadStruct m_HotReloadStruct;
	public:
		static HotReload& Get() { return *s_Instance; }
	private:

		static HotReload* s_Instance;
	};
}
