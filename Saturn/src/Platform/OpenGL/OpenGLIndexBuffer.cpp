#include "sppch.h"
#include "OpenGLIndexBuffer.h"

#include <glad/glad.h>

#include "Saturn/Renderer/Renderer.h"

namespace Saturn {

	OpenGLIndexBuffer::OpenGLIndexBuffer(void* data, uint32_t size)
		: m_RendererID(0), m_Size(size)
	{
		m_LocalData = Buffer::Copy(data, size);

		Ref<OpenGLIndexBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			glCreateBuffers(1, &instance->m_RendererID);
			glNamedBufferData(instance->m_RendererID, instance->m_Size, instance->m_LocalData.Data, GL_STATIC_DRAW);
		});
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t size)
		: m_Size(size)
	{
		// m_LocalData = Buffer(size);

		Ref<OpenGLIndexBuffer> instance = this;
		Renderer::Submit([instance]() mutable {
			glCreateBuffers(1, &instance->m_RendererID);
			glNamedBufferData(instance->m_RendererID, instance->m_Size, nullptr, GL_DYNAMIC_DRAW);
		});
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		GLuint rendererID = m_RendererID;
		Renderer::Submit([rendererID]() {
			glDeleteBuffers(1, &rendererID);
		});
	}

	void OpenGLIndexBuffer::SetData(void* data, uint32_t size, uint32_t offset)
	{
		m_LocalData = Buffer::Copy(data, size);
		m_Size = size;
		Ref<OpenGLIndexBuffer> instance = this;
		Renderer::Submit([instance, offset]() {
			glNamedBufferSubData(instance->m_RendererID, offset, instance->m_Size, instance->m_LocalData.Data);
		});
	}

	void OpenGLIndexBuffer::Bind() const
	{
		Ref<const OpenGLIndexBuffer> instance = this;
		Renderer::Submit([instance]() {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->m_RendererID);
		});
	}

}
