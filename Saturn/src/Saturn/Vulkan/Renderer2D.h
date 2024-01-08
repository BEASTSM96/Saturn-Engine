/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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
#include "VertexBuffer.h"

#include "Saturn/Scene/Scene.h"

namespace Saturn {

	struct Renderer2DDrawCommand
	{
		glm::vec3 Position;
		glm::vec4 Color;
		
		// TexCoord is calculated before we render.
		glm::vec2 TexCoord;
		float TextureIndex;
	};

	class Renderer2D : public RefTarget
	{
	public:
		static inline Renderer2D& Get() { return *SingletonStorage::GetOrCreateSingleton<Renderer2D>(); }
	public:
		Renderer2D() = default;
		~Renderer2D() = default;

		void SetInitialRenderPass( Ref<Pass> pass, Ref<Framebuffer> targetFramebuffer );
		Ref<Pass> GetTargetRenderPass() { return m_TargetRenderPass; }

		void Render();

		void SubmitQuad( const glm::mat4& transform, const glm::vec4& color );
		void SubmitQuad( const glm::vec3& position, const glm::vec4& color, const glm::vec2& size );
		void SubmitQuadTextured( const glm::mat4& transform, const glm::vec4& color, const Ref<Texture2D>& rTexture );
		
		void SubmitBillboard( const glm::vec3& position, const glm::vec4& color, const glm::vec2& rSize );
		void SubmitBillboardTextured( const glm::vec3& position, const glm::vec4& color, const Ref<Texture2D>& rTexture, const glm::vec2& rSize );

		void SetCamera( const glm::mat4& viewProjection, const glm::mat4& view );

		void Prepare();

		void Init();
		void Terminate();
		void SetViewportSize( uint32_t w, uint32_t h );

	private:
		void LateInit( Ref<Pass> targetPass = nullptr, Ref<Framebuffer> framebuffer = nullptr);
		void Recreate();
		void Reset();

		void FlushDrawList();

		void RenderAllQuads();

	private:
		Ref<Pass> m_TargetRenderPass = nullptr;
		Ref<Pass> m_TempRenderPass = nullptr;

		std::vector<Renderer2DDrawCommand> m_DrawList;
		std::vector<glm::vec4> m_QuadPositions;

		std::vector< Ref<VertexBuffer> > m_QuadVertexBuffers;
		std::vector< Renderer2DDrawCommand* > m_CurrentQuadBase;
		
		Renderer2DDrawCommand* m_CurrentQuad = nullptr;

		std::array<Ref<Texture2D>, 32> m_Textures;
		uint32_t m_DefaultTextureSlot = 1;
		uint32_t m_CurrentTextureSlot = 0;

		glm::mat4 m_CameraView = glm::mat4( 1.0f );
		glm::mat4 m_CameraViewProjection = glm::mat4( 1.0f );

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_QuadIndexCount = 0;

		bool m_Resized = false;

		VkCommandBuffer m_CommandBuffer = nullptr;

		Ref<Pipeline> m_QuadPipeline = nullptr;
		Ref<Framebuffer> m_TargetFramebuffer = nullptr;
		Ref<IndexBuffer> m_QuadIndexBuffer = nullptr;
		Ref<Shader> m_QuadShader = nullptr;
		Ref<Material> m_QuadMaterial = nullptr;
	};
}