#include "sppch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"


namespace Saturn {


	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

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
}