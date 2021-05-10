#include "sppch.h"
#include "PhysXRuntime.h"

namespace Saturn {

	static physx::PxScene* s_Scene;
	static float s_UpdateTime = 0.0f;

	void PhysXRuntime::Update( Timestep ts, Scene& scene )
	{
		s_UpdateTime += ts.GetMilliseconds();

		ForceSetRbUserData();

		s_Scene->simulate( ts );
		s_Scene->fetchResults( true );
		scene.PhysicsUpdate( PhysicsType::PhysX, ts );
	}

	void PhysXRuntime::CreateScene()
	{
		SAT_ASSERT( s_Scene == nullptr, "Scene already exist" );
		s_Scene = PhysXFnd::CreateScene();
	}

	physx::PxScene& PhysXRuntime::GetPhysXScene()
	{
		return *s_Scene;
	}

	void PhysXRuntime::CreatePhysXCompsForEntity( Entity entity )
	{
		PhysXFnd::AddRigidBody( entity );
	}

	void PhysXRuntime::Clear()
	{
		s_Scene = nullptr;
	}

	void PhysXRuntime::ForceSetRbUserData()
	{

	}

}
