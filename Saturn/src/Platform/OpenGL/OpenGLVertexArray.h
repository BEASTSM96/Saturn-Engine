#pragma once

#include "Saturn/Renderer\VertexArray.h"


namespace Saturn {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const RefSR<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const RefSR<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<RefSR<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		virtual const RefSR<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		RefSR<IndexBuffer> m_IndexBuffer;
	};

}