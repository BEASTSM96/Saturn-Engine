#pragma once

#include "Saturn/Core/Base.h"
#include "Framebuffer.h"

namespace Saturn {

	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFramebuffer;
	};

	class RenderPass : public RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification( void )  = 0;
		virtual const RenderPassSpecification& GetSpecification( void )  const = 0;

		static Ref<RenderPass> Create( const RenderPassSpecification& spec );
	};
}