#include "sppch.h"
#include "PhysicsActor.h"
#include "physx/PxPhysicsAPI.h"

namespace Saturn::Physics::Actor {

	PhysicsActor::PhysicsActor()
	{
		InitPhysics(false);
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

	void PhysicsActor::StepPhysics(bool)
	{
		m_PhysXScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
		m_PhysXScene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES, 2.0f);

		m_PhysXScene->simulate(1.0f / 60.0f);
		m_PhysXScene->fetchResults(true);
		static bool xdone = false;
		if (!xdone)
		{
			physx::PxReal loc = 10.0f;
			CreateStack(physx::PxTransform(physx::PxVec3(0, 0, loc -= 10.0f)), 10, 2.0f);
			xdone = true;
		}
	}

	void PhysicsActor::Cleanup(bool)
	{
		RELEASE(m_PhysXScene);
		RELEASE(m_Dispatcher);
		RELEASE(m_Physics);
		RELEASE(m_Foundation);

		SAT_CORE_WARN("[PhysicsActor] Scene Cleanup!");
	}

	physx::PxRigidDynamic* PhysicsActor::CreateDynamic(const physx::PxTransform& t, const physx::PxGeometry& geometry, const physx::PxVec3& velocity)
	{
		physx::PxRigidDynamic* dynamic = physx::PxCreateDynamic(*m_Physics, t, geometry, *m_Material, 10.0f);
		dynamic->setAngularDamping(0.5f);
		dynamic->setLinearVelocity(velocity);
		m_PhysXScene->addActor(*dynamic);
		return dynamic;
	}

	void PhysicsActor::CreateStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent)
	{
		physx::PxShape* shape = m_Physics->createShape(physx::PxBoxGeometry(halfExtent, halfExtent, halfExtent), *m_Material);
		for (physx::PxU32 i = 0; i < size; i++)
		{
			for (physx::PxU32 j = 0; j < size - i; j++)
			{

				physx::PxTransform localTm(physx::PxVec3(physx::PxReal(j * 2) - physx::PxReal(size - i), physx::PxReal(i * 2 + 1), 0) * halfExtent);
				physx::PxRigidDynamic* body = m_Physics->createRigidDynamic(t.transform(localTm));
				body->attachShape(*shape);
				physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
				m_PhysXScene->addActor(*body);
			}
		}
		shape->release();

	}

}

