/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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
#include "Renderer.h"

#include "Saturn/Core/Window.h"

#include "Saturn/Core/App.h"
#include "Saturn/Scene/Scene.h"

#include "Saturn/Core/Math.h"

#include <glad/glad.h>

namespace Saturn {

	static void OpenGLLogMessage( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam )
	{
		switch( severity )
		{
			case GL_DEBUG_SEVERITY_HIGH:
				SAT_CORE_ERROR( "[OpenGL Debug HIGH] {0}", message );
				SAT_CORE_ASSERT( false, "GL_DEBUG_SEVERITY_HIGH" );
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				SAT_CORE_WARN( "[OpenGL Debug MEDIUM] {0}", message );
				break;
			case GL_DEBUG_SEVERITY_LOW:
				SAT_CORE_INFO( "[OpenGL Debug LOW] {0}", message );
				break;
		}
	}

	void Renderer::Init()
	{
		// Enable Debug logging
		glDebugMessageCallback( OpenGLLogMessage, nullptr );
		glEnable( GL_DEBUG_OUTPUT );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );

		// Gen empty vertex array
		unsigned int vao;
		glGenVertexArrays( 1, &vao );
		glBindVertexArray( vao );

		glEnable( GL_DEPTH_TEST );
		//glEnable( GL_CULL_FACE );
		glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
		glFrontFace( GL_CCW );

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

		glEnable( GL_MULTISAMPLE );
		glEnable( GL_STENCIL_TEST );

		m_ShaderLibrary = Ref<ShaderLibrary>::Create();

		m_ShaderLibrary->Load( "assets/shaders/SceneComposite.glsl" );
		m_ShaderLibrary->Load( "assets/shaders/PBR_Static.glsl" );

		m_Camera = EditorCamera( 45.0f, 1280.0f, 720.0f, 0.1f );

		m_CompositeShader = m_ShaderLibrary->Get( "SceneComposite" );

		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.ClearColor ={ 0.1f, 0.1f, 0.1f, 1.0f };

		RenderPassSpecification renderPassSpec;
		//renderPassSpec.TargetFramebuffer = Ref<Framebuffer>::Create( compFramebufferSpec );

		m_RenderPass = Ref<RenderPass>::Create( renderPassSpec );

		m_Framebuffer = Ref<Framebuffer>::Create( compFramebufferSpec );

		// Issue a resize event so the framebuffer resize to the right size
		m_Framebuffer->Resize( Window::Get().Width(), Window::Get().Height(), true );
	}

	void Renderer::Clear()
	{
		glClearColor( GL_CLEAR_COLOR_X_Y_Z, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	}

	void Renderer::Resize( int width, int height )
	{
		m_Camera.SetViewportSize( width, height );
		//m_GeoFramebuffer->Resize( width, height );
		m_Framebuffer->Resize( width, height );
	}

	void Renderer::Submit( const glm::mat4& trans, Shader& shader, Texture2D& texture )
	{
		shader.Bind();
		texture.Bind();

		glm::vec3 pos, rot, scale;
		Math::DecomposeTransform( trans, pos, rot, scale );

		shader.SetMat4( "u_Transform", trans );

		glDrawArrays( GL_TRIANGLES, 0, 6 );
	}

	void Renderer::OnEvent( Event& e )
	{
		m_Camera.OnEvent( e );
	}

	void Renderer::Enable( int cap )
	{
		glEnable( cap );
	}

	void Renderer::Disable( int cap )
	{
		glDisable( cap );
	}

	void Renderer::BeginScene( Ref<Scene> scene )
	{
		SAT_CORE_ASSERT( scene, " Scene is null " );

		m_ActiveScene = scene;
	}

	void Renderer::EndScene()
	{
		m_ActiveScene = nullptr;

		FlushDrawList();
	}

	void Renderer::SubmitMesh( Ref<Mesh> mesh, const glm::mat4 trans )
	{
		m_DrawList.push_back( { mesh, trans } );
	}

	void Renderer::RenderMesh( Ref<Mesh> mesh, const glm::mat4 trans, Ref<Shader>& shader )
	{
		mesh->m_VertexBuffer->Bind();
		mesh->m_Pipeline->Bind();
		mesh->m_IndexBuffer->Bind();

		auto viewProjection = m_Camera.ProjectionMatrix() * m_Camera.ViewMatrix();
		glm::vec3 cameraPosition = glm::inverse( m_Camera.ViewMatrix() )[ 3 ];

		SAT_CORE_INFO( "Camera Pos X Y Z {0} {1} {2}", cameraPosition.x, cameraPosition.y, cameraPosition.z );

		for( Submesh& submesh : mesh->m_Submeshes )
		{
			Enable( GL_DEPTH_TEST );

			shader->Bind();

			shader->SetMat4( "u_ViewProjectionMatrix", viewProjection );
			shader->SetFloat3( "u_CameraPosition", cameraPosition );
			shader->SetFloat3( "u_ViewPos", cameraPosition );

			shader->SetMat4( "u_Transform", trans * submesh.Transform );

			glDrawElementsBaseVertex( GL_TRIANGLES, submesh.IndexCount, GL_UNSIGNED_INT, ( void* )( sizeof( uint32_t ) * submesh.BaseIndex ), submesh.BaseVertex );
		}
	}

	void Renderer::FlushDrawList()
	{
		GeoPass();
		CompositePass();

		m_DrawList.clear();
	}

	uint32_t Renderer::GetFinalColorBufferRendererID()
	{
		return m_Framebuffer->ColorAttachmentRendererID();
	}

	void Renderer::CompositePass()
	{
	}

	void Renderer::GeoPass()
	{
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

		StartRenderPass( m_RenderPass );

		Clear();

		auto viewProjection = m_Camera.ProjectionMatrix() * m_Camera.ViewMatrix();
		glm::vec3 cameraPosition = glm::inverse( m_Camera.ViewMatrix() )[ 3 ];
		
		for( auto& dc : m_DrawList )
		{
			Ref<Shader>& shader = dc.m_Mesh->GetShader();

			RenderMesh( dc.m_Mesh, dc.m_Transform, shader );
		}

		EndRenderPass( m_RenderPass );
	}

	void Renderer::StartRenderPass( Ref<RenderPass>& pass )
	{
		//pass->TargetFramebuffer()->Bind();

		m_Framebuffer->Bind();
	}

	void Renderer::EndRenderPass( Ref<RenderPass>& pass )
	{
		//pass->TargetFramebuffer()->Unbind();

		m_Framebuffer->Unbind();
	}

}