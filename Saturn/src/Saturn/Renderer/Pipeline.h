#pragma once

#include "Saturn/Renderer/VertexBuffer.h"
#include "Saturn/Renderer/Shader.h"

namespace Saturn {

	struct PipelineSpecification
	{
		Ref<Saturn::Shader> Shader;
		VertexBufferLayout Layout;
	};

	class Pipeline : public RefCounted
	{
	public:
		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification( void ) = 0;
		virtual const PipelineSpecification& GetSpecification( void ) const = 0;

		virtual void Invalidate( void ) = 0;

		virtual void Bind( void ) = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& spec);
	};

}