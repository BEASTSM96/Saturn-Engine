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

#include "Saturn/Core/App.h"

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

		m_Camera = EditorCamera( 45.0f, 1280.0f, 720.0f, 0.1f );

		m_CompositeShader = Ref<Shader>::Create( "assets/shaders/SceneComposite.glsl" );

		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.Attachments ={ FramebufferTextureFormat::RGBA8 };
		compFramebufferSpec.ClearColor ={ 0.1f, 0.1f, 0.1f, 1.0f };

		m_Framebuffer = Ref<Framebuffer>::Create( compFramebufferSpec );

		FramebufferSpecification geoFramebufferSpec;
		geoFramebufferSpec.Attachments ={ FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::RGBA16F, FramebufferTextureFormat::Depth };
		geoFramebufferSpec.Samples = 8;
		geoFramebufferSpec.ClearColor ={ 0.1f, 0.1f, 0.1f, 1.0f };

		m_GeoFramebuffer = Ref<Framebuffer>::Create( geoFramebufferSpec );
	}

	void Renderer::Clear()
	{
		glClearColor( GL_CLEAR_COLOR_X_Y_Z, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	}

	void Renderer::Resize( int width, int height )
	{
		m_Camera.SetViewportSize( width, height );
		m_GeoFramebuffer->Resize( width, height );
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

	void Renderer::BeginCompositePass()
	{
		m_Framebuffer->Bind();
		Clear();
	}

	void Renderer::EndCompositePass()
	{
		m_Framebuffer->Unbind();
	}

	void Renderer::BeginGeoPass()
	{
		m_GeoFramebuffer->Bind();
	}

	void Renderer::EndGeoPass()
	{
		m_GeoFramebuffer->Unbind();
	}

	uint32_t Renderer::GetFinalColorBufferRendererID()
	{
		return m_Framebuffer->ColorAttachmentRendererID();
	}

	void Renderer::CompositePass()
	{
		BeginCompositePass();

		m_CompositeShader->Bind();
		m_CompositeShader->SetFloat( "u_Exposure", 0.800000012 );
		m_CompositeShader->SetInt( "u_TextureSamples", m_GeoFramebuffer->Specification().Samples );

		m_GeoFramebuffer->BindTexture();

		EndCompositePass();
	}

	void Renderer::GeoPass()
	{
		BeginGeoPass();

		unsigned int vertexShader = glCreateShader( GL_VERTEX_SHADER );
		glShaderSource( vertexShader, 1, &vertexShaderSource, NULL );
		glCompileShader( vertexShader );
		// check for shader compile errors
		int success;
		char infoLog[ 512 ];
		glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &success );
		if( !success )
		{
			glGetShaderInfoLog( vertexShader, 512, NULL, infoLog );
		}
		// fragment shader
		unsigned int fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
		glShaderSource( fragmentShader, 1, &fragmentShaderSource, NULL );
		glCompileShader( fragmentShader );
		// check for shader compile errors
		glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &success );
		if( !success )
		{
			glGetShaderInfoLog( fragmentShader, 512, NULL, infoLog );
		}
		// link shaders
		unsigned int shaderProgram = glCreateProgram();
		glAttachShader( shaderProgram, vertexShader );
		glAttachShader( shaderProgram, fragmentShader );
		glLinkProgram( shaderProgram );
		// check for linking errors
		glGetProgramiv( shaderProgram, GL_LINK_STATUS, &success );
		if( !success )
		{
			glGetProgramInfoLog( shaderProgram, 512, NULL, infoLog );
		}
		glDeleteShader( vertexShader );
		glDeleteShader( fragmentShader );

		// set up vertex data (and buffer(s)) and configure vertex attributes
		// ------------------------------------------------------------------
		float vertices[] ={
			 0.5f,  0.5f, 0.0f,  // top right
			 0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left 
		};
		unsigned int indices[] ={  // note that we start from 0!
			0, 1, 3,  // first Triangle
			1, 2, 3   // second Triangle
		};
		unsigned int VBO, VAO, EBO;
		glGenVertexArrays( 1, &VAO );
		glGenBuffers( 1, &VBO );
		glGenBuffers( 1, &EBO );
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray( VAO );

		glBindBuffer( GL_ARRAY_BUFFER, VBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), ( void* )0 );
		glEnableVertexAttribArray( 0 );

		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer( GL_ARRAY_BUFFER, 0 );

		// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		glBindVertexArray( 0 );

		glUseProgram( shaderProgram );
		glBindVertexArray( VAO ); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
		// glBindVertexArray(0); // no need to unbind it every time 

		EndGeoPass();
	}
}