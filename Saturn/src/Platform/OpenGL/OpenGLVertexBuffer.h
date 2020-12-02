#pragma once

#include "Saturn/Renderer/VertexBuffer.h"

#include "Saturn/Core/Buffer.h"

namespace Saturn {

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* data, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
		OpenGLVertexBuffer(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
		virtual ~OpenGLVertexBuffer();

		virtual void SetData(void* data, uint32_t size, uint32_t offset = 0);
		virtual void Bind( void ) const;

		virtual const VertexBufferLayout& GetLayout( void ) const override { return m_Layout; }
		virtual void SetLayout(const VertexBufferLayout& layout) override { m_Layout = layout; }

		virtual uint32_t GetSize( void ) const { return m_Size; }
		virtual RendererID GetRendererID( void ) const { return m_RendererID; }
	private:
		RendererID m_RendererID = 0;
		uint32_t m_Size;
		VertexBufferUsage m_Usage;
		VertexBufferLayout m_Layout;

		Buffer m_LocalData;
	};


}