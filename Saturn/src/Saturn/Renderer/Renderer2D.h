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

#include <glm/glm.hpp>

#include "Saturn/Renderer/Texture.h"

namespace Saturn {
	class Renderer2D
	{
	public:
		static void Init( void );
		static void Shutdown( void );

		static void BeginScene( const glm::mat4& viewProj, bool depthTest = true );
		static void EndScene( void );
		static void Flush( void );

		// Primitives
		static void DrawQuad( const glm::mat4& transform, const glm::vec4& color );
		static void DrawQuad( const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4( 1.0f ) );

		static void DrawQuad( const glm::vec2& position, const glm::vec2& size, const glm::vec4& color );
		static void DrawQuad( const glm::vec3& position, const glm::vec2& size, const glm::vec4& color );
		static void DrawQuad( const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4( 1.0f ) );
		static void DrawQuad( const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4( 1.0f ) );

		static void DrawRotatedQuad( const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color );
		static void DrawRotatedQuad( const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color );
		static void DrawRotatedQuad( const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4( 1.0f ) );
		static void DrawRotatedQuad( const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4( 1.0f ) );

		static void DrawLine( const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4( 1.0f ) );
		// Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t LineCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4 + LineCount * 2; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6 + LineCount * 2; }
		};
		static void ResetStats( void );
		static Statistics GetStats( void );
	private:
		static void FlushAndReset( void );
		static void FlushAndResetLines( void );
	};
}