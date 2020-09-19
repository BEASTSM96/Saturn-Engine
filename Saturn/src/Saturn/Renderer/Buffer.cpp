#include "sppch.h"
#include "Buffer.h"
#include "Renderer.h"

#include "Platform\OpenGL\OpenGLBuffer.h"

namespace Saturn {

	VertexBuffer * VertexBuffer::Create(float * vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: SAT_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLVertexBuffer(vertices, size);
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	IndexBuffer * IndexBuffer::Create(uint32_t * indices, uint32_t size)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: SAT_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLIndexBuffer(indices, size);
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}