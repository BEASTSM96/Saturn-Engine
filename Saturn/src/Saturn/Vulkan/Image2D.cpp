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
#include "Image2D.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

namespace Saturn {

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

	static bool IsColorFormat( ImageFormat format )
	{
		switch( format )
		{
			case Saturn::ImageFormat::RGBA8:
			case Saturn::ImageFormat::RGBA16F:
			case Saturn::ImageFormat::RGBA32F:
			case Saturn::ImageFormat::RGB32F:
			case Saturn::ImageFormat::BGRA8:
				return true;
		}

		return false;
	}

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

	Image2D::Image2D( ImageFormat Format, uint32_t Width, uint32_t Height )
		: m_Format( Format ), m_Width( Width ), m_Height( Height )
	{
		Create();
	}

	Image2D::~Image2D()
	{
		vkDestroyImage( VulkanContext::Get().GetDevice(), m_Image, nullptr );
		vkDestroyImageView( VulkanContext::Get().GetDevice(), m_ImageView, nullptr );
		vkDestroySampler( VulkanContext::Get().GetDevice(), m_Sampler, nullptr );
		vkFreeMemory( VulkanContext::Get().GetDevice(), m_Memory, nullptr );

		m_Image = nullptr;
		m_ImageView = nullptr;
		m_Sampler = nullptr;
		m_Memory = nullptr;
	}

	void Image2D::Resize( uint32_t Width, uint32_t Height )
	{
		vkDestroyImage( VulkanContext::Get().GetDevice(), m_Image, nullptr );
		vkDestroyImageView( VulkanContext::Get().GetDevice(), m_ImageView, nullptr );
		vkDestroySampler( VulkanContext::Get().GetDevice(), m_Sampler, nullptr );
		vkFreeMemory( VulkanContext::Get().GetDevice(), m_Memory, nullptr );

		m_Width = Width;
		m_Height = Height;

		Create();
	}

	void Image2D::Create()
	{
		// Create image
		VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format = VulkanFormat( m_Format );
		ImageCreateInfo.extent = { .width = m_Width, .height = m_Height, .depth = 1 };
		// TODO: Get mip levels
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

		if( IsColorFormat( m_Format ) )
			ImageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		else
			ImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		VK_CHECK( vkCreateImage( VulkanContext::Get().GetDevice(), &ImageCreateInfo, nullptr, &m_Image ) );
		SetDebugUtilsObjectName( "Image", ( uint64_t ) m_Image, VK_OBJECT_TYPE_IMAGE );

		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements( VulkanContext::Get().GetDevice(), m_Image, &MemoryRequirements );

		VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
		MemoryAllocateInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &MemoryAllocateInfo, nullptr, &m_Memory ) );
		VK_CHECK( vkBindImageMemory( VulkanContext::Get().GetDevice(), m_Image, m_Memory, 0 ) );

		// Create image view.
		VkImageViewCreateInfo ImageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewCreateInfo.image = m_Image;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = VulkanFormat( m_Format );

		if( IsColorFormat( m_Format ) )
			ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		else
			ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		VK_CHECK( vkCreateImageView( VulkanContext::Get().GetDevice(), &ImageViewCreateInfo, nullptr, &m_ImageView ) );
		SetDebugUtilsObjectName( "Image view", ( uint64_t ) m_ImageView, VK_OBJECT_TYPE_IMAGE_VIEW );

		// Create sampler.
		VkSamplerCreateInfo SamplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerCreateInfo.mipLodBias = 0.0f;
		SamplerCreateInfo.maxAnisotropy = 1.0f;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		SamplerCreateInfo.minLod = 0.0f;
		SamplerCreateInfo.maxLod = 1.0f;
		SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK( vkCreateSampler( VulkanContext::Get().GetDevice(), &SamplerCreateInfo, nullptr, &m_Sampler ) );
		SetDebugUtilsObjectName( "Sampler", ( uint64_t ) m_Sampler, VK_OBJECT_TYPE_SAMPLER );

		if( IsColorFormat( m_Format ) )
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		else
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		m_DescriptorImageInfo.sampler = m_Sampler;
		m_DescriptorImageInfo.imageView = m_ImageView;
	}

}