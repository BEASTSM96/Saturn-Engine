#include "sppch.h"
#include "PhysXSimulationEventCallback.h"

#include <physx/PxPhysicsAPI.h>

namespace Saturn {

	PhysXSimulationEventCallback::PhysXSimulationEventCallback( physx::PxSceneDesc PxSceneDesc )
	{
		PxSceneDesc.simulationEventCallback = this;
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
		if( pairHeader.flags.isSet( physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0 ) || pairHeader.flags.isSet( physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1 ) )
		{
			SAT_CORE_ERROR( "pairHeader.flags.isSet( PxContactPairHeaderFlag::eREMOVED_ACTOR_0 ) || pairHeader.flags.isSet( PxContactPairHeaderFlag::eREMOVED_ACTOR_1 )" );
			return;
		}
		if( pairHeader.actors[ 0 ]->userData == nullptr || pairHeader.actors[ 1 ]->userData == nullptr )
			return;
		for ( physx::PxU32 index = 0; index < nbPairs; ++index )
		{
			const physx::PxContactPair& contactPair = pairs[index];
			if ( contactPair.flags.isSet( physx::PxContactPairFlag::eREMOVED_SHAPE_0 ) || contactPair.flags.isSet( physx::PxContactPairFlag::eREMOVED_SHAPE_1 ) )
			{
				SAT_CORE_INFO( "contactPair.flags.isSet( PxContactPairFlag::eREMOVED_SHAPE_0 ) || contactPair.flags.isSet( PxContactPairFlag::eREMOVED_SHAPE_1 )" );
				continue;
			}
			if( contactPair.flags.isSet( physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH ) ) 
			{
				//TODO: Add ContactType
			}
			if( contactPair.flags.isSet( physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH ) )
			{
				//TODO: Add ContactType
			}
			physx::PxContactStreamIterator iterator( contactPair.contactPatches, contactPair.contactPoints, contactPair.getInternalFaceIndices(), contactPair.patchCount, contactPair.contactCount );
			while( iterator.hasNextPatch() )
			{
				iterator.nextPatch();
				while( iterator.hasNextContact() ) 
				{
					iterator.nextContact();
					const physx::PxVec3 position = iterator.getContactPoint();
					const physx::PxVec3 normal = iterator.getContactNormal();
				}
			}

		}
	}

	void PhysXSimulationEventCallback::onTrigger( physx::PxTriggerPair* pairs, physx::PxU32 count )
	{

	}

	void PhysXSimulationEventCallback::onAdvance( const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count )
	{
		return;
	}

}
