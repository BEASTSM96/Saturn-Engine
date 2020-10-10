#pragma once

#include <glm/glm.hpp>
#include "Saturn/Core.h"
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
		uint32_t Width = 1280, Height = 720;
		glm::vec3 ClearColor;
		FramebufferFormat Format;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class SATURN_API Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID() const = 0;

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};


	class FramebufferPool final
	{
	public:
		FramebufferPool(uint32_t maxFBs = 32);
		~FramebufferPool();

		std::weak_ptr<Framebuffer> AllocateBuffer();
		void Add(std::weak_ptr<Framebuffer> framebuffer);

		const std::vector<std::weak_ptr<Framebuffer>>& GetAll() const { return m_Pool; }

		inline static FramebufferPool* GetGlobal() { return s_Instance; }
	private:
		std::vector<std::weak_ptr<Framebuffer>> m_Pool;

		static FramebufferPool* s_Instance;
	};

}