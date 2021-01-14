#include "sppch.h"
#include "PhysXScene.h"

#include "Saturn/Scene/Scene.h"
#include "Saturn/Renderer/Renderer.h"

namespace Saturn {

	PhysXScene::PhysXScene( Scene* scene ) : m_Scene( scene )
	{
		m_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, m_DefaultAllocatorCallback, m_DefaultErrorCallback );

		if( !m_Cooking )
		{
			physx::PxTolerancesScale ToleranceScale;
			m_Cooking = PxCreateCooking( PX_PHYSICS_VERSION, *m_Foundation, physx::PxCookingParams( ToleranceScale ) );
		}
		physx::PxCookingParams cookingParameters = m_Cooking->getParams();
		cookingParameters.meshPreprocessParams.set( physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH );
		m_Cooking->setParams( cookingParameters );

		if( !m_Physics )
		{
			physx::PxTolerancesScale ToleranceScale;
			m_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *m_Foundation, ToleranceScale );
		}

		physx::PxSceneDesc sceneDesc( m_Physics->getTolerancesScale() );
		sceneDesc.gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate( 2 );
		sceneDesc.cpuDispatcher	= m_Dispatcher;
		sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;
		m_PhysXScene = m_Physics->createScene( sceneDesc );

	#if 0

		Renderer::Submit( [=]
		{
			 physx::PxMaterial* m_Material = NULL;
			 m_Material = m_Physics->createMaterial( 0.5f, 0.5f, 0.6f );
			 physx::PxRigidStatic* groundPlane = PxCreatePlane( *m_Physics, physx::PxPlane( 0, 1, 0, 0 ), *m_Material );
			 m_PhysXScene->addActor( *groundPlane );
		} );

		
	#endif
	}

	void PhysXScene::Update( Timestep ts )
	{
		m_Scene->PhysicsUpdate( PhysicsType::PhysX, ts );
		m_PhysXScene->simulate( ts, nullptr, MemoryBlock, sizeof(MemoryBlock) );
		m_PhysXScene->fetchResults( true );
	}

	PhysXScene::~PhysXScene()
	{

	}

}