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
#include "PhysicsCooking.h"

#include "PhysicsAuxiliary.h"
#include "PhysicsFoundation.h"

#include "Saturn/Core/Math.h"

namespace Saturn {

	PhysicsCooking::PhysicsCooking()
	{
		physx::PxCookingParams Params( PhysicsFoundation::Get().m_Physics->getTolerancesScale() );
		m_Cooking = PxCreateCooking( PX_PHYSICS_VERSION, *PhysicsFoundation::Get().m_Foundation, Params );
	}

	PhysicsCooking::~PhysicsCooking()
	{
		Terminate();
	}

	void PhysicsCooking::Terminate()
	{
		PHYSX_TERMINATE_ITEM( m_Cooking );
	}

	void PhysicsCooking::CookMeshCollider( const Ref<StaticMesh>& rMesh )
	{
		// Create the triangle mesh.
		// TODO: Cook each submesh

		for( auto& rSubmesh : rMesh->Submeshes() )
		{
			physx::PxTriangleMeshDesc MeshDesc;
			MeshDesc.points.data = &rMesh->Vertices()[ rSubmesh.BaseVertex ];
			MeshDesc.points.count = rSubmesh.VertexCount;
			MeshDesc.points.stride = sizeof( StaticVertex );

			MeshDesc.triangles.data = &rMesh->Indices()[ rSubmesh.BaseIndex / 3 ];
			MeshDesc.triangles.count = rSubmesh.IndexCount / 3;
			MeshDesc.triangles.stride = sizeof( Index );

			physx::PxDefaultMemoryOutputStream stream;
			if( m_Cooking->cookTriangleMesh( MeshDesc, stream ) )
			{
				SubmeshColliderData data{};
				data.Index = 0;
				data.Stream = Buffer::Copy( stream.getData(), stream.getSize() );

				m_SubmeshData.push_back( data );
			}
		}

		WriteCache( rMesh );
		ClearCache();
	}

	void PhysicsCooking::LoadColliderFile( const std::filesystem::path& rPath )
	{
		Buffer fileBuffer;
		std::ifstream stream( rPath, std::ios::binary | std::ios::ate );

		auto end = stream.tellg();
		stream.seekg( 0, std::ios::beg );
		auto size = end - stream.tellg();

		fileBuffer.Allocate( ( size_t ) size );
		stream.read( reinterpret_cast< char* >( fileBuffer.Data ), fileBuffer.Size );

		stream.close();

		MeshCacheHeader hd = *( MeshCacheHeader* ) fileBuffer.Data;

		if( strcmp( hd.pHeader, "SMC\0" ) )
		{
			SAT_CORE_ASSERT( false, "Invaild file header!" );
		}

		uint8_t* colliderData = fileBuffer.As<uint8_t>() + sizeof( MeshCacheHeader );

		for( uint32_t i = 0; i < hd.Submeshes; i++ )
		{
			SubmeshColliderData submesh{};

			uint32_t index = *( uint32_t* ) colliderData;

			colliderData += sizeof( uint32_t );

			size_t size = *( size_t* ) colliderData;

			colliderData += sizeof( size_t );

			submesh.Index = i;
			submesh.Stream = Buffer::Copy( colliderData, size );

			m_SubmeshData.push_back( submesh );

			colliderData += size;
		}

		fileBuffer.Free();
	}

	std::vector<physx::PxShape*> PhysicsCooking::UncookMeshCollider( const Ref<StaticMesh>& rMesh, physx::PxRigidActor& rActor, glm::vec3 Scale )
	{
		std::vector<physx::PxShape*> Shapes;

		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();
		cachePath /= rMesh->GetName();
		cachePath.replace_extension( ".smcs" );

		LoadColliderFile( cachePath );

		// TEMP: Until we have physics material assets.
		physx::PxMaterial* mat = PhysicsFoundation::Get().GetPhysics().createMaterial( 0, 0, 0 );

		// TEMP: We might want to change the filter data.
		physx::PxFilterData data;
		data.word0 = BIT( 0 );
		data.word1 = BIT( 0 );

		int i = 0;
		for( const auto& rCookedData : m_SubmeshData )
		{
			const Submesh& rSubmesh = rMesh->Submeshes()[ i ];

			// Read the cooked data.
			physx::PxDefaultMemoryInputData readBuffer( rCookedData.Stream.Data, static_cast< physx::PxU32 >( rCookedData.Stream.Size ) );
			physx::PxTriangleMesh* mesh = PhysicsFoundation::Get().GetPhysics().createTriangleMesh( readBuffer );

			// Create the shape.
			glm::vec3 submeshPosition, submeshRotation, submeshScale;
			Math::DecomposeTransform( rSubmesh.Transform, submeshPosition, submeshRotation, submeshScale );

			physx::PxVec3 ShapeScale = Auxiliary::GLMToPx( submeshScale * Scale );
			physx::PxMeshScale MeshScale( ShapeScale );

			physx::PxTriangleMeshGeometry MeshGeometry( mesh, ShapeScale );

			physx::PxShape* pShape = PhysicsFoundation::Get().GetPhysics().createShape( MeshGeometry, *mat, true );
			pShape->setFlag( physx::PxShapeFlag::eSIMULATION_SHAPE, true ); // We will always set it to be part of the simulation.
			pShape->setFlag( physx::PxShapeFlag::eTRIGGER_SHAPE, false );

			pShape->setLocalPose( Auxiliary::GLMTransformToPx( submeshPosition, submeshRotation ) );

			pShape->setSimulationFilterData( data );

			mesh->release();

			rActor.attachShape( *pShape );

			Shapes.push_back( pShape );

			i++;
		}

		ClearCache();

		return Shapes;
	}

	void PhysicsCooking::WriteCache( const Ref<StaticMesh>& rMesh )
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();

		if( !std::filesystem::exists( cachePath ) )
			std::filesystem::create_directories( cachePath );

		cachePath /= rMesh->GetName();
		cachePath.replace_extension(".smcs" );

		MeshCacheHeader hd{};
		hd.ID = rMesh->ID;
		hd.Submeshes = rMesh->Submeshes().size();

		std::ofstream fout( cachePath, std::ios::binary | std::ios::trunc );

		fout.write( reinterpret_cast< char* >( &hd ), sizeof( MeshCacheHeader ) );

		for( auto& rMeshData : m_SubmeshData )
		{
			fout.write( reinterpret_cast<char*>( &rMeshData.Index ), sizeof( uint32_t ) );

			fout.write( reinterpret_cast< char* >( &rMeshData.Stream.Size ), sizeof( size_t ) );

			fout.write( reinterpret_cast< char* >( rMeshData.Stream.Data ), rMeshData.Stream.Size );
		}
	}

	void PhysicsCooking::ClearCache()
	{
		m_SubmeshData.clear();
	}
}