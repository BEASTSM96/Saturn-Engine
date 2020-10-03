#include "sppch.h"
#include "Framebuffer.h"

#include "Saturn/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Saturn {

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
<<<<<<< HEAD
		Ref<Framebuffer> result = nullptr;

		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    SAT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLFramebuffer>::Create(spec);
		}

		FramebufferPool::GetGlobal()->Add(result);
		return result;
	}

	FramebufferPool* FramebufferPool::s_Instance = new FramebufferPool;

	FramebufferPool::FramebufferPool(uint32_t maxFBs /* = 32 */)
	{

	}

	FramebufferPool::~FramebufferPool()
	{
=======
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    SAT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
		}
>>>>>>> parent of 0ef25b2... TestCommit

		SAT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}


}