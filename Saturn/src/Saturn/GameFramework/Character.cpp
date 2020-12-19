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

	}

	void Character::ProcessInput( Timestep ts )
	{
		if( Input::IsKeyPressed( Key::W ) )
		{
			GetComponent<TransformComponent>().Position.y += 90 * ts;
		}

		if( Input::IsKeyPressed( Key::A ) )
		{
			GetComponent<TransformComponent>().Position.x -= 90 * ts;
		}

		if( Input::IsKeyPressed( Key::S ) )
		{
			GetComponent<TransformComponent>().Position.y -= 90 * ts;
		}

		if( Input::IsKeyPressed( Key::D ) )
		{
			GetComponent<TransformComponent>().Position.x += 90 * ts;
		}

	}

	void Character::OnUpdate( Timestep ts )
	{
		SAT_PROFILE_FUNCTION();
		SAT_PROFILE_SCOPE("Character::OnUpdate");

		ProcessInput(ts);

		SAT_INFO("{0}", GetComponent<TransformComponent>().Position.y);
	}

}
