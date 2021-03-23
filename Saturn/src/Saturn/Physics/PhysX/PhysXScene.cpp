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
		/*
		physx::PxActorTypeFlags types = physx::PxActorTypeFlag::eRIGID_STATIC | physx::PxActorTypeFlag::eRIGID_DYNAMIC;
		physx::PxU32 count = m_PhysXScene->getNbActors( types );
		physx::PxRigidActor** Buffer = new physx::PxRigidActor * [ count ];
		m_PhysXScene->getActors( types, (physx::PxActor**)Buffer, count );
		for ( physx::PxU32 i = 0; i < count; i++ )
		{
			physx::PxU32 shapes = Buffer[ i ]->getNbShapes();
			physx::PxShape** Shape = new physx::PxShape * [ shapes ];
			Buffer[ i ]->getShapes( Shape, count );
			for( physx::PxU32 i = 0; i < shapes; i++ )
			{
				auto shapetype = Shape[ i ]->getGeometryType();
				physx::PxTransform pose = Shape[ i ]->getLocalPose();

				auto& camera = *m_Scene->GetMainCameraEntity().GetComponent<CameraComponent>().Camera.Raw();
				glm::vec4 color = glm::vec4{ 0.2f, 0.9f, 0.2f, 1.0f };
				auto viewProj = camera.GetViewProjection();

				Ref<MaterialInstance> material;
				auto shader = Shader::Create( "assets/shaders/Grid.glsl" );
				material = MaterialInstance::Create( Material::Create( shader ) );
				float gridScale = 28.025f, gridSize = 1.025f;
				material->Set( "u_Scale", gridScale );
				material->Set( "u_Res", gridSize );
				Renderer::SubmitQuad( material, glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) * glm::scale( glm::mat4( 1.0f ), glm::vec3( 16.0f ) ) );
			}
		}
		*/

	}

	void PhysXScene::RenderPhysXDebug( const SceneCamera& camera )
	{
		glm::vec4 color = glm::vec4{ 0.2f, 0.9f, 0.2f, 1.0f };
		const physx::PxRenderBuffer& rb = m_PhysXScene->getRenderBuffer();
		for( physx::PxU32 i=0; i < rb.getNbLines(); i++ )
		{
			SAT_CORE_INFO( "getNbLines" );

			const physx::PxDebugLine& line = rb.getLines()[ i ];
			glm::vec3 p0 ={ static_cast< float >( line.pos0.x ), static_cast< float >( line.pos0.y ), static_cast< float >( line.pos0.z ) };
			glm::vec3 p1 ={ static_cast< float >( line.pos1.x ), static_cast< float >( line.pos1.y ), static_cast< float >( line.pos1.z ) };

			Renderer::BeginRenderPass( SceneRenderer::GetFinalRenderPass(), false );
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene( viewProj, false );
			glm::vec4 color;
			Renderer2D::DrawLine( p0, glm::vec3( 10, 10, 10 ), color );
			Renderer2D::DrawLine( p1, glm::vec3( 10, 10, 10 ), color );
			Renderer2D::EndScene();
			Renderer::EndRenderPass();


		}
		for( physx::PxU32 i=0; i < rb.getNbPoints(); ++i )
		{

			SAT_CORE_INFO( "getNbPoints" );

			const physx::PxDebugPoint& point = rb.getPoints()[ i ];
			auto viewProj = camera.GetViewProjection();

			glm::vec3 p0 ={ static_cast< float >( point.pos.x ), static_cast< float >( point.pos.y ), static_cast< float >( point.pos.z ) };

			Renderer::BeginRenderPass( SceneRenderer::GetFinalRenderPass(), false );
			Renderer2D::BeginScene( viewProj, false );
			glm::vec4 color ={ static_cast< float >( point.color ), static_cast< float >( point.color ), static_cast< float >( point.color ), static_cast< float >( point.color ) };
			Renderer2D::DrawLine( p0, color );
			Renderer2D::EndScene();
			Renderer::EndRenderPass();
		}
		for( physx::PxU32 i=0; i < rb.getNbTriangles(); ++i )
		{

			SAT_CORE_INFO( "getNbTriangles" );


			const physx::PxDebugTriangle& triangle = rb.getTriangles()[ i ];
			auto viewProj = camera.GetViewProjection();

			glm::vec3 p0 ={ static_cast< float >( triangle.pos0.x ), static_cast< float >( triangle.pos0.y ), static_cast< float >( triangle.pos0.z ) };
			glm::vec3 p1 ={ static_cast< float >( triangle.pos1.x ), static_cast< float >( triangle.pos1.y ), static_cast< float >( triangle.pos2.z ) };
			glm::vec3 p2 ={ static_cast< float >( triangle.pos2.x ), static_cast< float >( triangle.pos2.y ), static_cast< float >( triangle.pos2.z ) };

			Renderer::BeginRenderPass( SceneRenderer::GetFinalRenderPass(), false );
			Renderer2D::BeginScene( viewProj, false );
			Renderer2D::DrawLine(
				p0,
				p1,
				color
			);
			Renderer2D::EndScene();
			Renderer::EndRenderPass();
		}
	}

	void PhysXScene::RenderPhysXDebug( const EditorCamera& camera )
	{
		glm::vec4 color = glm::vec4{ 0.2f, 0.9f, 0.2f, 1.0f };
		const physx::PxRenderBuffer& rb = m_PhysXScene->getRenderBuffer();
		for( physx::PxU32 i=0; i < rb.getNbLines(); i++ )
		{
			SAT_CORE_INFO( "getNbLines" );

			const physx::PxDebugLine& line = rb.getLines()[ i ];
			glm::vec3 p0 ={ static_cast< float >( line.pos0.x ), static_cast< float >( line.pos0.y ), static_cast< float >( line.pos0.z ) };
			glm::vec3 p1 ={ static_cast< float >( line.pos1.x ), static_cast< float >( line.pos1.y ), static_cast< float >( line.pos1.z ) };

			Renderer::BeginRenderPass( SceneRenderer::GetFinalRenderPass(), false );
			auto viewProj = camera.GetViewProjection();
			Renderer2D::BeginScene( viewProj, false );
			glm::vec4 color;
			Renderer2D::DrawLine( p0, glm::vec3(10, 10, 10), color );
			Renderer2D::DrawLine( p1, glm::vec3( 10, 10, 10 ), color );
			Renderer2D::EndScene();
			Renderer::EndRenderPass();


		}
		for( physx::PxU32 i=0; i < rb.getNbPoints(); ++i )
		{

			SAT_CORE_INFO( "getNbPoints" );

			const physx::PxDebugPoint& point = rb.getPoints()[ i ];
			auto viewProj = camera.GetViewProjection();

			glm::vec3 p0 ={ static_cast< float >( point.pos.x ), static_cast< float >( point.pos.y ), static_cast< float >( point.pos.z ) };

			Renderer::BeginRenderPass( SceneRenderer::GetFinalRenderPass(), false );
			Renderer2D::BeginScene( viewProj, false );
			glm::vec4 color ={ static_cast< float >( point.color ), static_cast< float >( point.color ), static_cast< float >( point.color ), static_cast< float >( point.color ) };
			Renderer2D::DrawLine( p0, color );
			Renderer2D::EndScene();
			Renderer::EndRenderPass();
		}
		for( physx::PxU32 i=0; i < rb.getNbTriangles(); ++i )
		{

			SAT_CORE_INFO( "getNbTriangles" );


			const physx::PxDebugTriangle& triangle = rb.getTriangles()[ i ];
			auto viewProj = camera.GetViewProjection();

			glm::vec3 p0 ={ static_cast< float >( triangle.pos0.x ), static_cast< float >( triangle.pos0.y ), static_cast< float >( triangle.pos0.z ) };
			glm::vec3 p1 ={ static_cast< float >( triangle.pos1.x ), static_cast< float >( triangle.pos1.y ), static_cast< float >( triangle.pos2.z ) };
			glm::vec3 p2 ={ static_cast< float >( triangle.pos2.x ), static_cast< float >( triangle.pos2.y ), static_cast< float >( triangle.pos2.z ) };

			Renderer::BeginRenderPass( SceneRenderer::GetFinalRenderPass(), false );
			Renderer2D::BeginScene( viewProj, false );
			Renderer2D::DrawLine(
				p0,
				p1,
				color
			);
			Renderer2D::EndScene();
			Renderer::EndRenderPass();
		}
	}

	PhysXScene::~PhysXScene()
	{

	}

}