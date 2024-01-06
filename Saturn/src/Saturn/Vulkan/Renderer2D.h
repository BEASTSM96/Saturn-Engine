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

#pragma once

#include "Saturn/Core/Ref.h"

#include "Pass.h"
#include "Texture.h"
#include "Framebuffer.h"

#include "Saturn/Scene/Scene.h"

namespace Saturn {

	struct Renderer2DDrawCommand
	{
		glm::vec3 Position;
		glm::vec4 Color;
		
		// TexCoord is calculated before we render.
		glm::vec2 TexCoord;
		Ref<Texture2D> Texture = nullptr;
	};

	class Renderer2D : public RefTarget
	{
	public:
		Renderer2D() { Init(); }
		~Renderer2D() { Terminate(); }

		void SetInitialRenderPass( Ref<Pass> pass );
		Ref<Pass> GetTargetRenderPass() { return m_TargetRenderPass; }

		void Render( const glm::mat4& viewProjection, const glm::mat4& view );

		void SubmitQuad( const glm::mat4& transform, const glm::vec4& color );
		void SubmitQuadTextured( const glm::mat4& transform, const glm::vec4& color, const Ref<Texture2D>& rTexture );
		
		void SubmitBillboard( const glm::vec3& position, const glm::vec4& color, const glm::vec2& rSize );
		void SubmitBillboardTextured( const glm::vec3& position, const glm::vec4& color, const Ref<Texture2D>& rTexture, const glm::vec2& rSize );

	private:
		void Init();
		void LateInit( Ref<Pass> targetPass = nullptr );

		void Terminate();
		void FlushDrawList();

		void RenderAllQuads();

	private:
		Ref<Pass> m_TargetRenderPass = nullptr;
		Ref<Pass> m_TempRenderPass = nullptr;

		std::vector<Renderer2DDrawCommand> m_DrawList;
		std::vector<glm::vec4> m_QuadPositions;

		glm::mat4 m_CameraView;
		glm::mat4 m_CameraViewProjection;

		uint32_t m_Width, m_Height;

		VkCommandBuffer m_CommandBuffer = nullptr;

		Ref<Pipeline> m_QuadPipeline = nullptr;
		Ref<Framebuffer> m_QuadFramebuffer = nullptr;
		Ref<IndexBuffer> m_QuadIndexBuffer = nullptr;
	};
}