#pragma once

#include "PhysXPhysics.h"
#include "PhysXMaterial.h"
#include "PhysXRigidBody.h"

#include <vector>
#include <glm/glm.hpp>

namespace Saturn::Physics {

	class PhysXCollider
	{
	public:
		PhysXCollider() = delete;
		PhysXCollider(const PhysXCollider&) =delete;
		PhysXCollider(PhysXWorld* world, std::vector<physx::PxShape*> shapes);
		virtual ~PhysXCollider();

		void UpdateShape(std::vector<physx::PxShape*> newShapes);

		PhysXCollider& operator=(const PhysXCollider&) = delete;

	public:
		PhysXWorld* GetWorld() 
		{ 
			if (m_World)
			{
				return m_World;
			}
			else
			{
				return nullptr;
			}
		}
	protected:
		std::vector<physx::PxShape*>& GetShapes();

		const std::vector<physx::PxShape*>& GetShapes() const;

	private:
		std::vector<physx::PxShape*> m_Shapes;

		PhysXRigidBody* m_RigidBody = nullptr;

		PhysXWorld* m_World = nullptr;

		void SetAsTrigger(bool mustBeATrigger);

		bool IsATrigger() const;

		void SetFilterGroup(FilterGroup group);

		FilterGroup GetFilterGroup() const;

		void SetMaterial(const std::string& name);

		void SetPosition(const glm::vec3& position);

		void SetRotation(const glm::quat& rotation);
	private:
		template <typename T>
		T* GetDataAs();

		template <typename T>
		const T* GetDataAs() const;

		template <typename T>
		inline T* PhysXCollider::GetDataAs()
		{
			return static_cast<T*>(getData()->data);
		}

		template <typename T>
		inline const T* PhysXCollider::GetDataAs(void) const
		{
			return static_cast<const T*>(getData()->data);
		}

	};
}