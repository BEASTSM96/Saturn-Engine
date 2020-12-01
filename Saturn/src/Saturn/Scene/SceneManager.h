#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Core/Ref.h"

namespace Saturn {

	class Scene;

	class SceneManager : public RefCounted
	{
	public:
		SceneManager();
		~SceneManager();

		void AddScene( const Ref< Scene >& scene );

		std::vector < Ref< Scene > >& GetScenes() { return m_Scenes;  }
		const std::vector < Ref< Scene > >& GetScenes() const { return m_Scenes; }

	protected:
		std::vector< Ref < Scene > >m_Scenes;

	private:
		static SceneManager* s_Instance;
	};
}