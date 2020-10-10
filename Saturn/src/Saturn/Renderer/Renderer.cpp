#include "sppch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"


namespace Saturn {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	struct RendererData
	{
		//Ref<RenderPass> m_ActiveRenderPass;
		RenderCommandQueue m_CommandQueue;
		//Scope<ShaderLibrary> m_ShaderLibrary;
		Ref<VertexArray> m_FullscreenQuadVertexArray;
	};
	static RendererData s_Data;

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::Begin3DScene(SCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit3D(const Ref<DShader>& shader, const Ref<VertexArray>& vertexArray, FTransform Intransform)
	{
	}

	void Renderer::Init()
	{
		RenderCommand::Init();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	RenderCommandQueue& Renderer::GetRenderCommandQueue()
	{
		return s_Data.m_CommandQueue;
	}
}