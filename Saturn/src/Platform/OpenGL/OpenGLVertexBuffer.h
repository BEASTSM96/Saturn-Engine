#pragma once

#include "Saturn/Renderer/VertexBuffer.h"

#include "Saturn/Core/Buffer.h"

namespace Saturn {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* data, u32 size, VertexBufferUsage usage = VertexBufferUsage::Static);
		OpenGLVertexBuffer(u32 size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
		virtual ~OpenGLVertexBuffer();

		virtual void SetData(void* data, u32 size, u32 offset = 0);
		virtual void Bind() const;

		virtual const VertexBufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

		virtual u32 GetSize() const { return m_Size; }
		virtual RendererID GetRendererID() const { return m_RendererID; }
	private:
		RendererID m_RendererID = 0;
		u32 m_Size;
		VertexBufferUsage m_Usage;
		VertexBufferLayout m_Layout;

		Buffer m_LocalData;
	};

}