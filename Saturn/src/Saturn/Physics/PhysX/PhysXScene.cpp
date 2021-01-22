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

namespace Saturn {

	PhysXScene::PhysXScene( Scene* scene ) : m_Scene( scene )
	{
		if( !m_Foundation )
		{
			m_Foundation = PxCreateFoundation( PX_PHYSICS_VERSION, m_DefaultAllocatorCallback, m_DefaultErrorCallback );
		}

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

		if ( !m_PVD )
		{
		}
		physx::PxSceneDesc sceneDesc( m_Physics->getTolerancesScale() );
		sceneDesc.gravity = physx::PxVec3( 0.0f, -9.81f, 0.0f );
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
		m_Dispatcher = physx::PxDefaultCpuDispatcherCreate( 2 );
		sceneDesc.cpuDispatcher	= m_Dispatcher;
		sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;
		m_PhysXScene = m_Physics->createScene( sceneDesc );

		if( !m_PhysXSimulationEventCallback )
		{
			m_PhysXSimulationEventCallback = new PhysXSimulationEventCallback( sceneDesc );
		}
		sceneDesc.simulationEventCallback = m_PhysXSimulationEventCallback;

		physx::PxPvdSceneClient* pvdClient = m_PhysXScene->getScenePvdClient();
		if( pvdClient )
		{
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
			pvdClient->setScenePvdFlag( physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );
		}

		m_PhysXScene->setVisualizationParameter( physx::PxVisualizationParameter::eSCALE, 1.0f );
		m_PhysXScene->setVisualizationParameter( physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f );
		//m_PhysXScene->setVisualizationParameter( physx::PxVisualizationParameter::eWORLD_AXES, 2.0f );
		//m_PhysXScene->setVisualizationParameter( physx::PxVisualizationParameter::eCOLLISION_AABBS, 3.0f );
		//m_PhysXScene->setVisualizationParameter( physx::PxVisualizationParameter::eCULL_BOX, 4.0f );


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
		m_PhysXScene->simulate( ts, nullptr, MemoryBlock, sizeof(MemoryBlock) );
		m_Scene->PhysicsUpdate( PhysicsType::PhysX, ts );
		m_PhysXScene->fetchResults( true );
	}

	void PhysXScene::RenderPhysXDebug( const SceneCamera& camera )
	{
		glm::vec4 color = glm::vec4{ 0.2f, 0.9f, 0.2f, 1.0f };
		const physx::PxRenderBuffer& rb = m_PhysXScene->getRenderBuffer();
		for( physx::PxU32 i=0; i < rb.getNbLines(); i++ )
		{
			//SAT_CORE_INFO( "getNbLines" );

			const physx::PxDebugLine& line = rb.getLines()[ i ];
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene(viewProj, false);

			Renderer2D::DrawLine(
				{ static_cast< float >( 10 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				{ static_cast< float >( 20 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				color
			);

			Renderer2D::EndScene();
		}
		for( physx::PxU32 i=0; i < rb.getNbPoints(); ++i )
		{

			//SAT_CORE_INFO( "getNbPoints" );

			const physx::PxDebugPoint& point = rb.getPoints()[ i ];
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene( viewProj, false );

			Renderer2D::DrawLine(
				{ static_cast< float >( 10 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				{ static_cast< float >( 20 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				color
			);

			Renderer2D::EndScene();
		}
		for( physx::PxU32 i=0; i < rb.getNbTriangles(); ++i )
		{

			//SAT_CORE_INFO( "getNbTriangles" );

			const physx::PxDebugTriangle& point = rb.getTriangles()[ i ];
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene( viewProj, false );

			Renderer2D::DrawLine(
				{ static_cast< float >( 10 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				{ static_cast< float >( 20 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				color
			);

			Renderer2D::EndScene();
		}
	}

	void PhysXScene::RenderPhysXDebug( const EditorCamera& camera )
	{
		glm::vec4 color = glm::vec4{ 0.2f, 0.9f, 0.2f, 1.0f };
		const physx::PxRenderBuffer& rb = m_PhysXScene->getRenderBuffer();
		for( physx::PxU32 i=0; i < rb.getNbLines(); i++ )
		{
			//SAT_CORE_INFO( "getNbLines" );

			glm::vec3 p0 ={ static_cast< float >( 10 ), static_cast< float >( 10 ), static_cast< float >( 40 ) };
			glm::vec3 p1 ={ static_cast< float >( 20 ), static_cast< float >( 10 ), static_cast< float >( 40 ) };

			const physx::PxDebugLine& line = rb.getLines()[ i ];
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene( viewProj, false );
			Renderer2D::EndScene();
		}
		for( physx::PxU32 i=0; i < rb.getNbPoints(); ++i )
		{

			//SAT_CORE_INFO( "getNbPoints" );

			const physx::PxDebugPoint& point = rb.getPoints()[ i ];
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene( viewProj, false );

			Renderer2D::DrawLine(
				{ static_cast< float >( 10 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				{ static_cast< float >( 20 ), static_cast< float >( 10 ), static_cast< float >( 40 ) },
				color
			);

			Renderer2D::EndScene();
		}
		for( physx::PxU32 i=0; i < rb.getNbTriangles(); ++i )
		{

			//SAT_CORE_INFO( "getNbTriangles" );

		glm::vec3 p0 ={ static_cast< float >( 10 ), static_cast< float >( 10 ), static_cast< float >( 40 ) };
		glm::vec3 p1 ={ static_cast< float >( 20 ), static_cast< float >( 10 ), static_cast< float >( 40 ) };

			const physx::PxDebugTriangle& point = rb.getTriangles()[ i ];
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene( viewProj, false );

			Renderer2D::DrawLine(
				p0,
				p1,
				color
			);

			Renderer2D::EndScene();
		}
	}

	PhysXScene::~PhysXScene()
	{

	}

}