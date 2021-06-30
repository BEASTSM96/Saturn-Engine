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

#include "Saturn/Core/Math.h"

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
	static PhysXAssertCallback s_AssertHandler;

	void PhysXFnd::Init()
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
		PxSetAssertHandler( s_AssertHandler );

	}

	void PhysXFnd::Clear()
	{
		if( s_Dispatcher )
			s_Dispatcher->release();
		s_Dispatcher = nullptr;

		if( s_Cooking )
			s_Cooking->release();
		s_Cooking = nullptr;

		if( s_Physics )
			s_Physics->release();
		s_Physics = nullptr;

		if( s_PhysXScene )
			s_PhysXScene->release();
		s_PhysXScene = nullptr;

		if( s_PVD )
			s_PVD->release();
		s_PVD = nullptr;

		if( s_Foundation )
			s_Foundation->release();
		s_Foundation = nullptr;

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
		auto& mat = entity.GetComponent<PhysXMaterialComponent>();
		glm::vec3 size = comp.Extents;
		glm::vec3 entitySize = trans.Scale;


		physx::PxBoxGeometry boxGeo = physx::PxBoxGeometry( size.x / 2.0f, size.y / 2.0f, size.z / 2.0f );
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape( actor, boxGeo, *s_Physics->createMaterial( mat.StaticFriction, mat.DynamicFriction, mat.Restitution ) );
		shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
		shape->setLocalPose( glmTransformToPx( glm::translate( glm::mat4( 1.0f ), comp.Offset ) ) );
	}

	void PhysXFnd::CreateSphereCollider( Entity& entity, physx::PxRigidActor& actor )
	{
		auto& comp = entity.GetComponent<PhysXSphereColliderComponent>();
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();
		auto& mat = entity.GetComponent<PhysXMaterialComponent>();
		float size = comp.Radius;
		glm::vec3 entitySize = trans.Scale;

		if( entitySize.x != 0.0f )
			size *= entitySize.x;

		physx::PxSphereGeometry sphereGeo = physx::PxSphereGeometry( size / 2.0f );
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape( actor, sphereGeo, *s_Physics->createMaterial( mat.StaticFriction, mat.DynamicFriction, mat.Restitution ) );
		shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
	}

	void PhysXFnd::CreateCapsuleCollider( Entity& entity, physx::PxRigidActor& actor )
	{
		auto& comp = entity.GetComponent<PhysXCapsuleColliderComponent>();
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();
		auto& mat = entity.GetComponent<PhysXMaterialComponent>();
		float size = comp.Radius;
		float height = comp.Height;

		glm::vec3 entitySize = trans.Scale;

		if( entitySize.x != 0.0f )
			size *= ( entitySize.x );

		if( entitySize.y != 0.0f )
			height *= ( entitySize.y );

		physx::PxCapsuleGeometry capsuleGeo = physx::PxCapsuleGeometry( size, height / 2.0f );
		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape( actor, capsuleGeo, *s_Physics->createMaterial( mat.StaticFriction, mat.DynamicFriction, mat.Restitution ) );
		shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !comp.IsTrigger );
		shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, comp.IsTrigger );
		shape->setLocalPose( physx::PxTransform( physx::PxQuat( physx::PxHalfPi, physx::PxVec3( 0, 0, 1 ) ) ) );
	}

	void PhysXFnd::CreateMeshCollider( Entity& entity, physx::PxRigidActor& actor )
	{
		auto& collider = entity.GetComponent<PhysXMeshColliderComponent>();
		auto& material = entity.GetComponent<PhysXMaterialComponent>();
		glm::vec3 size = entity.GetComponent<TransformComponent>().Scale;

		if( collider.IsConvex )
		{
			std::vector<physx::PxShape*> shapes = CreateConvexMesh( collider, size );

			for( auto shape : shapes )
			{
				physx::PxMaterial* materials[] ={ s_Physics->createMaterial( material.StaticFriction, material.DynamicFriction, material.Restitution ) };
				shape->setMaterials( materials, 1 );
				shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger );
				shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger );
				actor.attachShape( *shape );
				shape->release();
			}
		}
		else
		{
			std::vector<physx::PxShape*> shapes = CreateTriangleMesh( collider, size );

			for( auto shape : shapes )
			{
				physx::PxMaterial* materials[] ={ s_Physics->createMaterial( material.StaticFriction, material.DynamicFriction, material.Restitution ) };
				shape->setMaterials( materials, 1 );
				shape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, !collider.IsTrigger );
				shape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, collider.IsTrigger );
				actor.attachShape( *shape );
				shape->release();
			}
		}
	}

	std::vector<physx::PxShape*> PhysXFnd::CreateConvexMesh( PhysXMeshColliderComponent& collider, const glm::vec3& size, bool invalidateOld /*= false */ )
	{
		std::vector<physx::PxShape*> shapes;

		collider.ProcessedMeshes.clear();

		const physx::PxCookingParams& currentParams = s_Cooking->getParams();
		physx::PxCookingParams newParams = currentParams;
		newParams.planeTolerance = 0.0F;
		newParams.meshPreprocessParams = physx::PxMeshPreprocessingFlags( physx::PxMeshPreprocessingFlag::eWELD_VERTICES );
		newParams.meshWeldTolerance = 0.01f;
		s_Cooking->setParams( newParams );

		const std::vector<Vertex>& vertices = collider.CollisionMesh->GetVertices();
		const std::vector<Index>& indices = collider.CollisionMesh->GetIndices();

		for( const auto& submesh : collider.CollisionMesh->GetSubmeshes() )
		{
			physx::PxConvexMeshDesc convexDesc;
			convexDesc.points.count = submesh.VertexCount;
			convexDesc.points.stride = sizeof( Vertex );
			convexDesc.points.data = &vertices[ submesh.BaseVertex ];
			convexDesc.indices.count = submesh.IndexCount / 3;
			convexDesc.indices.data = &indices[ submesh.BaseIndex / 3 ];
			convexDesc.indices.stride = sizeof( Index );
			convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eSHIFT_VERTICES;

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxConvexMeshCookingResult::Enum result;
			if( !s_Cooking->cookConvexMesh( convexDesc, buf, &result ) )
			{
				SAT_CORE_ERROR( "Failed to cook convex mesh {0}", submesh.MeshName );
				continue;
			}

			physx::PxDefaultMemoryInputData input( buf.getData(), buf.getSize() );
			physx::PxConvexMesh* convexMesh = s_Physics->createConvexMesh( input );
			physx::PxConvexMeshGeometry convexGeometry = physx::PxConvexMeshGeometry( convexMesh, physx::PxMeshScale( glmVec3ToPx( size ) ) );
			convexGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;
			physx::PxMaterial* material = s_Physics->createMaterial( 0, 0, 0 );
			physx::PxShape* shape = s_Physics->createShape( convexGeometry, *material, true );
			shape->setLocalPose( glmTransformToPx( submesh.Transform ) );
			shapes.push_back( shape );
		}

		if( collider.ProcessedMeshes.size() <= 0 )
		{
			for( auto shape : shapes )
			{
				physx::PxConvexMeshGeometry convexGeometry;
				shape->getConvexMeshGeometry( convexGeometry );
				physx::PxConvexMesh* mesh = convexGeometry.convexMesh;

				const uint32_t nbPolygons = mesh->getNbPolygons();
				const physx::PxVec3* convexVertices = mesh->getVertices();
				const physx::PxU8* convexIndices = mesh->getIndexBuffer();

				uint32_t nbVertices = 0;
				uint32_t nbFaces = 0;

				std::vector<Vertex> collisionVertices;
				std::vector<Index> collisionIndices;
				uint32_t vertCounter = 0;
				uint32_t indexCounter = 0;

				for( uint32_t i = 0; i < nbPolygons; i++ )
				{
					physx::PxHullPolygon polygon;
					mesh->getPolygonData( i, polygon );
					nbVertices += polygon.mNbVerts;
					nbFaces += ( polygon.mNbVerts - 2 ) * 3;

					uint32_t vI0 = vertCounter;

					for( uint32_t vI = 0; vI < polygon.mNbVerts; vI++ )
					{
						Vertex v;
						v.Position = PxVec3ToGLM( convexVertices[ convexIndices[ polygon.mIndexBase + vI ] ] );
						collisionVertices.push_back( v );
						vertCounter++;
					}

					for( uint32_t vI = 1; vI < uint32_t( polygon.mNbVerts ) - 1; vI++ )
					{
						Index index;
						index.V1 = uint32_t( vI0 );
						index.V2 = uint32_t( vI0 + vI + 1 );
						index.V3 = uint32_t( vI0 + vI );
						collisionIndices.push_back( index );
						indexCounter++;
					}

					collider.ProcessedMeshes.push_back( Ref<Mesh>::Create( collisionVertices, collisionIndices, PxTransformToGlm( shape->getLocalPose() ) ) );
				}
			}
		}

		s_Cooking->setParams( currentParams );
		return shapes;
	}

	std::vector<physx::PxShape*> PhysXFnd::CreateTriangleMesh( PhysXMeshColliderComponent& collider, const glm::vec3& scale /*= glm::vec3( 1.0f )*/, bool invalidateOld /*= false */ )
	{
		std::vector<physx::PxShape*> shapes;

		collider.ProcessedMeshes.clear();

		const std::vector<Vertex>& vertices = collider.CollisionMesh->GetVertices();
		const std::vector<Index>& indices = collider.CollisionMesh->GetIndices();

		for( const auto& submesh : collider.CollisionMesh->GetSubmeshes() )
		{
			physx::PxTriangleMeshDesc triangleDesc;
			triangleDesc.points.count = submesh.VertexCount;
			triangleDesc.points.stride = sizeof( Vertex );
			triangleDesc.points.data = &vertices[ submesh.BaseVertex ];
			triangleDesc.triangles.count = submesh.IndexCount / 3;
			triangleDesc.triangles.data = &indices[ submesh.BaseIndex / 3 ];
			triangleDesc.triangles.stride = sizeof( Index );

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxTriangleMeshCookingResult::Enum result;
			if( !s_Cooking->cookTriangleMesh( triangleDesc, buf, &result ) )
			{
				SAT_CORE_ERROR( "Failed to cook triangle mesh: {0}", submesh.MeshName );
				continue;
			}

			glm::vec3 submeshTranslation, submeshScale;
			glm::quat submeshRotation;
			DecomposeTransform( submesh.Transform, submeshTranslation, submeshRotation, submeshScale );

			physx::PxDefaultMemoryInputData input( buf.getData(), buf.getSize() );
			physx::PxTriangleMesh* trimesh = s_Physics->createTriangleMesh( input );
			physx::PxTriangleMeshGeometry triangleGeometry = physx::PxTriangleMeshGeometry( trimesh, physx::PxMeshScale( glmVec3ToPx( submeshScale * scale ) ) );
			physx::PxMaterial* material = s_Physics->createMaterial( 0, 0, 0 ); // Dummy material, will be replaced at runtime.
			physx::PxShape* shape = s_Physics->createShape( triangleGeometry, *material, true );
			shape->setLocalPose( glmTransformToPx( submeshTranslation, submeshRotation ) );
			shapes.push_back( shape );
		}

		if( collider.ProcessedMeshes.size() <= 0 )
		{
			for( auto shape : shapes )
			{
				physx::PxTriangleMeshGeometry triangleGeometry;
				shape->getTriangleMeshGeometry( triangleGeometry );
				physx::PxTriangleMesh* mesh = triangleGeometry.triangleMesh;

				const uint32_t nbVerts = mesh->getNbVertices();
				const physx::PxVec3* triangleVertices = mesh->getVertices();
				const uint32_t nbTriangles = mesh->getNbTriangles();
				const physx::PxU16* tris = ( const physx::PxU16* )mesh->getTriangles();

				std::vector<Vertex> vertices;
				std::vector<Index> indices;

				for( uint32_t v = 0; v < nbVerts; v++ )
				{
					Vertex v1;
					v1.Position = PxVec3ToGLM( triangleVertices[ v ] );
					vertices.push_back( v1 );
				}

				for( uint32_t tri = 0; tri < nbTriangles; tri++ )
				{
					Index index;
					index.V1 = tris[ 3 * tri + 0 ];
					index.V2 = tris[ 3 * tri + 1 ];
					index.V3 = tris[ 3 * tri + 2 ];
					indices.push_back( index );
				}

				glm::mat4 scale = glm::scale( glm::mat4( 1.0f ), *( glm::vec3* )&triangleGeometry.scale.scale );
				//scale = glm::mat4(1.0f);
				glm::mat4 transform = PxTransformToGlm( shape->getLocalPose() ) * scale;
				collider.ProcessedMeshes.push_back( Ref<Mesh>::Create( vertices, indices, transform ) );
			}
		}

		return shapes;
	}

	void PhysXFnd::AddRigidBody( Entity& entity )
	{
		auto& rb = entity.GetComponent<PhysXRigidbodyComponent>();
		auto& trans = entity.GetComponent<TransformComponent>();
		rb.m_Rigidbody = new PhysXRigidbody( entity, trans.Position, trans.Rotation );
		rb.m_Rigidbody->Init();
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

	void PhysXAssertCallback::operator()( const char* exp, const char* file, int line, bool& ignore )
	{
		SAT_CORE_ERROR( "PhysX : {0}, File : {1}, Line : {2}", exp, file, line );
	}

}
