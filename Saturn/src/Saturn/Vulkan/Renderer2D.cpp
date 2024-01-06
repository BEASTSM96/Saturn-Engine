/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include "sppch.h"
#include "Renderer2D.h"

#include "Renderer.h"
#include "VulkanDebug.h"

#include <Ruby/RubyWindow.h>

namespace Saturn {

	void Renderer2D::Init()
	{
		m_Width = Application::Get().GetWindow()->GetWidth();
		m_Height = Application::Get().GetWindow()->GetHeight();

		// Setup Quads
		m_QuadPositions[ 0 ] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_QuadPositions[ 1 ] = { -0.5f,  0.5f, 0.0f, 1.0f };
		m_QuadPositions[ 2 ] = { 0.5f,  0.5f, 0.0f, 1.0f };
		m_QuadPositions[ 3 ] = { 0.5f, -0.5f, 0.0f, 1.0f };

		uint32_t indices[ 6 ] = { 0, 1, 2, 2, 3, 0, };
		m_QuadIndexBuffer = Ref<IndexBuffer>::Create( indices, 6 * sizeof( uint32_t ) );

		// Construct a temporary render pass this is be changed when the scene renderer is ready.
		PassSpecification PassSpec;
		PassSpec.Name = "Renderer2D-TemporaryRP";
		PassSpec.Attachments = { ImageFormat::RGB32F, ImageFormat::Depth };

		m_TempRenderPass = Ref<Pass>::Create( PassSpec );
		m_TargetRenderPass = m_TempRenderPass;

		LateInit( nullptr );
	}

	void Renderer2D::LateInit( Ref<Pass> targetPass /*= nullptr */ )
	{
		FramebufferSpecification FBSpec = {};
		FBSpec.Width = m_Width;
		FBSpec.Height = m_Height;
		FBSpec.RenderPass = targetPass == nullptr ? m_TempRenderPass : targetPass;
		FBSpec.Attachments = { ImageFormat::RGB32F, ImageFormat::Depth };

		m_QuadFramebuffer = Ref<Framebuffer>::Create( FBSpec );

		if( !m_QuadShader )
		{
			m_QuadShader = ShaderLibrary::Get().TryFind( "Billboard", "content/shaders/Billboard.glsl" );
			m_QuadDS = m_QuadShader->CreateDescriptorSet( 0 );
		}

		PipelineSpecification PipelineSpec{};
		PipelineSpec.Width = m_Width;
		PipelineSpec.Height = m_Height;
		PipelineSpec.Name = "Renderer2D(Quads)";
		PipelineSpec.Shader = m_QuadShader;
		PipelineSpec.RenderPass = targetPass == nullptr ? m_TempRenderPass : targetPass;
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		m_QuadPipeline = Ref<Pipeline>::Create( PipelineSpec );
	}

	void Renderer2D::Terminate()
	{
		if( m_TempRenderPass )
			m_TempRenderPass = nullptr;

		m_QuadFramebuffer = nullptr;
		m_QuadPipeline = nullptr;
		m_TargetRenderPass = nullptr;
		m_QuadIndexBuffer = nullptr;
	}

	void Renderer2D::SetInitialRenderPass( Ref<Pass> pass )
	{
		if( m_TargetRenderPass != pass )
		{
			m_TargetRenderPass = pass;

			m_TempRenderPass = nullptr;

			m_QuadFramebuffer = nullptr;
			m_QuadPipeline = nullptr;

			LateInit( m_TargetRenderPass );
		}
	}

	void Renderer2D::Render( const glm::mat4& viewProjection, const glm::mat4& view )
	{
		m_CommandBuffer = Renderer::Get().ActiveCommandBuffer();

		m_CameraView = view;
		m_CameraViewProjection = viewProjection;

		// First, check if we have a render pass.
		if( !m_TargetRenderPass ) 
		{
			FlushDrawList();
			return;
		}

		// Start by rendering all quads
		CmdBeginDebugLabel( m_CommandBuffer, "Renderer2D (Quads)" );

		RenderAllQuads();

		CmdEndDebugLabel( m_CommandBuffer );

		FlushDrawList();
	}

	void Renderer2D::RenderAllQuads()
	{
		VkExtent2D Extent = { m_Width, m_Height };

		m_TargetRenderPass->BeginPass( m_CommandBuffer, m_QuadFramebuffer->GetVulkanFramebuffer(), Extent );

		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = 0;
		Viewport.width = (float) m_Width;
		Viewport.height = (float) m_Height;
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;

		VkRect2D Scissor = { .offset = { 0,0 }, .extent = Extent };

		vkCmdSetScissor( m_CommandBuffer, 0, 1, &Scissor );
		vkCmdSetViewport( m_CommandBuffer, 0, 1, &Viewport );

		struct QuadMatricesObject
		{
			glm::mat4 ViewProjection = glm::mat4( 1.0f );
		} u_Matrices;

		u_Matrices.ViewProjection = m_CameraViewProjection;

		m_QuadShader->UploadUB( ShaderType::Vertex, 0, 0, &u_Matrices, sizeof( u_Matrices ) );
		m_QuadShader->WriteDescriptor( "u_InputTexture", Renderer::Get().GetPinkTexture()->GetDescriptorInfo(), m_QuadDS->GetVulkanSet() );

		m_QuadShader->WriteAllUBs( m_QuadDS );

		for( const auto& rCommand : m_DrawList )
		{
			m_QuadPipeline->Bind( m_CommandBuffer );
			m_QuadDS->Bind( m_CommandBuffer, m_QuadPipeline->GetPipelineLayout() );

			m_QuadIndexBuffer->Bind( m_CommandBuffer );

			glm::mat4 transform = glm::mat4(1.0f);
			vkCmdPushConstants( m_CommandBuffer, m_QuadPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( glm::mat4 ), &transform );

			m_QuadIndexBuffer->Draw( m_CommandBuffer );
		}

		m_TargetRenderPass->EndPass();
	}

