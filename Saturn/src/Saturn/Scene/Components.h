#pragma once

#include <glm/glm.hpp>
#include "Saturn/Core/UUID.h"
#include "physx/PxPhysicsAPI.h"

namespace Saturn {

	/** @brief A TransformComponent.
	*
	* @code
	*
	* glm::mat4 Transform (default 1.0f)
	*
	* TransformComponent()
	* TransformComponent(const TransformComponent&)
	* TransformComponent(const glm::mat4 & Transform)
	*
	* 
	*	operator glm::mat4& ()
	*
	*	operator const glm::mat4& ()
	* 
	* @endcode
	*/

	struct TransformComponent
	{
		glm::mat4  Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4 & Transform) 
			: Transform(Transform) {}


		operator glm::mat4& () { return Transform; }

		operator const glm::mat4& () { return Transform; }

	};


	/** @brief A SpriteRendererComponent.
	*
	* @code
	* 
	* glm::vec4 Color (default  1.0f)
	* 
	* SpriteRendererComponent()
	* SpriteRendererComponent(const SpriteRendererComponent&)
	* SpriteRendererComponent(const glm::vec4& color)
	* 
	* @endcode
	*/
	struct SpriteRendererComponent
	{
		glm::vec4 Color { 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};


	/** @brief A TagComponent.
	*
	* @code
	*
	* std::string Tag;
	*
	* TagComponent()
	* TagComponent(const TagComponent&) = default
	* TagComponent(const std::string& tag)
	*
	* @endcode
	*/
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	/** @brief A IdComponent.
	*
	* @code
	* 
	* UUID ID;
	*
	* IdComponent()
	* IdComponent(const IdComponent&) = default
	* IdComponent(const UUID& uuid)
	* 
	* @endcode
	*/
	struct IdComponent
	{
		UUID ID;

		IdComponent() = default;
		IdComponent(const IdComponent&) = default;
		IdComponent(const UUID& uuid)
			: ID(uuid) {}
	};


	/** @brief A MeshComponent.
	*
	* @code
	*
	* Model * Model;
	*
	*
	* MeshComponent()
	* MeshComponent(const MeshComponent&) = default
	* MeshComponent(const Model& Model)
	*
	*
	* @endcode
	*/

	class Model;

	struct MeshComponent
	{
	public:
		Model* GetModel() { return m_Model; }
	private:
		Model* m_Model = nullptr;
	public:
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(Model * model)
			: m_Model(model) {}
	};

	/** @brief RelationshipComponent.
	*
	*/

	struct RelationshipComponent 
	{
		entt::entity Parent{ entt::null };
		std::unordered_set < entt::entity > Children;

		glm::mat4 RelativeTransform = glm::mat4{ 0.0f };
	};

	/** @brief SkyboxComponent.
	*
	*/

	struct SkyboxComponent
	{
		float texture_inst;
		MeshComponent m_Mesh;
		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent&) = default;
		SkyboxComponent(MeshComponent mesh) : m_Mesh(mesh) {};
	};


	/** @brief BoxCollider.
	*
	*/

	class BoxCollision;

	struct BoxCollider
	{
		RefSR<BoxCollision>m_BoxCollision;

		BoxCollider() = default;
		BoxCollider(const BoxCollider&) = default;
		BoxCollider(RefSR<BoxCollision> BoxCollision) : m_BoxCollision(BoxCollision) {};
	};


	/** @brief RigidBodyComponent.
	*
	
	struct RigidBodyComponent
	{
		physx::PxRigidDynamic * m_RigidBody;

		physx::PxRigidDynamic * m_RigidBody = new physx::PxRigidDynamic();

		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
		RigidBodyComponent(physx::PxRigidDynamic * RigidBody) : m_RigidBody(RigidBody) {};
	};
	*/
}
