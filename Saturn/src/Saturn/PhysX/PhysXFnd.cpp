/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "PhysXFnd.h"

#include "PhysXRigidBody.h"

namespace Saturn {

	void PhysXContact::onConstraintBreak( physx::PxConstraintInfo* pConstraints, physx::PxU32 Count )
	{
	}

	void PhysXContact::onWake( physx::PxActor** ppActors, physx::PxU32 Count )
	{
	}

	void PhysXContact::onSleep( physx::PxActor** ppActors, physx::PxU32 Count )
	{
	}

	void PhysXContact::onTrigger( physx::PxTriggerPair* pPairs, physx::PxU32 Count )
	{
	}

	void PhysXContact::onAdvance( const physx::PxRigidBody* const* pBodyBuffer, const physx::PxTransform* PoseBuffer, const physx::PxU32 Count )
	{
	}

	void PhysXContact::onContact( const physx::PxContactPairHeader& rPairHeader, const physx::PxContactPair* pPairs, physx::PxU32 Pairs )
	{
		SAT_CORE_INFO( "onContact" );
	}

	//////////////////////////////////////////////////////////////////////////
	
	void PhysXAssertCallback::operator()( const char* pError, const char* pFile, int Line, bool& rIngore )
	{
		SAT_CORE_ERROR( "PhsyX error : {0}, File : {1}, Line : {2}", pError, pFile, Line );
		SAT_CORE_ASSERT( false );
	}

	//////////////////////////////////////////////////////////////////////////

	void PhysXErrorCallback::reportError( physx::PxErrorCode::Enum ErrorCode, const char* pErrorString, const char* pFile, int Line )
	{
		switch( ErrorCode )
		{
			case physx::PxErrorCode::eNO_ERROR:
				SAT_CORE_INFO( "PhysX: {0} {1} {2}", pErrorString, pFile, Line );
				break;
			case physx::PxErrorCode::eDEBUG_INFO:
				SAT_CORE_INFO( "PhysX: {0} {1} {2}", pErrorString, pFile, Line );
				break;
			case physx::PxErrorCode::eDEBUG_WARNING:
				SAT_CORE_WARN( "PhysX: {0} {1} {2}", pErrorString, pFile, Line );
				break;
			case physx::PxErrorCode::eINVALID_PARAMETER:
				SAT_CORE_WARN( "PhysX: Invaild paramater!" );
				break;
			case physx::PxErrorCode::eINVALID_OPERATION:
				SAT_CORE_WARN( "PhysX: Invaild operation!" );
				break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:
				SAT_CORE_ASSERT( false, "PhysX: Out of memory");
			case physx::PxErrorCode::eINTERNAL_ERROR:
				SAT_CORE_ERROR( "PhysX: Interal Error! ");
				break;
			case physx::PxErrorCode::eABORT:
				SAT_CORE_ASSERT( false, "PhysX request an abort!" );
				
			case physx::PxErrorCode::ePERF_WARNING:
				break;
			case physx::PxErrorCode::eMASK_ALL:
				break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	
	static physx::PxFoundation* s_Foundation;
	static physx::PxPhysics* s_Physics;
	static physx::PxCooking* s_Cooking;
	static physx::PxPvd* s_PVD;
	static physx::PxScene* s_PhysXScene;
	
	static physx::PxDefaultCpuDispatcher* s_Dispatcher;
	static physx::PxDefaultAllocator s_DefaultAllocatorCallback;
	static PhysXErrorCallback s_DefaultErrorCallback;
	static PhysXAssertCallback s_AssertHandler;
	static PhysXContact s_ContactCallback;

	void PhysXFnd::Init()
	{
		physx::PxTolerancesScale Scale;
		Scale.length = 10.0f;

		s_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, s_DefaultAllocatorCallback, s_DefaultErrorCallback );
		s_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *s_Foundation, Scale, true );

		s_PVD = PxCreatePvd( *s_Foundation );

		s_Cooking = PxCreateCooking( PX_PHYSICS_VERSION, *s_Foundation, Scale );
		s_Dispatcher = physx::PxDefaultCpuDispatcherCreate( std::thread::hardware_concurrency() / 2 );
		physx::PxSetAssertHandler( s_AssertHandler );
	}

#define TERMINATE_ITEM( x ) if(x) x->release(); x = nullptr

	void PhysXFnd::Terminate()
	{
		TERMINATE_ITEM( s_Dispatcher );
		TERMINATE_ITEM( s_Cooking );
		TERMINATE_ITEM( s_Physics );
		TERMINATE_ITEM( s_PhysXScene );
		TERMINATE_ITEM( s_PVD );
		TERMINATE_ITEM( s_Foundation );
	}
	
	physx::PxFilterFlags CollisionFilterShader( 
		physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, 
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize ) 
	{
		if( physx::PxFilterObjectIsTrigger( attributes0 ) || physx::PxFilterObjectIsTrigger( attributes1 ) )
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
		pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;

		if( ( filterData0.word0 & filterData1.word1 ) || ( filterData1.word0 & filterData0.word1 ) ) 
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_CCD;
			
			return physx::PxFilterFlag::eDEFAULT;
		}

