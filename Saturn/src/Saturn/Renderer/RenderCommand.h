#pragma once


#include "RendererAPI.h"

namespace Saturn {

	class SATURN_API RenderCommand
	{
	public:

		SAT_FORCE_INLINE static void Init()
		{
			s_RendererAPI->Init();
		}

		SAT_FORCE_INLINE static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}


		SAT_FORCE_INLINE static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		SAT_FORCE_INLINE static void Clear()
		{
			s_RendererAPI->Clear();
		}

		SAT_FORCE_INLINE static void DrawIndexed(const Ref<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};
}