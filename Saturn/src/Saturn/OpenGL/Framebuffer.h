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
#include "Common.h"

#include <glm/glm.hpp>
#include <stdint.h>
#include <vector>

namespace Saturn {

	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8   = 1,
		RGBA16F = 2,
		RGBA32F = 3,
		RGB32F  = 4,

		DEPTH32F = 5,
		DEPTH24STENCIL8 = 6,

		Depth = DEPTH24STENCIL8
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 1280;
		uint32_t Height = 720;

		glm::vec4 ClearColor;

		uint32_t Samples = 1;

		bool NoResize = false;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		Framebuffer( const FramebufferSpecification& spec );
		Framebuffer() { }
		~Framebuffer();

		void Bind();
		void Unbind();

		void Resize( uint32_t width, uint32_t height, bool forceRecreate = false );

		void BindTexture( uint32_t index = 0, uint32_t slot = 0 );

		uint32_t Width()  { return m_Specification.Width; }
		uint32_t Height() { return m_Specification.Height; }

		RendererID GetRendererID() const { return m_RendererID; }

		RendererID ColorAttachmentRendererID( int index = 0 );
		RendererID DepthAttachmentRendererID( void ) const;

		virtual const FramebufferSpecification& Specification( void ) const { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;

		RendererID m_RendererID = 0;

		RendererID m_TextureID = 0;
		RendererID m_DepthTextureID = 0;

		uint32_t m_Width = 0, m_Height = 0;
	};
}