/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "PhysXHelpers.h"

#include <physx/PxPhysicsAPI.h>

#include "Saturn/Script/ScriptEngine.h"

namespace Saturn {

	static PhysXContact s_PhysXSimulationEventCallback;
	static physx::PxDefaultCpuDispatcher* s_Dispatcher;
	static PhysXErrorCallback s_DefaultErrorCallback;
	static physx::PxDefaultAllocator s_DefaultAllocatorCallback;
	static physx::PxFoundation* s_Foundation;
	static physx::PxCooking* s_Cooking;
	static physx::PxPhysics* s_Physics;
	static physx::PxScene* s_PhysXScene;
	static physx::PxPvd* s_PVD;

	void PhysXFnd::Init()
	{
		physx::PxTolerancesScale FndToleranceScale;

		s_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, s_DefaultAllocatorCallback, s_DefaultErrorCallback );
		s_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *s_Foundation, FndToleranceScale, true );

		s_PVD = PxCreatePvd( *s_Foundation );
		if( s_PVD )
		{
			physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate( "localhost", 0001, 10 );
			s_PVD->connect( *transport, physx::PxPvdInstrumentationFlag::eALL );
		}

		physx::PxTolerancesScale ToleranceScale;
		ToleranceScale.length = 10;

		s_Cooking = PxCreateCooking( PX_PHYSICS_VERSION, *s_Foundation, s_Physics->getTolerancesScale() );
		s_Dispatcher = physx::PxDefaultCpuDispatcherCreate( 1 );

		physx::PxCookingParams cookingParameters = s_Cooking->getParams();
		cookingParameters.meshPreprocessParams.set( physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH );
		s_Cooking->setParams( cookingParameters );

	}

	physx::PxScene* PhysXFnd::CreateScene()
	{
		physx::PxSceneDesc sceneDesc( s_Physics->getTolerancesScale() );
		sceneDesc.gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );
		sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eABP;
		sceneDesc.cpuDispatcher = s_Dispatcher;
		//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		sceneDesc.filterShader = CollisionFilterShader;
		sceneDesc.simulationEventCallback = &s_PhysXSimulationEventCallback;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
		sceneDesc.frictionType = physx::PxFrictionType::ePATCH;

		SAT_ASSERT( sceneDesc.isValid(), "Scene is not valid" );
		return s_Physics->createScene( sceneDesc );
	}

	void PhysXFnd::CreateBoxCollider( Entity& entity, physx::PxRigidActor& actor )
	{
		auto& comp = entity.GetComponent<PhysXBoxColliderComponent>();
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();
		glm::vec3 size = comp.Extents;
		glm::vec3 entitySize = trans.Scale;


		physx::PxBoxGeometry boxGeo = physx::PxBoxGeometry( size.x / 2.0f, size.y / 2.0f, size.z / 2.0f );
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape( actor, boxGeo, *s_Physics->createMaterial( 1, 1, 1 ) );
		shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
		shape->setLocalPose( glmTransformToPx( glm::translate( glm::mat4( 1.0f ), comp.Offset ) ) );
	}

	void PhysXFnd::CreateSphereCollider( Entity& entity, physx::PxRigidActor& actor )
	{
		auto& comp = entity.GetComponent<PhysXSphereColliderComponent>();
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();
		float size = comp.Radius;
		glm::vec3 entitySize = trans.Scale;

		if( entitySize.x != 0.0f )
			size *= entitySize.x;

		physx::PxSphereGeometry sphereGeo = physx::PxSphereGeometry( size / 2.0f );
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape( actor, sphereGeo, *s_Physics->createMaterial( 1, 1, 1 ) );
		shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
	}

	void PhysXFnd::CreateCapsuleCollider( Entity& entity, physx::PxRigidActor& actor )
	{
		auto& comp = entity.GetComponent<PhysXCapsuleColliderComponent>();
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();
		float size = comp.Radius;
		float height = comp.Height;

		glm::vec3 entitySize = trans.Scale;

		if( entitySize.x != 0.0f )
			size *= (entitySize.x);

		if( entitySize.y != 0.0f )
			height *= ( entitySize.y );

		physx::PxCapsuleGeometry capsuleGeo = physx::PxCapsuleGeometry( size, height / 2.0f );
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape( actor, capsuleGeo, *s_Physics->createMaterial( 1, 1, 1 ) );
		shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
		shape->setLocalPose( physx::PxTransform( physx::PxQuat( physx::PxHalfPi, physx::PxVec3( 0, 0, 1 ) ) ) );
	}

	void PhysXFnd::AddRigidBody( Entity entity )
	{
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();
		rb.m_Rigidbody = new PhysXRigidbody( entity, trans.Position, trans.Rotation );
	}

	physx::PxPhysics& PhysXFnd::GetPhysics()
	{
		return *s_Physics;
	}

	physx::PxScene& PhysXFnd::GetPhysXScene()
	{
		return *s_PhysXScene;
	}

	physx::PxAllocatorCallback& PhysXFnd::GetAllocator()
	{
		return s_DefaultAllocatorCallback;
	}

	PhysXContact& PhysXFnd::GetPhysXContact()
	{
		return s_PhysXSimulationEventCallback;
	}

	void PhysXContact::onConstraintBreak( physx::PxConstraintInfo* constraints, physx::PxU32 count )
	{
		PX_UNUSED( constraints );
		PX_UNUSED( count );
	}

	void PhysXContact::onWake( physx::PxActor** actors, physx::PxU32 count )
	{

	}

	void PhysXContact::onSleep( physx::PxActor** actors, physx::PxU32 count )
	{

	}

	void PhysXContact::onContact( const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs )
	{
		Entity& a = *( Entity* )pairHeader.actors[ 0 ]->userData;
		Entity& b = *( Entity* )pairHeader.actors[ 1 ]->userData;

		if( pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH )
		{
			if( ScriptEngine::IsEntityModuleValid( a ) )
				ScriptEngine::OnCollisionBegin( a );
			if( ScriptEngine::IsEntityModuleValid( b ) )
				ScriptEngine::OnCollisionBegin( b );
		}
		else if( pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH )
		{
			if( ScriptEngine::IsEntityModuleValid( a ) )
				ScriptEngine::OnCollisionExit( a );
			if( ScriptEngine::IsEntityModuleValid( b ) )
				ScriptEngine::OnCollisionExit( b );
		}
	}

	void PhysXContact::onTrigger( physx::PxTriggerPair* pairs, physx::PxU32 count )
	{

	}

	void PhysXContact::onAdvance( const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count )
	{
		PX_UNUSED( bodyBuffer );
		PX_UNUSED( poseBuffer );
		PX_UNUSED( count );
	}

	void PhysXErrorCallback::reportError( physx::PxErrorCode::Enum code, const char* message, const char* file, int line )
	{
		switch( code )
		{
			case physx::PxErrorCode::eNO_ERROR:
				SAT_CORE_INFO( "PhysX: {0} {1} {2}", message, file, line );
				break;
			case physx::PxErrorCode::eDEBUG_INFO:
				SAT_CORE_INFO( "PhysX: {0} {1} {2}", message, file, line );
				break;
			case physx::PxErrorCode::eDEBUG_WARNING:
				SAT_CORE_WARN( "PhysX: {0} {1} {2}", message, file, line );
				break;
			case physx::PxErrorCode::eINVALID_PARAMETER:
				SAT_CORE_WARN( "PhysX: {0}, {1}, {2}", message, file, line );
				break;
			case physx::PxErrorCode::eINVALID_OPERATION:
				SAT_CORE_WARN( "PhysX: {0}, {1}, {2}", message, file, line );
				break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:
				SAT_CORE_FATAL( "PhysX: Out of memory!" );
				break;
			case physx::PxErrorCode::eINTERNAL_ERROR:
				SAT_CORE_ERROR( "PhysX: Internal Error!" );
				break;
			case physx::PxErrorCode::eABORT:
				SAT_CORE_ERROR( "PhysX: Abort!" );
				break;
			case physx::PxErrorCode::ePERF_WARNING:
				SAT_CORE_WARN( "PhysX: {0}, {1}, {2}", message, file, line );
				break;
			case physx::PxErrorCode::eMASK_ALL:
				break;
			default:
				SAT_CORE_WARN( "PhysX: {0}, {1}, {2}", message, file, line );
				break;
		}
	}
}