	void Renderer2D::FlushDrawList()
	{
		m_DrawList.clear();
	}

	void Renderer2D::SubmitQuad( const glm::mat4& transform, const glm::vec4& color )
	{
		// One quad has 4 vertexes so we need to submit them one by one.
		glm::vec2 TexCoord[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		for( size_t i = 0; i < 4; i++ )
		{
			Renderer2DDrawCommand command{};
			command.Color = color;
			command.Position = transform * m_QuadPositions[ i ];
			command.TexCoord = TexCoord[ i ];

			m_DrawList.push_back( command );
		}
	}

	void Renderer2D::SubmitQuadTextured( const glm::mat4& transform, const glm::vec4& color, const Ref<Texture2D>& rTexture )
	{
		// One quad has 4 vertexes so we need to submit them one by one.
		glm::vec2 TexCoord[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		for( size_t i = 0; i < 4; i++ )
		{
			Renderer2DDrawCommand command{};
			command.Color = color;
			command.Position = transform * m_QuadPositions[ i ];
			command.TexCoord = TexCoord[ i ];
			command.Texture = rTexture;

			m_DrawList.push_back( command );
		}
	}

	void Renderer2D::SubmitBillboard( const glm::vec3& position, const glm::vec4& color, const glm::vec2& rSize )
	{
		glm::vec2 TexCoord[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		glm::vec3 CamRight = { m_CameraView[ 0 ][ 0 ], m_CameraView[ 1 ][ 0 ], m_CameraView[ 2 ][ 0 ] };
		glm::vec3 CamUp = { m_CameraView[ 0 ][ 1 ], m_CameraView[ 1 ][ 1 ], m_CameraView[ 2 ][ 1 ] };

		Renderer2DDrawCommand command{};
		command.Position = position + CamRight * ( m_QuadPositions[ 0 ].x ) * rSize.x + CamUp * m_QuadPositions[ 0 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 0 ];

		m_DrawList.push_back( command );

		command.Position = position + CamRight * ( m_QuadPositions[ 1 ].x ) * rSize.x + CamUp * m_QuadPositions[ 1 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 1 ];

		m_DrawList.push_back( command );

		command.Position = position + CamRight * ( m_QuadPositions[ 2 ].x ) * rSize.x + CamUp * m_QuadPositions[ 2 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 2 ];

		m_DrawList.push_back( command );

		command.Position = position + CamRight * ( m_QuadPositions[ 3 ].x ) * rSize.x + CamUp * m_QuadPositions[ 3 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 3 ];

		m_DrawList.push_back( command );
	}

	void Renderer2D::SubmitBillboardTextured( const glm::vec3& position, const glm::vec4& color, const Ref<Texture2D>& rTexture, const glm::vec2& rSize )
	{
		glm::vec2 TexCoord[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		glm::vec3 CamRight = { m_CameraView[ 0 ][ 0 ], m_CameraView[ 1 ][ 0 ], m_CameraView[ 2 ][ 0 ] };
		glm::vec3 CamUp = { m_CameraView[ 0 ][ 1 ], m_CameraView[ 1 ][ 1 ], m_CameraView[ 2 ][ 1 ] };

		Renderer2DDrawCommand command{};
		command.Texture = rTexture;

		command.Position = position + CamRight * ( m_QuadPositions[ 0 ].x ) * rSize.x + CamUp * m_QuadPositions[ 0 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 0 ];

		m_DrawList.push_back( command );

		command.Position = position + CamRight * ( m_QuadPositions[ 1 ].x ) * rSize.x + CamUp * m_QuadPositions[ 1 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 1 ];

		m_DrawList.push_back( command );

		command.Position = position + CamRight * ( m_QuadPositions[ 2 ].x ) * rSize.x + CamUp * m_QuadPositions[ 2 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 2 ];

		m_DrawList.push_back( command );

		command.Position = position + CamRight * ( m_QuadPositions[ 3 ].x ) * rSize.x + CamUp * m_QuadPositions[ 3 ].y * rSize.y;
		command.Color = color;
		command.TexCoord = TexCoord[ 3 ];

		m_DrawList.push_back( command );
	}

}