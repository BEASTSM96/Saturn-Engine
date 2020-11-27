#pragma once

#include "Saturn/Renderer/IndexBuffer.h"

#include "Saturn/Core/Buffer.h"
#include "Saturn/Core/Base.h"


namespace Saturn {

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(u32 size);
		OpenGLIndexBuffer(void* data, u32 size);
		virtual ~OpenGLIndexBuffer();

		virtual void SetData(void* data, u32 size, u32 offset = 0);
		virtual void Bind() const;

		virtual u32 GetCount() const { return m_Size / sizeof(u32); }

		virtual u32 GetSize() const { return m_Size; }
		virtual RendererID GetRendererID() const { return m_RendererID; }
	private:
		RendererID m_RendererID = 0;
		u32 m_Size;

		Buffer m_LocalData;
	};

}