#pragma once

#include "PhysXCollider.h"

namespace Saturn::Physics {

	class PhysXBoxCollider : PhysXCollider
	{
	public:
		PhysXBoxCollider();
		PhysXBoxCollider(PhysXWorld* world);
		~PhysXBoxCollider();


		PhysXBoxCollider& operator=(const PhysXBoxCollider&) = delete;

		virtual inline PhysXBoxCollider* GetAsPhysXBoxCollider() { return this; }
	private:

		void SetCenter(const glm::vec3& center);

		glm::vec3 GetCenter() const;

		void SetSize(const glm::vec3& size);

		glm::vec3 GetSize() const;

		void Scale(const glm::vec3& scaling);

	};
}