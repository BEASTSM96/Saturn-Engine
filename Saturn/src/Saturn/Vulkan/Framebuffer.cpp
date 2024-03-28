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

				case ImageFormat::RED8:
					return VK_FORMAT_R8_UNORM;

				case Saturn::ImageFormat::DEPTH24STENCIL8:
					return VK_FORMAT_D32_SFLOAT_S8_UINT;
				case Saturn::ImageFormat::DEPTH32F:
					return VK_FORMAT_D32_SFLOAT;
			}

			return VK_FORMAT_UNDEFINED;
		}
	}

	Framebuffer::Framebuffer( const FramebufferSpecification& Specification )
		: m_Specification( Specification )
	{

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
		Terminate();
	}

	void Framebuffer::Terminate()
	{
		if( m_Framebuffer )
			vkDestroyFramebuffer( VulkanContext::Get().GetDevice(), m_Framebuffer, nullptr );

		for( auto& resource : m_ColorAttachmentsResources )
		{
			resource = nullptr;
		}

		for( auto& [index, resource] : m_Specification.ExistingImages )
		{
			resource = nullptr;
		}

		m_DepthAttachmentResource = nullptr;

		m_ColorAttachmentsResources.clear();
		m_ColorAttachmentsFormats.clear();
		m_AttachmentImageViews.clear();
	}

	void Framebuffer::Recreate( uint32_t Width, uint32_t Height, const FramebufferSpecification& newSpec /*= {}*/ )
	{
		if( newSpec.ExistingImages.size() )
		{
			for( auto& [index, resource] : m_Specification.ExistingImages )
			{
				resource = nullptr;
			}
		}

		Terminate();
		
		m_Specification.ExistingImages = newSpec.ExistingImages;

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

		size_t totalImageViews = m_Specification.Attachments.Attachments.size() + m_Specification.ExistingImages.size();

		m_AttachmentImageViews.resize( totalImageViews );

		for( auto& [imageIndex, rImage] : m_Specification.ExistingImages )
		{
			if( FramebufferUtills::IsDepthFormat( rImage->GetImageFormat() ) )
			{
				m_DepthAttachmentResource = rImage;
				m_DepthFormat = rImage->GetImageFormat();
			}
			else
			{
				if( m_ColorAttachmentsResources.size() )
					m_ColorAttachmentsResources[ imageIndex ] = rImage;
				else
					m_ColorAttachmentsResources.push_back( rImage );
			}

			if( m_AttachmentImageViews.size() )
			{
				m_AttachmentImageViews[ imageIndex ] = rImage->GetImageView( m_Specification.ExistingImageLayer );
			}
			else 
			{
				m_AttachmentImageViews.push_back( rImage->GetImageView( m_Specification.ExistingImageLayer ) );
			}
		}

		int i = 0;
		for( auto format : m_ColorAttachmentsFormats )
		{
			Ref<Image2D> image = Ref<Image2D>::Create( format, m_Specification.Width, m_Specification.Height, m_Specification.ArrayLevels, m_Specification.MSAASamples );

			std::string imageDebugName = std::format( "Color Attachment for framebuffer {0} ({1})", m_Specification.RenderPass->GetName(), i );

			SetDebugUtilsObjectName( imageDebugName.c_str(), (uint64_t)image->GetImage(), VK_OBJECT_TYPE_IMAGE );

			if( imageDebugName == "Color Attachment for framebuffer Late Composite pass (0)" )
				Core::BreakDebug();

			m_ColorAttachmentsResources.push_back( image );

			if( m_AttachmentImageViews.size() )
				m_AttachmentImageViews[ i ] = image->GetImageView();
			else
				m_AttachmentImageViews.push_back( image->GetImageView() );

			i++;
		}
		
		if( !m_DepthAttachmentResource && m_Specification.CreateDepth ) 
		{
			m_DepthAttachmentResource = Ref<Image2D>::Create( m_DepthFormat, m_Specification.Width, m_Specification.Height, m_Specification.ArrayLevels, m_Specification.MSAASamples );

			std::string imageDebugName = std::format( "Depth Attachment for framebuffer {0}", m_Specification.RenderPass->GetName() );

			SetDebugUtilsObjectName( imageDebugName.c_str(), ( uint64_t ) m_DepthAttachmentResource->GetImage(), VK_OBJECT_TYPE_IMAGE );

			if( m_AttachmentImageViews.size() )
				m_AttachmentImageViews[ i ] = m_DepthAttachmentResource->GetImageView( m_Specification.ExistingImageLayer );
			else
				m_AttachmentImageViews.push_back( m_DepthAttachmentResource->GetImageView( m_Specification.ExistingImageLayer ) );
		}

		VkFramebufferCreateInfo FramebufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		FramebufferCreateInfo.renderPass = m_Specification.RenderPass->GetVulkanPass();

		FramebufferCreateInfo.attachmentCount = ( uint32_t ) m_AttachmentImageViews.size();
		FramebufferCreateInfo.pAttachments = m_AttachmentImageViews.data();

		FramebufferCreateInfo.width = m_Specification.Width;
		FramebufferCreateInfo.height = m_Specification.Height;
		FramebufferCreateInfo.layers = 1;

		VK_CHECK( vkCreateFramebuffer( VulkanContext::Get().GetDevice(), &FramebufferCreateInfo, nullptr, &m_Framebuffer ) );
	}
}