#pragma once

#include <glm/glm.hpp>

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
	* float Id;
	* 
	* std::string Idname;
	*
	* IdComponent()
	* IdComponent(const IdComponent&) = default
	* IdComponent(const std::string& id)
	*
	* 
	* @endcode
	*/
	struct IdComponent
	{
		float Id;

		std::string Idname;

		IdComponent() = default;
		IdComponent(const IdComponent&) = default;
		IdComponent(const float& id, const std::string& idname)
			: Id(id), Idname(idname) {}
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
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(Model * model)
			: m_Model(model) {}
	public:
		Model* GetModel() { return m_Model; };
	private:
		Model * m_Model;

	};


}