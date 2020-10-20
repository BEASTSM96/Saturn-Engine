#include "sppch.h"
#include "PhysicsActor.h"
#include "physx/PxPhysicsAPI.h"

namespace Saturn::Physics::Actor {

	PhysicsActor::PhysicsActor()
	{
	}

	PhysicsActor::~PhysicsActor()
	{
	}

	void PhysicsActor::InitPhysics(bool isinteractive)
	{
		m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);

		m_Pvd = PxCreatePvd(*m_Foundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		m_Pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, physx::PxTolerancesScale(), true, m_Pvd);

		physx::PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = m_Dispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		m_PhysXScene = m_Physics->createScene(sceneDesc);

		m_Material = m_Physics->createMaterial(0.5f, 0.5f, 0.6f);

		physx::PxRigidStatic* groundPlane = PxCreatePlane(*m_Physics, physx::PxPlane(0, 1, 0, 0), *m_Material);
		m_PhysXScene->addActor(*groundPlane);
	}

	void PhysicsActor::Cleanup(bool)
	{
		RELEASE(m_PhysXScene);
		RELEASE(m_Dispatcher);
		RELEASE(m_Physics);
		if (m_Pvd)
		{
			physx::PxPvdTransport* transport = m_Pvd->getTransport();
			m_Pvd->release();	
			m_Pvd = NULL;
			RELEASE(transport);
		}
		RELEASE(m_Foundation);
	}

	physx::PxRigidDynamic* PhysicsActor::CreateDynamic(const physx::PxTransform& t, const physx::PxGeometry& geometry, const physx::PxVec3& velocity)
	{
		physx::PxRigidDynamic* dynamic = physx::PxCreateDynamic(*m_Physics, t, geometry, *m_Material, 10.0f);
		dynamic->setAngularDamping(0.5f);
		dynamic->setLinearVelocity(velocity);
		m_PhysXScene->addActor(*dynamic);
		return dynamic;
	}

}

