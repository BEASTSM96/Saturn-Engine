#pragma once

#include <glm/glm.hpp>
#include "Saturn/Core/UUID.h"

namespace Saturn {

	struct Component {};

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

	struct TransformComponent : Component
	{
		glm::vec3  Position =		{ 0.0f , 0.0f, 0.0f };
		glm::vec3  Rotation =		{ 0.0f , 0.0f, 0.0f };
		glm::vec3  Scale	=		{ 1.0f , 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& Position)
			: Position(Position) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
				* glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
				* glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

			return glm::translate(glm::mat4(1.0f), Position)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}

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
	struct SpriteRendererComponent : Component
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
	struct TagComponent : Component
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
	struct IdComponent : Component
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

	class Mesh;

	struct MeshComponent : Component
	{
		Ref<Saturn::Mesh> Mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;
		MeshComponent(Ref<Saturn::Mesh>& model)
			: Mesh(model) {}

		operator Ref<Saturn::Mesh>() { return Mesh; }
	};

	/** @brief RelationshipComponent.
	*
	*/

	struct RelationshipComponent : Component
	{
		entt::entity Parent{ entt::null };
		std::unordered_set < entt::entity > Children;

		glm::mat4 RelativeTransform = glm::mat4{ 0.0f };
	};
}
