/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include "sppch.h"
#include "Character.h"

#include "Saturn/Renderer/Renderer.h"

#include "Saturn/Input.h"

namespace Saturn {

	Character::Character()
	{
	}

	Character::~Character()
	{
	}

	void Character::OnCreate()
	{
		SAT_INFO("Character::OnCreate");
	}

	void Character::OnDestroy()
	{

	}

	void Character::BeginPlay()
	{
		SAT_PROFILE_FUNCTION();
		SAT_INFO("Character::BeginPlay");

		std::string filepath = "assets/meshes/Cube1m.fbx";
		AddComponent<MeshComponent>().Mesh = Ref<Mesh>::Create(filepath);

	}

	void Character::ProcessInput( Timestep ts )
	{
		if( Input::IsKeyPressed( Key::W ) )
		{
			GetComponent<TransformComponent>().Position.z -= 90 * ts;
		}

		if( Input::IsKeyPressed( Key::A ) )
		{
			GetComponent<TransformComponent>().Position.x -= 90 * ts;
		}

		if( Input::IsKeyPressed( Key::S ) )
		{
			GetComponent<TransformComponent>().Position.z += 90 * ts;
		}

		if( Input::IsKeyPressed( Key::D ) )
		{
			GetComponent<TransformComponent>().Position.x += 90 * ts;
		}

		if ( Input::IsKeyPressed( Key::Space ) )
		{
			GetComponent<TransformComponent>().Position.y += 90 * ts;
		}

		if( Input::IsKeyPressed( Key::F ) )
		{
			auto mynewScriptableEntity = m_Scene->SpawnEntity<ScriptableEntity>( "Scriptable Entity", glm::vec3(0, 0, 0 ), glm::quat( 0, 0, 0, 0 ) );
		}

	}

	void Character::OnUpdate( Timestep ts )
	{
		SAT_PROFILE_FUNCTION();
		SAT_PROFILE_SCOPE("Character::OnUpdate");

		ProcessInput(ts);
	}

}
