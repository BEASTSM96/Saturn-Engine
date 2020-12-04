
#if !0

/*MIT License

Copyright (c) 2020 BEAST

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Saturn/Log.h>
#include <entt.hpp>
#include <unordered_set>
#include <Saturn/Renderer/Mesh.h>
#include <Saturn/Scene/Components.h>
#include <Saturn/GameFramework/Game.h>

namespace game {

	class GGame : public Saturn::Game {
	public:
		GGame();
		~GGame();

		//virtual void ConfigGame( Ref<Scene> runtimeScece ) override;


		void ConfigGame( Saturn::Ref<Saturn::Scene> runtimeScece ) override;

	};

	GGame::GGame()
	{

	}

	GGame::~GGame()
	{

	}

	void GGame::ConfigGame( Saturn::Ref<Saturn::Scene> runtimeScece )
	{
		auto entities =  runtimeScece->GetRegistry().view<Saturn::TransformComponent>();
		for (auto& entity : entities)
		{
			auto& transform = entities.get<Saturn::TransformComponent>(entity);



		}

	}

	class debug
	{
	public:
		debug();
		~debug();

		void log( char msg );
		void logEx( char msg );
	private:

	};

	debug::debug()
	{
	}

	debug::~debug()
	{
	}

	void debug::log( char msg )
	{
		logEx(msg);
	}

	void debug::logEx( char msg )
	{
		__debugbreak();
	}

}

#endif