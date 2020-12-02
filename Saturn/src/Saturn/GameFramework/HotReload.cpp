#include "sppch.h"
#include "HotReload.h"

#include "Game.h"

namespace Saturn {

	HotReload* HotReload::s_Instance = nullptr;

	HotReload::HotReload()
	{
		s_Instance = this;

		m_GameContext = Ref<GameContext>::Create();
	}

	HotReload::~HotReload()
	{

	}

	void HotReload::OnHotReload( char name )
	{
		m_GameContext->CompileGame( name );
	}

	void HotReload::OnHotReload()
	{
		m_GameContext->CompileAllGames();
	}

}