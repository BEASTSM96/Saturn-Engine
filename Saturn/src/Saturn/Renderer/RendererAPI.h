#pragma once

#include "VertexArray.h"

namespace Saturn {

	using RendererID = uint32_t;

	enum class RendererAPIType
	{
		None,
		OpenGL,
		DX,
		Vulkan
	};

	// TODO: move into separate header
	enum class PrimitiveType
	{
		None = 0, Triangles, Lines
	};


	class SATURN_API RendererAPI
	{
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;


		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		static RendererAPIType Current() { return s_CurrentRendererAPI; }
	private:
		static RendererAPIType s_CurrentRendererAPI;
	};
}