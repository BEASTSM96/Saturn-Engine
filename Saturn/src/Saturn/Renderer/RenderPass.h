#pragma once

#include "Saturn/Core.h"
#include "Framebuffer.h"

namespace Saturn {

	struct RenderPassSpecification
	{
		RefSR<Framebuffer> TargetFramebuffer;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification & spec);
	};

}