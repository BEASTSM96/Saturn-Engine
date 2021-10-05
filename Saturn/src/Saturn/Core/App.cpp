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
#include "App.h"

#include "Window.h"

#include "Saturn/OpenGL/Shader.h"
#include "Saturn/OpenGL/Renderer.h"
#include "Saturn/OpenGL/Texture.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

namespace Saturn {

	void Application::Run()
	{
		Window::Get();
		Renderer::Get();

		Shader shader( "assets\\shaders\\ShaderBasic.glsl" );

		float vertices[] ={
			// positions          // colors           // texture coords
			 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
			 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
		};

		unsigned int indices[] ={
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
		};

		unsigned int VBO, VAO, EBO;
		glGenVertexArrays( 1, &VAO );
		glGenBuffers( 1, &VBO );
		glGenBuffers( 1, &EBO );

		glBindVertexArray( VAO );

		glBindBuffer( GL_ARRAY_BUFFER, VBO );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

		// position attribute
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* )0 );
		glEnableVertexAttribArray( 0 );
		// color attribute
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* )( 3 * sizeof( float ) ) );
		glEnableVertexAttribArray( 1 );
		// texture coord attribute
		glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* )( 6 * sizeof( float ) ) );
		glEnableVertexAttribArray( 2 );

		Texture2D text2d( "assets\\meshes\\m1911\\m1911_color.png", true );
		text2d.Bind();

		shader.Bind();
		shader.SetInt( "texture2", 1 );

		while( m_Running )
		{
			Window::Get().Render();

			Window::Get().OnUpdate();

			Renderer::Get().Clear();

			glm::mat4 trans = glm::mat4( 1.0f );
			trans = glm::translate( trans, glm::vec3( 0.5f, -0.5f, 0.0f ) );
			trans = glm::rotate( trans, ( float )glfwGetTime(), glm::vec3( 0.0f, 0.0f, 1.0f ) );

			unsigned int transformLoc = glGetUniformLocation( shader.GetRendererID(), "u_Transform" );
			glUniformMatrix4fv( transformLoc, 1, GL_FALSE, glm::value_ptr( trans ) );

			glBindVertexArray( VAO );
			Renderer::Get().Submit( trans, shader, text2d );
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}

}
