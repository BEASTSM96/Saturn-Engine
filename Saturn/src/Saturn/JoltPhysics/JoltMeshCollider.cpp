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
#include "JoltMeshCollider.h"

#include "JoltConversions.h"
#include "JoltMeshColliderStream.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>

namespace Saturn {

	struct ColliderHeaderData
	{
		const char pHeader[ 5 ] = "SMC\0";
		uint64_t ID = 0;
		size_t Submeshes = 0;
	};

	JoltMeshCollider::JoltMeshCollider( const Ref<StaticMesh>& rStaticMesh, const glm::vec3& rScale )
		: m_StaticMesh( rStaticMesh ), m_Scale( rScale )
	{
	}

	JoltMeshCollider::~JoltMeshCollider()
	{
	}

	void JoltMeshCollider::Load()
	{
		std::ifstream stream( Project::GetActiveProject()->FilepathAbs( Path ), std::ios::binary | std::ios::ate );

		Buffer fileBuf;

		auto end = stream.tellg();
		stream.seekg( 0, std::ios::beg );
		auto size = end - stream.tellg();

		fileBuf.Allocate( ( uint32_t ) size );
		stream.read( reinterpret_cast<char*>( fileBuf.Data ), fileBuf.Size );
		
		stream.close();

		ColliderHeaderData hd = *( ColliderHeaderData* ) fileBuf.Data;
		
		if( strcmp( hd.pHeader, "SMC\0" ) )
		{
			SAT_CORE_ERROR( "Invaild file header!" );
		}

		// Read the collider data.
		//  sizeof( ColliderHeaderData ) as an offset.
		uint8_t* colliderData = fileBuf.As<uint8_t>() + sizeof( ColliderHeaderData );

		for( uint32_t i = 0; i < hd.Submeshes; i++ )
		{
			SubmeshColliderData data{};
			
			// Index
			uint32_t index = *( uint32_t* )colliderData;

			colliderData += sizeof( uint32_t ); // Sizeof the index

			uint32_t size = *( uint32_t* ) colliderData + sizeof( uint32_t );

			colliderData += sizeof( uint32_t ); // Sizeof the size

			data.Index = index;

			data.Buffer = Buffer::Copy( colliderData, size );

			colliderData += size;

			m_SubmeshData.push_back( data );
		}

		fileBuf.Free();

		// Create the shape
		for( uint32_t i = 0; i < hd.Submeshes; i++ )
		{
			const auto& rData = m_SubmeshData[ i ];

			JoltMeshColliderReader reader( rData.Buffer );
			JPH::Shape::ShapeResult result = JPH::Shape::sRestoreFromBinaryState( reader );

			const auto& rShape = result.Get();

			m_Shapes.push_back( rShape );
		}
	}

	void JoltMeshCollider::Save()
	{
		Create();

		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();

		if( !std::filesystem::exists( cachePath ) )
			std::filesystem::create_directories( cachePath );

		cachePath /= Name;
		cachePath.replace_extension( ".smcs" );

		Path = cachePath;

		// MeshCollider file structure.
		// Header
		// Static mesh ID
		// Physics shapes

		ColliderHeaderData hd{};
		hd.ID = m_StaticMesh->ID;
		hd.Submeshes = m_StaticMesh->Submeshes().size();

		std::ofstream fout( cachePath, std::ios::binary | std::ios::trunc );

		fout.write( reinterpret_cast<char*>( &hd ), sizeof( ColliderHeaderData ) );

		for( auto& rMeshData : m_SubmeshData )
		{
			fout.write( reinterpret_cast< char* >( &rMeshData.Index ), sizeof( uint32_t ) );

			fout.write( reinterpret_cast< char* >( &rMeshData.Buffer.Size ), sizeof( rMeshData.Buffer.Size ) );
			fout.write( reinterpret_cast< char* >( rMeshData.Buffer.Data ), rMeshData.Buffer.Size );
		}
	}

	void JoltMeshCollider::CreateBodies()
	{
	}

	void JoltMeshCollider::Create()
	{
		const auto& vertices = m_StaticMesh->Vertices();
		const auto& indices = m_StaticMesh->Indices();

		uint32_t Index = 0;
		for( auto& rSubmesh : m_StaticMesh->Submeshes() )
		{
			JPH::VertexList list;
			JPH::IndexedTriangleList triList;

			for( uint32_t i = rSubmesh.BaseVertex; i < rSubmesh.BaseVertex + rSubmesh.VertexCount; i++ )
			{
				const auto& rVertex = vertices[ i ];

				list.push_back( JPH::Float3( rVertex.Position.x, rVertex.Position.y, rVertex.Position.z ) );
			}

			// We divide by three because there is 3 faces in one index. 
			for( uint32_t j = rSubmesh.BaseIndex / 3; j < rSubmesh.BaseIndex / 3 + rSubmesh.IndexCount / 3; j++ )
			{
				const auto& rIndex = indices[ j ];

				triList.push_back( JPH::IndexedTriangle( rIndex.V1, rIndex.V2, rIndex.V3, 0 ) );
			}

			JPH::RefConst<JPH::MeshShapeSettings> MeshSettings = new JPH::MeshShapeSettings( list, triList );
			JPH::RefConst<JPH::ScaledShapeSettings> Settings = new JPH::ScaledShapeSettings( MeshSettings, Auxiliary::GLMToJPH( m_Scale ) );

			JPH::Shape::ShapeResult result = Settings->Create();
			const auto& rShape = result.Get();

			if( result.HasError() )
			{
				SAT_CORE_ERROR( "Failed to created mesh collider: {0}. Moving on to the next submesh...", result.GetError() );
				continue;
			}

			JoltMeshColliderWriter writer;
			rShape->SaveBinaryState( writer );

			SubmeshColliderData cd{};
			cd.Index = Index;
			cd.Buffer.Zero_Memory();
			cd.Buffer = writer.ToBuffer();

			m_SubmeshData.push_back( cd );

			m_Shapes.push_back( result.Get() );

			Index++;
		}
	}

}