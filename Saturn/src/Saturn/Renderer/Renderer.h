#pragma once

#include "RenderCommand.h"

#include "OrthographicCamera.h"

#include "Saturn/Renderer/3DCamera.h"
#include "3D/3dShader.h"
#include "Shader.h"

#include "RenderCommandQueue.h"
#include "Saturn/Renderer/Material.h"

namespace Saturn {

	class ShaderLibrary;

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


	class SATURN_API Renderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);

		/*Commands*/

		static void Clear();
		static void Clear(float r, float g, float b, float a = 1.0f);
		static void SetClearColor(float r, float g, float b, float a);
		static void DrawIndexed(uint32_t count, PrimitiveType type, bool depthTest = true);

		/*OpenGL*/
		static void SetLineThickness(float thickness);
		static void ClearMagenta();
		static void Init();

		static Ref<ShaderLibrary> GetShaderLibrary();

		template<typename FuncT>
		static void Submit(FuncT&& func)
		{
			auto renderCmd = [](void* ptr) {
				auto pFunc = (FuncT*)ptr;
				(*pFunc)();

				// NOTE: Instead of destroying we could try and enforce all items to be trivally destructible
				// however some items like uniforms which contain std::strings still exist for now
				// static_assert(std::is_trivially_destructible_v<FuncT>, "FuncT must be trivially destructible");
				pFunc->~FuncT();
			};
			auto storageBuffer = GetRenderCommandQueue().Allocate(renderCmd, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}

		static void WaitAndRender();

		/*Render stuff*/
		static void BeginRenderPass(RenderPass renderPass, bool clear = true);
		static void EndRenderPass();

		static void SubmitQuad(Material material, const glm::mat4& transform = glm::mat4(1.0f));
		static void SubmitFullscreenQuad(Material material);
		static void SubmitMesh(Mesh mesh, const glm::mat4& transform, Material overrideMaterial);

		static void DrawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));
		static void DrawAABB(Mesh mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f));

		static RendererAPIType GetAPI() { return RendererAPI::Current(); }
		static void OnWindowResize(uint32_t width, uint32_t height);
	private:
		static RenderCommandQueue& GetRenderCommandQueue();
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}