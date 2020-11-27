#pragma once

#include "Saturn/Renderer/Framebuffer.h"

namespace Saturn {

	class Application;
	class GameObject;

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		virtual void Resize(u32 width, u32 height, bool forceRecreate = false) override;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void BindTexture(u32 slot = 0) const override;

		virtual RendererID GetRendererID() const { return m_RendererID; }
		virtual RendererID GetColorAttachmentRendererID() const { return m_ColorAttachment; }
		virtual RendererID GetDepthAttachmentRendererID() const { return m_DepthAttachment; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }
	private:
		FramebufferSpecification m_Specification;
		RendererID m_RendererID = 0;
		RendererID m_ColorAttachment = 0, m_DepthAttachment = 0;
	};
}