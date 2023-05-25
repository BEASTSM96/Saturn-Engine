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

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>

namespace Saturn {

	JoltMeshCollider::JoltMeshCollider( const Ref<StaticMesh>& rStaticMesh, const glm::vec3& rScale )
		: m_StaticMesh( rStaticMesh ), m_Scale( rScale )
	{
	}

	JoltMeshCollider::~JoltMeshCollider()
	{

	}

	void JoltMeshCollider::Create()
	{
		const auto& vertices = m_StaticMesh->Vertices();
		const auto& indices = m_StaticMesh->Indices();

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

			if( result.HasError() )
			{
				SAT_CORE_ERROR( "Failed to created mesh collider: {0}. Moving on to the next submesh...", result.GetError() );
				continue;
			}

			m_Shapes.push_back( result.Get() );
		}
	}

}