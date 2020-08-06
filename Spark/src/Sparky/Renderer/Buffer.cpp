#include "sppch.h"
#include "Buffer.h"
#include "Renderer.h"

#include "Platform\OpenGL\OpenGLBuffer.h"

namespace Sparky {

	VertexBuffer * VertexBuffer::Create(float * vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: SP_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLVertexBuffer(vertices, size);
		}

		SP_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	IndexBuffer * IndexBuffer::Create(uint32_t * indices, uint32_t size)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: SP_CORE_ASSERT(false, "RendererAPI none"); return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLIndexBuffer(indices, size);
		}

		SP_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}