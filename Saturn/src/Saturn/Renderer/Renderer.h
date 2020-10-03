#pragma once

#include "RenderCommand.h"

#include "OrthographicCamera.h"

#include "Saturn/Renderer/3DCamera.h"
#include "3D/3dShader.h"
#include "Shader.h"

#include "RenderCommandQueue.h"
#include "RenderPass.h"
#include "Saturn/Core/Math/AABB.h"
#include "Saturn/Renderer/Material.h"

namespace Saturn {

	class ShaderLibrary;

	class Renderer
	{
	public:
		typedef void(*RenderCommandFn)(void*);
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void Begin3DScene(SCamera& camera);
		static void EndScene();

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

		static RendererAPIType GetAPI() { return RendererAPI::Current(); }

		static void Init();

		static void OnWindowResize(uint32_t width, uint32_t height);
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};


}
