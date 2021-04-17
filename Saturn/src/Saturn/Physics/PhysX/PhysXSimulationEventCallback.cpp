#include "sppch.h"
#include "PhysXSimulationEventCallback.h"

#include <physx/PxPhysicsAPI.h>

#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"

namespace Saturn {

	PhysXSimulationEventCallback::PhysXSimulationEventCallback()
	{
		SetSceneContext( nullptr );
	}

	void PhysXSimulationEventCallback::SetSceneContext( PhysXScene* scene )
	{
		m_Scene = scene;
	}

	PhysXSimulationEventCallback::~PhysXSimulationEventCallback()
	{
		if( m_Scene )
			m_Scene->m_PhysXScene->setSimulationEventCallback( nullptr );
	}

	void PhysXSimulationEventCallback::onConstraintBreak( physx::PxConstraintInfo* constraints, physx::PxU32 count )
	{

	}

	void PhysXSimulationEventCallback::onWake( physx::PxActor** actors, physx::PxU32 count )
	{
		m_OnWake;
	}

	void PhysXSimulationEventCallback::onSleep( physx::PxActor** actors, physx::PxU32 count )
	{
		m_OnSleep;
	}

	void PhysXSimulationEventCallback::onContact( const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs )
	{
		Entity& a = *( Entity* )pairHeader.actors[ 0 ]->userData;
		Entity& b = *( Entity* )pairHeader.actors[ 1 ]->userData;

		SAT_CORE_INFO( "onContact" );

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionBegin( a );
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionBegin( b );
		}

		if( pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH )
		{
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionExit( a );
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionExit( b );
		}
	}

	void PhysXSimulationEventCallback::onTrigger( physx::PxTriggerPair* pairs, physx::PxU32 count )
	{
		Entity& a = *( Entity* )pairs->triggerActor->userData;
		Entity& b = *( Entity* )pairs->otherActor->userData;

		SAT_CORE_INFO( "onContact" );

		if( pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND )
		{
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionBegin( a );
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionBegin( b );
		}

		if( pairs->status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST )
		{
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionExit( a );
			if( a.HasComponent<ScriptComponent>() && ScriptEngine::ModuleExists( a.GetComponent<ScriptComponent>().ModuleName ) )
				ScriptEngine::OnCollisionExit( b );
		}
	}

	void PhysXSimulationEventCallback::onAdvance( const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count )
	{
		return;
	}

}
