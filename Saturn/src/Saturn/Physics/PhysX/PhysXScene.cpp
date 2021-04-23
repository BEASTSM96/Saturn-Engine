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
#include "PhysXScene.h"
#include "Saturn/Scene/Scene.h"
#include "Saturn/Renderer/Renderer.h"
#include "Saturn/Renderer/Renderer2D.h"
#include "PhysXSimulationEventCallback.h"
#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Components.h"
#include "Saturn/Renderer/SceneRenderer.h"

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


	PhysXScene::PhysXScene( Scene* scene ) : m_Scene( scene )
	{

	}

	PhysXScene::~PhysXScene()
	{

	}

	void PhysXScene::Update( Timestep ts )
	{
		s_PhysXScene->simulate( ts, nullptr, MemoryBlock, sizeof( MemoryBlock ) );
		m_Scene->PhysicsUpdate( PhysicsType::PhysX, ts );
		s_PhysXScene->fetchResults( true );
	}

	physx::PxAllocatorCallback& PhysXScene::GetAllocator()
	{
		return s_DefaultAllocatorCallback;
	}

	const physx::PxAllocatorCallback& PhysXScene::GetAllocator() const
	{
		return s_DefaultAllocatorCallback;
	}

	physx::PxPhysics& PhysXScene::GetPhysics()
	{
		return *s_Physics;
	}

	const physx::PxPhysics& PhysXScene::GetPhysics() const
	{
		return *s_Physics;
	}

	physx::PxScene& PhysXScene::GetPhysXScene()
	{
		return *s_PhysXScene;
	}

	const physx::PxScene& PhysXScene::GetPhysXScene() const
	{
		return *s_PhysXScene;
	}

	void PhysXScene::Init()
	{
		physx::PxTolerancesScale ToleranceScale;
		ToleranceScale.length = 10;

		s_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, s_DefaultAllocatorCallback, s_DefaultErrorCallback );
		s_Physics = PxCreatePhysics( PX_PHYSICS_VERSION, *s_Foundation, ToleranceScale, true );

		s_PVD = PxCreatePvd( *s_Foundation );
		if( s_PVD )
		{
			physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate( "localhost", 0001, 10 );
			s_PVD->connect( *transport, physx::PxPvdInstrumentationFlag::eALL );
		}

		s_Cooking = PxCreateCooking( PX_PHYSICS_VERSION, *s_Foundation, s_Physics->getTolerancesScale() );
		s_Dispatcher = physx::PxDefaultCpuDispatcherCreate( 1 );

		physx::PxCookingParams cookingParameters = s_Cooking->getParams();
		cookingParameters.meshPreprocessParams.set( physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH );
		s_Cooking->setParams( cookingParameters );

		physx::PxSceneDesc sceneDesc( s_Physics->getTolerancesScale() );
		sceneDesc.gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );
		sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eABP;
		sceneDesc.cpuDispatcher = s_Dispatcher;
		//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		sceneDesc.filterShader = CollisionFilterShader;
		sceneDesc.simulationEventCallback = &s_PhysXSimulationEventCallback;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
		sceneDesc.frictionType = physx::PxFrictionType::ePATCH;

		s_PhysXScene = s_Physics->createScene( sceneDesc );
	}

	void PhysXContact::onConstraintBreak( physx::PxConstraintInfo* constraints, physx::PxU32 count )
	{

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

		SAT_CORE_INFO( "onContact" );

		if( pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH )
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

	void PhysXContact::onTrigger( physx::PxTriggerPair* pairs, physx::PxU32 count )
	{

	}

	void PhysXContact::onAdvance( const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count )
	{

	}

}