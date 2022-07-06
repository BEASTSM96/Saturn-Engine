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

#include "sppch.h"
#include "Framebuffer.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"
#include <backends/imgui_impl_vulkan.h>

namespace Saturn {

	namespace FramebufferUtills {

		static bool IsDepthFormat( ImageFormat format )
		{
			switch( format )
			{
				case Saturn::ImageFormat::DEPTH32F:
				case Saturn::ImageFormat::DEPTH24STENCIL8:
					return true;
			}

			return false;
		}

		static VkFormat VulkanFormat( ImageFormat format )
		{
			switch( format )
			{
				case Saturn::ImageFormat::RGBA8:
					return VK_FORMAT_R8G8B8A8_UNORM;

				case Saturn::ImageFormat::RGBA16F:
					return VK_FORMAT_R16G16B16A16_UNORM;

				case Saturn::ImageFormat::RGBA32F:
					return VK_FORMAT_R32G32B32A32_SFLOAT;

				case ImageFormat::BGRA8:
					return VK_FORMAT_B8G8R8A8_UNORM;

				case Saturn::ImageFormat::DEPTH24STENCIL8:
				case Saturn::ImageFormat::DEPTH32F:
					return VK_FORMAT_D32_SFLOAT;
			}

			return VK_FORMAT_UNDEFINED;
		}
	}

	Framebuffer::Framebuffer( const FramebufferSpecification& Specification )
	{
		m_Specification = Specification;

		for( auto format : m_Specification.Attachments.Attachments )
		{
			if( FramebufferUtills::IsDepthFormat( format.TextureFormat ) )
				m_DepthFormat = format.TextureFormat;
			else
				m_ColorAttachmentsFormats.push_back( format.TextureFormat );
		}

		Create();
	}

	Framebuffer::~Framebuffer()
	{
		if( m_Framebuffer )
			vkDestroyFramebuffer( VulkanContext::Get().GetDevice(), m_Framebuffer, nullptr );

		for( auto& resource : m_ColorAttachmentsResources )
		{
			resource = nullptr;
		}
		
		m_DepthAttachmentResource = nullptr;

		m_ColorAttachmentsResources.clear();
		m_ColorAttachmentsFormats.clear();
		m_AttachmentImageViews.clear();
	}

	void Framebuffer::Recreate( uint32_t Width, uint32_t Height )
	{
		if( m_Framebuffer )
			vkDestroyFramebuffer( VulkanContext::Get().GetDevice(), m_Framebuffer, nullptr );

		m_Framebuffer = nullptr;

		for( auto& resource : m_ColorAttachmentsResources )
		{
			resource = nullptr;
		}

		m_ColorAttachmentsResources.clear();
		m_DepthAttachmentResource = nullptr;

		m_ColorAttachmentsFormats.clear();
		m_AttachmentImageViews.clear();

		////

		m_Specification.Width = Width;
		m_Specification.Height = Height;

		for( auto format : m_Specification.Attachments.Attachments )
		{
			if( FramebufferUtills::IsDepthFormat( format.TextureFormat ) )
				m_DepthFormat = format.TextureFormat;
			else
				m_ColorAttachmentsFormats.push_back( format.TextureFormat );
		}

		Create();
	}

	void Framebuffer::Create()
	{
		// Create Attachment resources
		// Color
		VkExtent3D Extent = { m_Specification.Width, m_Specification.Height, 1 };

		for( auto format : m_ColorAttachmentsFormats )
		{
			Ref<Image2D> image = Ref<Image2D>::Create( format, m_Specification.Width, m_Specification.Height, m_Specification.ArrayLevels );

			m_ColorAttachmentsResources.push_back( image );

			m_AttachmentImageViews.push_back( image->GetImageView() );
		}
		
		m_DepthAttachmentResource = Ref<Image2D>::Create( ImageFormat::Depth, m_Specification.Width, m_Specification.Height, m_Specification.ArrayLevels );
		m_AttachmentImageViews.push_back( m_DepthAttachmentResource->GetImageView() );

		VkFramebufferCreateInfo FramebufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		FramebufferCreateInfo.renderPass = m_Specification.RenderPass->GetVulkanPass();
	
		FramebufferCreateInfo.attachmentCount = m_AttachmentImageViews.size();
		FramebufferCreateInfo.pAttachments = m_AttachmentImageViews.data();

		FramebufferCreateInfo.width = m_Specification.Width;
		FramebufferCreateInfo.height = m_Specification.Height;
		FramebufferCreateInfo.layers = 1;

		VK_CHECK( vkCreateFramebuffer( VulkanContext::Get().GetDevice(), &FramebufferCreateInfo, nullptr, &m_Framebuffer ) );
	
	}
}