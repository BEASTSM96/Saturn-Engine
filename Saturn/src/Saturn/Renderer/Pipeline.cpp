#include "sppch.h"
#include "Pipeline.h"

#include "Renderer.h"

#include "Saturn/Platform/OpenGL/OpenGLPipeline.h"

namespace Saturn {

	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:    return nullptr;
		case RendererAPIType::OpenGL:  return Ref<OpenGLPipeline>::Create(spec);
		}
		SAT_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}