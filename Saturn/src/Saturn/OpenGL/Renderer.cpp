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
	}

	void Renderer::Clear()
	{
		glClearColor( GL_CLEAR_COLOR_X_Y_Z, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	}

	void Renderer::Reszie( int width, int height )
	{
		glViewport( 0, 0, width, height );
	}

	void Renderer::Submit( const glm::mat4& trans, Shader& shader, Texture2D& texture )
	{
		shader.Bind();
		texture.Bind();

		glm::vec3 pos, rot, scale;
		Math::DecomposeTransform( trans, pos, rot, scale );

		shader.SetMat4( "u_Transform", trans );

		glDrawArrays( GL_TRIANGLES, 0, 3 );
	}

}