		return physx::PxFilterFlag::eDEFAULT;
	}

	physx::PxScene* PhysXFnd::CreateScene()
	{
		physx::PxSceneDesc SceneDesc( s_Physics->getTolerancesScale() );
		SceneDesc.gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );
		SceneDesc.filterShader = CollisionFilterShader;
		SceneDesc.simulationEventCallback = new PhysXContact();
		SceneDesc.cpuDispatcher = s_Dispatcher;
		SceneDesc.flags = physx::PxSceneFlag::eENABLE_CCD;
		SceneDesc.simulationEventCallback = &s_ContactCallback;
		SceneDesc.broadPhaseType = physx::PxBroadPhaseType::eABP;
		SceneDesc.frictionType = physx::PxFrictionType::ePATCH;
		
		return s_Physics->createScene( SceneDesc );
	}

	void PhysXFnd::CreateBoxCollider( Entity& rEntity, physx::PxRigidActor& rActor )
	{
		auto& comp = rEntity.GetComponent<PhysXBoxColliderComponent>();
		auto& rb = rEntity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = rEntity.GetComponent<TransformComponent>();
		auto& mat = rEntity.GetComponent<PhysXMaterialComponent>();
		auto& mesh = rEntity.GetComponent<MeshComponent>();

		glm::vec3 size = comp.Extents;
		glm::vec3 scale = trans.Scale;
		
		physx::PxBoxGeometry BoxGeometry = physx::PxBoxGeometry( size.x / 2.0f, size.y / 2.0f, size.z / 2.0f );
		physx::PxShape* pShape = physx::PxRigidActorExt::createExclusiveShape( rActor, BoxGeometry, *s_Physics->createMaterial( mat.StaticFriction, mat.DynamicFriction, mat.Restitution ) );
		
		pShape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		pShape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
		pShape->setLocalPose( glmTransformToPx( glm::translate( glm::mat4( 1.0f ), comp.Offset ) ) );
	}

	void PhysXFnd::CreateSphereCollider( Entity& rEntity, physx::PxRigidActor& rActor )
	{
		auto& comp = rEntity.GetComponent<PhysXSphereColliderComponent>();
		auto& rb = rEntity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = rEntity.GetComponent<TransformComponent>();
		auto& mat = rEntity.GetComponent<PhysXMaterialComponent>();
		auto& mesh = rEntity.GetComponent<MeshComponent>();
		
		float size = comp.Radius;
		glm::vec3 entitySize = trans.Scale;

		if( entitySize.x != 0.0f )
			size *= entitySize.x;
		
		physx::PxSphereGeometry SphereGeometry( size / 2.0f );
		
		physx::PxShape* pShape = physx::PxRigidActorExt::createExclusiveShape( rActor, SphereGeometry, *s_Physics->createMaterial( mat.StaticFriction, mat.DynamicFriction, mat.Restitution ) );
		pShape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		pShape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
	}

	void PhysXFnd::CreateCapsuleCollider( Entity& rEntity, physx::PxRigidActor& rActor )
	{
		auto& comp = rEntity.GetComponent<PhysXCapsuleColliderComponent>();
		auto& rb = rEntity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = rEntity.GetComponent<TransformComponent>();
		auto& mat = rEntity.GetComponent<PhysXMaterialComponent>();
		auto& mesh = rEntity.GetComponent<MeshComponent>();

		float size = comp.Radius;
		float height = comp.Height;

		glm::vec3 entitySize = trans.Scale;

		if( entitySize.x != 0.0f )
			size *= ( entitySize.x );

		if( entitySize.y != 0.0f )
			height *= ( entitySize.y );
		
		physx::PxCapsuleGeometry CapsuleGeometry( size, height / 2.0f );

		physx::PxShape* pShape = physx::PxRigidActorExt::createExclusiveShape( rActor, CapsuleGeometry, *s_Physics->createMaterial( mat.StaticFriction, mat.DynamicFriction, mat.Restitution ) );
		pShape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		pShape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
		pShape->setLocalPose( physx::PxTransform( physx::PxQuat( physx::PxHalfPi, physx::PxVec3( 0, 0, 1 ) ) ) );
	}

	void PhysXFnd::CreateMeshCollider( Entity& rEntity, physx::PxRigidActor& rActor )
	{

	}

	void PhysXFnd::AddRigidBody( Entity& entity )
	{
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();

		rb.m_Rigidbody = new PhysXRigidbody( entity, trans.Position, trans.Rotation );
		rb.m_Rigidbody->Create();
	}

	bool PhysXFnd::Raycast( glm::vec3& Origin, glm::vec3& Direction, float Distance, RaycastResult* pResult )
	{
		return true;
	}

	physx::PxPhysics* PhysXFnd::GetPhysics()
	{
		return s_Physics;
	}

	physx::PxAllocatorCallback& PhysXFnd::GetAllocator()
	{
		return s_DefaultAllocatorCallback;
	}

}