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

#include "Saturn/Core/Base.h"
#include "Saturn/Scene/Entity.h"

#include "PxPhysicsAPI.h"

#include "PhysicsShapeTypes.h"

namespace Saturn {

	class PhysicsShape : public RefTarget
	{
	public:
		PhysicsShape( Entity entity ) { m_Entity = entity; }
		virtual ~PhysicsShape() = default;

		virtual void Create( physx::PxRigidActor& rActor ) = 0;
		virtual void Detach( physx::PxRigidActor& rActor );

		// Only use this for basic shapes as this only works for one shape.
		void SetFilterData();

	protected:
		ShapeType m_Type = ShapeType::Unknown;

		Entity m_Entity;

		physx::PxShape* m_Shape = nullptr;
	};

	class BoxShape : public PhysicsShape
	{
	public:
		BoxShape( Entity entity );
		~BoxShape();

		void Create( physx::PxRigidActor& rActor ) override;

	private:
		float m_Extent = 0.0f;
	};

	class SphereShape : public PhysicsShape
	{
	public:
		SphereShape( Entity entity );
		~SphereShape();

		void Create( physx::PxRigidActor& rActor ) override;

	private:
		float m_Radius = 0.0f;
	};

	class CapusleShape : public PhysicsShape
	{
	public:
		CapusleShape( Entity entity );
		~CapusleShape();

		void Create( physx::PxRigidActor& rActor ) override;

	private:
		float m_Height = 0.0f;
		float m_Radius = 0.0f;
	};

	class TriangleMeshShape : public PhysicsShape
	{
	public:
		TriangleMeshShape( Entity entity );
		~TriangleMeshShape();

		// This assumes the the mesh collider has already been cooked.
		void Create( physx::PxRigidActor& rActor ) override;
		virtual void Detach( physx::PxRigidActor& rActor ) override;

	private:
		Ref<StaticMesh> m_Mesh;
		std::vector<physx::PxShape*> m_Shapes;
	};

	class ConvexMeshShape : public PhysicsShape
	{
	public:
		ConvexMeshShape( Entity entity );
		~ConvexMeshShape();

		// This assumes the the mesh collider has already been cooked.
		void Create( physx::PxRigidActor& rActor ) override;
		virtual void Detach( physx::PxRigidActor& rActor ) override;

	private:
		Ref<StaticMesh> m_Mesh;
		std::vector<physx::PxShape*> m_Shapes;
	};
}