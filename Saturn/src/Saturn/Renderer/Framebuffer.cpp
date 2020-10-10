#include "sppch.h"
#include "Framebuffer.h"

#include "Saturn/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Saturn {

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		Ref<Framebuffer> result = nullptr;
		switch (Renderer::GetAPI())
		{
			case RendererAPIType::None:    SAT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPIType::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI!");
		FramebufferPool::GetGlobal()->Add(result);
		return nullptr;
	}

	/// <summary>
	/// FramebufferPool
	/// </summary>

	FramebufferPool* FramebufferPool::s_Instance = new FramebufferPool;

	FramebufferPool::FramebufferPool(uint32_t maxFBs /* = 32 */)
	{

	}

	FramebufferPool::~FramebufferPool()
	{

	}

	std::weak_ptr<Framebuffer> FramebufferPool::AllocateBuffer()
	{
		return std::weak_ptr<Framebuffer>();
	}

	void FramebufferPool::Add(std::weak_ptr<Framebuffer> framebuffer)
	{
		m_Pool.push_back(framebuffer);
	}


}