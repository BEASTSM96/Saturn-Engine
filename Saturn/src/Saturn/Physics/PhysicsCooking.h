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

#pragma once

#include "Saturn/Vulkan/Mesh.h"

#include "PxPhysicsAPI.h"

namespace Saturn {

	// File header
	struct MeshCacheHeader
	{
		const char pHeader[ 5 ] = "SMC\0";
		uint64_t ID = 0;
		size_t Submeshes = 0;
		ShapeType Type;
	};

	// Data for each submesh
	struct SubmeshColliderData
	{
		uint32_t Index;
		Buffer Stream;
	};

	class PhysicsCooking
	{
	public:
		static inline PhysicsCooking& Get() { return *SingletonStorage::GetOrCreateSingleton<PhysicsCooking>(); }
	public:
		PhysicsCooking();
		~PhysicsCooking();

		// Cook mesh collider to a triangle mesh, if the collider cache does not exist we will create it if it does exist we will not override it and we will not cook the mesh.
		// For Static meshes only!
		bool CookMeshCollider( const Ref<StaticMesh>& rMesh, ShapeType Type );

		std::vector<physx::PxShape*> CreateTriangleMesh( const Ref<StaticMesh>& rMesh, physx::PxRigidActor& rActor, glm::vec3 Scale );

		std::vector<physx::PxShape*> CreateConvexMesh( const Ref<StaticMesh>& rMesh, physx::PxRigidActor& rActor, glm::vec3 Scale );

	private:
		void Terminate();
		void ClearCache();
		void WriteCache( const Ref<StaticMesh>& rMesh, ShapeType Type );
		bool LoadColliderFile( const std::filesystem::path& rPath );

		bool TryCookTriangleMesh( const Ref<StaticMesh>& rMesh );
		bool TryCookConvexMesh( const Ref<StaticMesh>& rMesh );

	private:

		std::vector<SubmeshColliderData> m_SubmeshData;

	private:
		physx::PxCooking* m_Cooking = nullptr;
	};
}