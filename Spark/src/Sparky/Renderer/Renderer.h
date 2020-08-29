#pragma once

#include "RenderCommand.h"

#include "OrthographicCamera.h"
#include "Shader.h"

namespace Sparky {

	using vector1 = glm::vec1;
	using vector2 = glm::vec2;
	using vector3 = glm::vec3;
	using vector4 = glm::vec4;
	using scale = glm::mat4;

	using transform = glm::mat4;

	struct FVector3 : vector3
	{
		FVector3(float x, float y, float z) : x(x), y(y), z(z) {  }
		float x;
		float y;
		float z;
	};

	struct FVector2 : vector2
	{
		FVector2(float x, float y) : x(x), y(y) {  }
		float x;
		float y;
	};

	struct FScale : scale
	{
		FScale(scale InScale) : InScale(InScale) {  }
		scale InScale;
	};



	struct FTransform : transform
	{
		FTransform(glm::vec3 loc, glm::mat4 scale, float rot) : loc(loc), scale(scale), rot(rot) {  }
		glm::vec3 loc;
		glm::mat4 scale;
		float rot;
	};


	class Renderer
	{
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, FTransform Intransform);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static void Init();
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};


}
