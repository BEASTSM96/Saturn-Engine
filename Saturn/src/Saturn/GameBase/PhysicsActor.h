#pragma once

#include <ctype.h>

#include "physx/PxFoundation.h"
#include "physx/PxPhysics.h"
#include "physx/extensions/PxDefaultCpuDispatcher.h"
#include "physx/PxScene.h"
#include "physx/PxFoundation.h"
#include "physx/extensions/PxDefaultAllocator.h"
#include "physx/extensions/PxDefaultErrorCallback.h"
#include "physx/pvd/PxPvd.h"
#include "physx/foundation/PxPreprocessor.h"
#include "physx/foundation/PxSimpleTypes.h"
#include "physx/PxFoundation.h"
#include "physx/foundation/PxTransform.h"

namespace Saturn::Physics {

	#define RELEASE(x)	if(x)	{ x->release(); x = NULL;	}

	namespace Actor {
	
		class PhysicsActor
		{
		public:
			PhysicsActor();
			~PhysicsActor();

			void InitPhysics(bool isinteractive);
			void StepPhysics(bool);
			void Cleanup(bool);
			void CreateStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent);
			physx::PxRigidDynamic* PhysicsActor::CreateDynamic(const physx::PxTransform& t, const physx::PxGeometry& geometry, const physx::PxVec3& velocity = physx::PxVec3(0));

		public:
physx::		PxFoundation&				GetPxFoundation()			{		return *m_Foundation;		}
physx::		PxPhysics&					GetPxPhysics()				{		return *m_Physics;			}
physx::		PxDefaultCpuDispatcher&		GetPxDefaultCpuDispatcher() {		return *m_Dispatcher;		}
physx::		PxScene&					GetPxScene()				{		return *m_PhysXScene;		}
physx::		PxDefaultAllocator&			GetPxDefaultAllocator()		{		return m_Allocator;			}
physx::		PxDefaultErrorCallback&		GetPxDefaultErrorCallback()	{		return m_ErrorCallback;		}
physx::		PxPvd&						GetPxPvd()					{		return *m_Pvd;				}
		protected:
physx::		PxFoundation*				m_Foundation = NULL;
physx::		PxPhysics*					m_Physics	 = NULL;
physx::		PxDefaultCpuDispatcher*		m_Dispatcher = NULL;
physx::		PxScene*					m_PhysXScene = NULL;
physx::		PxDefaultAllocator			m_Allocator;
physx::		PxDefaultErrorCallback		m_ErrorCallback;
physx::		PxMaterial*					m_Material = NULL;
physx::		PxPvd*						m_Pvd		= NULL;
		};
	}
}