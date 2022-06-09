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

		static bool IsDepthFormat( FramebufferTextureFormat format )
		{
			switch( format )
			{
				case Saturn::FramebufferTextureFormat::DEPTH32F:
				case Saturn::FramebufferTextureFormat::DEPTH24STENCIL8:
					return true;
			}

			return false;
		}

		static VkFormat VulkanFormat( FramebufferTextureFormat format ) 
		{
			switch( format )
			{
				case Saturn::FramebufferTextureFormat::RGBA8:
					return VK_FORMAT_R8G8B8A8_UNORM;

				case Saturn::FramebufferTextureFormat::RGBA16F:
					return VK_FORMAT_R16G16B16A16_UNORM;

				case Saturn::FramebufferTextureFormat::RGBA32F:
					return VK_FORMAT_R32G32B32A32_SFLOAT;

				case FramebufferTextureFormat::BGRA8:
					return VK_FORMAT_B8G8R8A8_UNORM;

				case Saturn::FramebufferTextureFormat::DEPTH24STENCIL8:
				case Saturn::FramebufferTextureFormat::DEPTH32F:
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
			vkDestroyImage( VulkanContext::Get().GetDevice(), resource.Image, nullptr );
			vkDestroyImageView( VulkanContext::Get().GetDevice(), resource.ImageView, nullptr );
			vkDestroySampler( VulkanContext::Get().GetDevice(), resource.Sampler, nullptr );
			vkFreeMemory( VulkanContext::Get().GetDevice(), resource.Memory, nullptr );
		}

		vkDestroyImage( VulkanContext::Get().GetDevice(), m_DepthAttachmentResource.Image, nullptr );
		vkDestroyImageView( VulkanContext::Get().GetDevice(), m_DepthAttachmentResource.ImageView, nullptr );
		vkDestroySampler( VulkanContext::Get().GetDevice(), m_DepthAttachmentResource.Sampler, nullptr );
		vkFreeMemory( VulkanContext::Get().GetDevice(), m_DepthAttachmentResource.Memory, nullptr );

		m_ColorAttachmentsResources.clear();
		m_ColorAttachmentsFormats.clear();
		m_AttachmentImageViews.clear();
	}

	void Framebuffer::Recreate()
	{
		if( m_Framebuffer )
			vkDestroyFramebuffer( VulkanContext::Get().GetDevice(), m_Framebuffer, nullptr );

		Create();
	}

	void Framebuffer::CreateDescriptorSets()
	{
		// This is all for ImGui.
		// TEMP
		// WE SHOULD NEVER USE IMGUI TO CREATE THE TEXTURES, but because this is really for debug, as in real runtime, we'll never use imgui.
		for( auto& resource : m_ColorAttachmentsResources ) 
		{
			resource.DescriptorSet = ( VkDescriptorSet )ImGui_ImplVulkan_AddTexture( resource.Sampler, resource.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
		}

		m_DepthAttachmentResource.DescriptorSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_DepthAttachmentResource.Sampler, m_DepthAttachmentResource.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
	}

	void Framebuffer::Create()
	{
		// Create Attachment resources
		// Color
		VkExtent3D Extent = { m_Specification.Width, m_Specification.Height, 1 };
		for( auto format : m_ColorAttachmentsFormats )
		{
			FramebufferAttachmentResource res = {};
			
			// Create image.
			Renderer::Get().CreateImage( VK_IMAGE_TYPE_2D, FramebufferUtills::VulkanFormat( format ), Extent, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &res.Image, &res.Memory );

			// Create image view.
			Renderer::Get().CreateImageView( res.Image, FramebufferUtills::VulkanFormat( format ), VK_IMAGE_ASPECT_COLOR_BIT, &res.ImageView );

			// Create sampler.
			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &res.Sampler );

			m_ColorAttachmentsResources.push_back( res );
			m_AttachmentImageViews.push_back( res.ImageView );
		}

		// Create depth resources
		FramebufferAttachmentResource depth = {};

		Renderer::Get().CreateImage( VK_IMAGE_TYPE_2D, FramebufferUtills::VulkanFormat( m_DepthFormat ), Extent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &depth.Image, &depth.Memory );

		// Create image view.
		Renderer::Get().CreateImageView( depth.Image, FramebufferUtills::VulkanFormat( m_DepthFormat ), VK_IMAGE_ASPECT_DEPTH_BIT, &depth.ImageView );

		// Create sampler.
		Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &depth.Sampler );

		m_DepthAttachmentResource = depth;
		m_AttachmentImageViews.push_back( depth.ImageView );

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