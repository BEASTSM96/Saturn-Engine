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
#include "Image2D.h"

#include <vulkan.h>

namespace Saturn {

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification( ImageFormat format ) : TextureFormat( format ) { }

		ImageFormat TextureFormat;
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
		uint32_t ArrayLevels = 1;

		Ref< Pass > RenderPass = nullptr;
		FramebufferAttachmentSpecification Attachments;
	};

	class Framebuffer : public CountedObj
	{
	public:
		Framebuffer( const FramebufferSpecification& Specification );
		~Framebuffer();
		
		void Recreate( uint32_t Width, uint32_t  Height );

		operator VkFramebuffer() const { return m_Framebuffer; }

		VkFramebuffer GetVulkanFramebuffer() { return m_Framebuffer; }

		std::vector< Ref<Image2D> >& GetColorAttachmentsResources() { return m_ColorAttachmentsResources; }
		const std::vector< Ref<Image2D> >& GetColorAttachmentsResources() const { return m_ColorAttachmentsResources; }

		std::vector< ImageFormat >& GetColorAttachmentsFormats() { return m_ColorAttachmentsFormats; }
		const std::vector< ImageFormat >& GetColorAttachmentsFormats() const { return m_ColorAttachmentsFormats; }

		Ref<Image2D>& GetDepthAttachmentsResource() { return m_DepthAttachmentResource; }
		const Ref<Image2D>& GetDepthAttachmentsResource() const { return m_DepthAttachmentResource; }

	private:
		void Create();

		VkFramebuffer m_Framebuffer = nullptr;

		std::vector< ImageFormat > m_ColorAttachmentsFormats;
		std::vector< Ref<Image2D> > m_ColorAttachmentsResources;

		ImageFormat m_DepthFormat;
		Ref<Image2D> m_DepthAttachmentResource;

		std::vector< VkImageView > m_AttachmentImageViews;

		FramebufferSpecification m_Specification;
	};
}