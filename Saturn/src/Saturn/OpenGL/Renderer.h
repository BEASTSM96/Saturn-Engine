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

#pragma once

#include "Saturn/Core/Base.h"
#include "Saturn/Core/Renderer/EditorCamera.h"

#include "Framebuffer.h"
#include "Common.h"

#include "Shader.h"
#include "Texture.h"

namespace Saturn {

	class Renderer
	{
		SINGLETON( Renderer );

		Renderer() { Init(); }
		~Renderer() { }

	public:

		void Clear();
		void Resize( int width, int height );

		// Render code
		void Submit( const glm::mat4& trans, Shader& shader, Texture2D& texture );

		//void Submit( const Mesh& mesh );

		void OnEvent( Event& e );

		void CompositePass();
		void GeoPass();
		void BeginCompositePass();
		void EndCompositePass();
		void BeginGeoPass();
		void EndGeoPass();

		uint32_t GetFinalColorBufferRendererID();

	public:

		EditorCamera& RendererCamera() { return m_Camera; }
		Ref<Framebuffer>& TargetFramebuffer() { return m_Framebuffer; }
		const Ref<Framebuffer>& TargetFramebuffer() const { return m_Framebuffer; }

	protected:
	private:

		Ref<Framebuffer> m_Framebuffer;
		Ref<Framebuffer> m_GeoFramebuffer;

		EditorCamera m_Camera;
		
		Ref<Shader> m_CompositeShader;

		void Init();

		const char* vertexShaderSource = "#version 330 core\n"
			"layout (location = 0) in vec3 aPos;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
			"}\0";
		const char* fragmentShaderSource = "#version 330 core\n"
			"out vec4 FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
			"}\n\0";
	};
}