#pragma once

#include <glm/glm.hpp>
#include "Saturn/Core/Base.h"
#include "Saturn/Renderer/RendererAPI.h"


namespace Saturn {

	enum class FramebufferFormat
	{
		None = 0,
		RGBA8 = 1,
		RGBA16F = 2
	};

	struct FramebufferSpecification
	{
		u32 Width = 1280;
		u32 Height = 720;
		glm::vec4 ClearColor;
		FramebufferFormat Format;
		u32 Samples = 1; // multisampling

		// SwapChainTarget = screen buffer (i.e. no framebuffer)
		bool SwapChainTarget = false;
	};

	class Framebuffer : public RefCounted
	{
	public:
		virtual ~Framebuffer() {}
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Resize(u32 width, u32 height, bool forceRecreate = false) = 0;

		virtual void BindTexture(u32 slot = 0) const = 0;

		virtual RendererID GetRendererID() const = 0;
		virtual RendererID GetColorAttachmentRendererID() const = 0;
		virtual RendererID GetDepthAttachmentRendererID() const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};

	class FramebufferPool final
	{
	public:
		FramebufferPool(u32 maxFBs = 32);
		~FramebufferPool();

		std::weak_ptr<Framebuffer> AllocateBuffer();
		void Add(const Ref<Framebuffer>& framebuffer);

		std::vector<Ref<Framebuffer>>& GetAll() { return m_Pool; }
		const std::vector<Ref<Framebuffer>>& GetAll() const { return m_Pool; }

		inline static FramebufferPool* GetGlobal() { return s_Instance; }
	private:
		std::vector<Ref<Framebuffer>> m_Pool;

		static FramebufferPool* s_Instance;
	};
}