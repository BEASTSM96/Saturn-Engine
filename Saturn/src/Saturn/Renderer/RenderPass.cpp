#include "sppch.h"
#include "RenderPass.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLRenderPass.h"

namespace Saturn {

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    SAT_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLRenderPass>::Create(spec);
		}

		SAT_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}