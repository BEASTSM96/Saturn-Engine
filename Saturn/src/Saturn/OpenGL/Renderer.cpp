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

#include "xGL.h"

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
		glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
		glFrontFace( GL_CCW );

		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glBlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

		glEnable( GL_MULTISAMPLE );
		glEnable( GL_STENCIL_TEST );

		// Gamma Correction -- but the problem with this is that it gives us no control over the power gamma - 2.2 as the gamma
		//glEnable( GL_FRAMEBUFFER_SRGB );
		m_ShaderLibrary = Ref<ShaderLibrary>::Create();

		m_ShaderLibrary->Load( "assets/shaders/SceneComposite.glsl" );
		m_ShaderLibrary->Load( "assets/shaders/PBR_Static.glsl" );

		m_Camera = EditorCamera( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 10000.0f ) );

		m_CompositeShader = m_ShaderLibrary->Get( "SceneComposite" );

		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.ClearColor ={ 0.07f, 0.13f, 0.17f, 1.0f };
		m_Framebuffer = Ref<Framebuffer>::Create( compFramebufferSpec );

		// Issue a resize event so the framebuffer resize to the right size
		m_Framebuffer->Resize( Window::Get().Width(), Window::Get().Height(), true );
	}

	void Renderer::Clear()
	{
		glClearColor( pow( 0.07f, m_Gamma ), pow( 0.13f, m_Gamma ), pow( 0.17f, m_Gamma ), 1.0f );
		//glClearColor( pow( 1, m_Gamma ), pow( 1, m_Gamma ), pow( 1, m_Gamma ), 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	}

	void Renderer::Resize( int width, int height )
	{
		m_Camera.SetViewportSize( width, height );
		m_Framebuffer->Resize( width, height );
	}

	void Renderer::Submit( const glm::mat4& trans, Shader& shader, Texture2D& texture )
	{
		texture.Bind();
		shader.Bind();

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
		FlushDrawList();

		m_ActiveScene = nullptr;
	}

	void Renderer::SubmitMesh( Ref<Mesh> mesh, const glm::mat4 trans )
	{
		m_DrawList.push_back( { mesh, trans } );
	}

	void Renderer::RenderMesh( Ref<Mesh> mesh, const glm::mat4 trans )
	{
		mesh->UnbindLastTexture();
		mesh->m_VertexBuffer->Bind();
		mesh->m_Pipeline->Bind();
		mesh->m_IndexBuffer->Bind();
		
		Entity& light = m_ActiveScene->LightEntity();
		auto& lightColor = light ? light.GetComponent<LightComponent>().Color : glm::vec3( 1.0f, 1.0f, 1.0f );
		auto& lightPos = light ? light.GetComponent<TransformComponent>().Position : glm::vec3( 0.0f, 10.0f, 0.0f );

		auto viewProjection = m_Camera.ProjectionMatrix() * m_Camera.ViewMatrix();
		glm::vec3 cameraPosition = glm::inverse( m_Camera.ViewMatrix() )[ 3 ];

		for( Submesh& submesh : mesh->m_Submeshes )
		{
			//Enable( GL_DEPTH_TEST );

			if( mesh->GetMaterial()->IsFlagSet( MaterialFlag::DepthTest ) )
				glEnable( GL_DEPTH_TEST );
			else
				glDisable( GL_DEPTH_TEST );

			mesh->GetMaterial()->Bind();
			auto shader = mesh->GetMaterial()->GetShader();

			shader->BindMaterialTextures();
			shader->SetMat4( "u_ViewProjectionMatrix", viewProjection );
			shader->SetFloat3( "u_CameraPosition", cameraPosition );
			shader->SetMat4( "u_Transform", trans * submesh.Transform );

			shader->SetFloat3( "u_LightPosition", light ? lightPos : glm::vec3( 0.0f, 10.0f, 0.0f ) );
			shader->SetFloat4( "u_LightColor", light ? glm::vec4( lightColor.x, lightColor.y, lightColor.z, 1.0f ) : glm::vec4( 1, 1, 1, 1 ) );

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
		StartRenderPass( m_RenderPass );
		
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

		m_CompositeShader->Bind();
		m_CompositeShader->SetFloat( "u_Gamma", m_Gamma );

		Clear();

		auto viewProjection = m_Camera.ProjectionMatrix() * m_Camera.ViewMatrix();
		glm::vec3 cameraPosition = glm::inverse( m_Camera.ViewMatrix() )[ 3 ];
		
		for( auto& dc : m_DrawList )
		{
			RenderMesh( dc.m_Mesh, dc.m_Transform );
		}

		EndRenderPass( m_RenderPass );
	}

	void Renderer::StartRenderPass( Ref<RenderPass>& pass )
	{
		m_Framebuffer->Bind();
	}

	void Renderer::EndRenderPass( Ref<RenderPass>& pass )
	{
		m_Framebuffer->Unbind();
	}

}