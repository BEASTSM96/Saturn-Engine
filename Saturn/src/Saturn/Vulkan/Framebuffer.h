/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
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

#include "Pass.h"
#include "Renderer.h"

#include <vulkan.h>

namespace Saturn {

	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8 = 1,
		RGBA16F = 2,
		RGBA32F = 3,
		RGB32F = 4,

		BGRA8 = 5,

		DEPTH32F = 6,
		DEPTH24STENCIL8 = 7,

		Depth = DEPTH32F
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification( FramebufferTextureFormat format ) : TextureFormat( format ) { }

		FramebufferTextureFormat TextureFormat;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification( const std::initializer_list<FramebufferTextureSpecification>&attachments ) : Attachments( attachments ) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width;
		uint32_t Height;

		Ref< Pass > RenderPass = nullptr;
		FramebufferAttachmentSpecification Attachments;
	};

	struct FramebufferAttachmentResource
	{
		VkImage Image =	nullptr;
		VkImageView ImageView = nullptr;
		VkSampler Sampler = nullptr;
		VkDeviceMemory Memory = nullptr;
		VkDescriptorSet DescriptorSet = nullptr;
	};

	class Framebuffer
	{
	public:
		Framebuffer( const FramebufferSpecification& Specification );
		~Framebuffer();
		
		void Recreate();

		void CreateDescriptorSets();

		operator VkFramebuffer() const { return m_Framebuffer; }

		VkFramebuffer GetVulkanFramebuffer() { return m_Framebuffer; }

		std::vector< FramebufferAttachmentResource >& GetColorAttachmentsResources() { return m_ColorAttachmentsResources; }
		const std::vector< FramebufferAttachmentResource >& GetColorAttachmentsResources() const { return m_ColorAttachmentsResources; }

		std::vector< FramebufferTextureFormat >& GetColorAttachmentsFormats() { return m_ColorAttachmentsFormats; }
		const std::vector< FramebufferTextureFormat >& GetColorAttachmentsFormats() const { return m_ColorAttachmentsFormats; }

	private:
		void Create();

		VkFramebuffer m_Framebuffer = nullptr;

		std::vector< VkDescriptorSet > m_FramebufferColorResults;
		std::vector< FramebufferTextureFormat > m_ColorAttachmentsFormats;
		std::vector< FramebufferAttachmentResource > m_ColorAttachmentsResources;

		FramebufferTextureFormat m_DepthFormat;
		FramebufferAttachmentResource m_DepthAttachmentResource;

		std::vector< VkImageView > m_AttachmentImageViews;

		FramebufferSpecification m_Specification;
	};
}