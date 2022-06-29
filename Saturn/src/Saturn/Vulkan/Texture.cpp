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
#include "Texture.h"

#include "VulkanContext.h"

#include <stb_image.h>
#include <backends/imgui_impl_vulkan.h>

namespace Saturn {

	Texture::Texture( uint32_t width, uint32_t height, VkFormat Format, const void* pData )
		: m_Width( width ), m_Height( height )
	{
		m_pData = ( void* ) pData;
	}

	void Texture::Terminate()
	{
		if( m_IsRendererTexture && !m_ForceTerminate )
			return;

		if( m_Image )
			vkDestroyImage( VulkanContext::Get().GetDevice(), m_Image, nullptr );

		if( m_ImageMemory )
			vkFreeMemory( VulkanContext::Get().GetDevice(), m_ImageMemory, nullptr );

		if( m_ImageView )
			vkDestroyImageView( VulkanContext::Get().GetDevice(), m_ImageView, nullptr );

		if( m_Sampler )
			vkDestroySampler( VulkanContext::Get().GetDevice(), m_Sampler, nullptr );

		m_Image = nullptr;
		m_ImageMemory = nullptr;
		m_ImageView = nullptr;
		m_Sampler = nullptr;
	}

	void Texture::TransitionImageLayout( VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout )
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		VkPipelineStageFlags SrcStage;
		VkPipelineStageFlags DstStage;

		VkImageMemoryBarrier ImageBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		ImageBarrier.oldLayout = OldLayout;
		ImageBarrier.newLayout = NewLayout;
		ImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageBarrier.image = m_Image;
		ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageBarrier.subresourceRange.baseMipLevel = 0;
		ImageBarrier.subresourceRange.levelCount = 1;
		ImageBarrier.subresourceRange.baseArrayLayer = 0;
		ImageBarrier.subresourceRange.layerCount = 1;

		if( OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
		{
			ImageBarrier.srcAccessMask = 0;
			ImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			DstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if( OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
		{
			ImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			ImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			DstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier( CommandBuffer, SrcStage, DstStage, 0, 0, nullptr, 0, nullptr, 1, &ImageBarrier );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	void Texture::CopyBufferToImage( VkBuffer Buffer )
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		VkBufferImageCopy Region = {};
		Region.bufferOffset = 0;
		Region.bufferRowLength = 0;
		Region.bufferImageHeight = 0;

		Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Region.imageSubresource.mipLevel = 0;
		Region.imageSubresource.baseArrayLayer = 0;
		Region.imageSubresource.layerCount = 1;

		Region.imageOffset = { 0, 0, 0 };
		Region.imageExtent = { ( uint32_t ) m_Width, ( uint32_t ) m_Height, 1 };

		vkCmdCopyBufferToImage( CommandBuffer, Buffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	//////////////////////////////////////////////////////////////////////////
	// GLOBAL HELPERS														//
	//////////////////////////////////////////////////////////////////////////

	void CreateImage(
		uint32_t Width,
		uint32_t Height,
		VkFormat Format,
		VkImageTiling Tiling,
		VkImageUsageFlags Usage,
		VkMemoryPropertyFlags MemProps,
		VkImage& rImage, VkDeviceMemory& rDeviceMemory )
	{
		VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.extent.width = Width;
		ImageCreateInfo.extent.height = Height;
		ImageCreateInfo.extent.depth = 1;
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.format = Format;
		ImageCreateInfo.tiling = Tiling;
		ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageCreateInfo.usage = Usage;
		ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		VK_CHECK( vkCreateImage( VulkanContext::Get().GetDevice(), &ImageCreateInfo, nullptr, &rImage ) );

		VkMemoryRequirements MemReq;
		vkGetImageMemoryRequirements( VulkanContext::Get().GetDevice(), rImage, &MemReq );

		VkMemoryAllocateInfo AllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		AllocInfo.allocationSize = MemReq.size;
		AllocInfo.memoryTypeIndex = VulkanContext::Get().GetMemoryType( MemReq.memoryTypeBits, MemProps );

		VK_CHECK( vkAllocateMemory( VulkanContext::Get().GetDevice(), &AllocInfo, nullptr, &rDeviceMemory ) );

		vkBindImageMemory( VulkanContext::Get().GetDevice(), rImage, rDeviceMemory, 0 );
	}

	VkImageView CreateImageView( VkImage Image, VkFormat Format )
	{
		VkImageViewCreateInfo ImageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewCreateInfo.image = Image;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = Format;
		ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		VkImageView ImageView;
		VK_CHECK( vkCreateImageView( VulkanContext::Get().GetDevice(), &ImageViewCreateInfo, nullptr, &ImageView ) );

		return ImageView;
	}

	VkImageView CreateImageView( VkImage Image, VkFormat Format, VkImageAspectFlags AspectMask )
	{
		VkImageViewCreateInfo ImageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewCreateInfo.image = Image;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = Format;
		ImageViewCreateInfo.subresourceRange.aspectMask = AspectMask;
		ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		ImageViewCreateInfo.subresourceRange.levelCount = 1;
		ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		ImageViewCreateInfo.subresourceRange.layerCount = 1;

		VkImageView ImageView;
		VK_CHECK( vkCreateImageView( VulkanContext::Get().GetDevice(), &ImageViewCreateInfo, nullptr, &ImageView ) );

		return ImageView;
	}

	void TransitionImageLayout( VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout )
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		VkPipelineStageFlags SrcStage;
		VkPipelineStageFlags DstStage;

		VkImageMemoryBarrier ImageBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		ImageBarrier.oldLayout = OldLayout;
		ImageBarrier.newLayout = NewLayout;
		ImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageBarrier.image = Image;
		ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ImageBarrier.subresourceRange.baseMipLevel = 0;
		ImageBarrier.subresourceRange.levelCount = 1;
		ImageBarrier.subresourceRange.baseArrayLayer = 0;
		ImageBarrier.subresourceRange.layerCount = 1;

		if( OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
		{
			ImageBarrier.srcAccessMask = 0;
			ImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			DstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if( OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
		{
			ImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			ImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			DstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier( CommandBuffer, SrcStage, DstStage, 0, 0, nullptr, 0, nullptr, 1, &ImageBarrier );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	//////////////////////////////////////////////////////////////////////////
	// TEXTURE 2D															//
	//////////////////////////////////////////////////////////////////////////

	void Texture2D::Terminate()
	{
		if( m_IsRendererTexture && !m_ForceTerminate )
			return;

		Texture::Terminate();
		
		//if( m_DescriptorSet )
		//	ImGui_ImplVulkan_RemoveTexture( m_DescriptorSet );
	}

	// Load and create a texture 2D for a file path.
	// Create a texture 2D
	void Texture2D::CreateTextureImage()
	{
		if( !std::filesystem::exists( m_Path ) )
			SAT_CORE_ASSERT( false, "Path does not exist!" );

		int Width, Height, Channels;

		// Flip texture
		stbi_set_flip_vertically_on_load( true );

		stbi_uc* pTextureData;

		if( stbi_is_hdr( m_Path.string().c_str() ) )
		{
			SAT_CORE_INFO( "Loading HDR texture {0}", m_Path.string() );
			pTextureData = ( uint8_t* ) stbi_loadf( m_Path.string().c_str(), &Width, &Height, &Channels, 0 );

			m_HDR = true;
		}
		else
		{
			SAT_CORE_INFO( "Loading texture {0}", m_Path.string() );

			// Load the hdr texture.
			pTextureData = stbi_load( m_Path.string().c_str(), &Width, &Height, &Channels, STBI_rgb_alpha );

			m_HDR = false;
		}

		if( !std::filesystem::exists( m_Path ) )
		{
			SAT_CORE_ERROR( "Failed to load texture image: {0}", m_Path.string() );
			return;
		}

		m_pData = pTextureData;

		m_Width = Width;
		m_Height = Height;

		VkDeviceSize ImageSize = Width * Height * 4;

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// Staging Buffer.
		VkBuffer StagingBuffer;

		VkBufferCreateInfo BufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = ImageSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto rBufferAlloc = pAllocator->AllocateBuffer( BufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, &StagingBuffer );

		void* pDstData = pAllocator->MapMemory< void >( rBufferAlloc );

		memcpy( pDstData, pTextureData, ImageSize );

		pAllocator->UnmapMemory( rBufferAlloc );

		stbi_image_free( pTextureData );

		// Create the image.
		CreateImage( Width, Height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory );

		TransitionImageLayout( VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

		CopyBufferToImage( StagingBuffer );

		TransitionImageLayout( VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		// Create image views
		m_ImageView = CreateImageView( m_Image, VK_FORMAT_R8G8B8A8_SRGB );

		// Create sampler
		VkSamplerCreateInfo SamplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		switch( m_AddressingMode )
		{
			case AddressingMode::ClampToBorder:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			} break;

			case AddressingMode::Repeat:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			} break;

			case AddressingMode::MirroredRepeat:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			} break;

			case AddressingMode::ClampToEdge:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			} break;
		}

		SamplerCreateInfo.anisotropyEnable = VK_TRUE;

		// We don't know the max anisotropy level, so we'll need to get it from the properties of the physical device.
		// We do this as this is the best way to get the max anisotropy level as it can be different on other devices.
		VkPhysicalDeviceProperties Properties = {};
		vkGetPhysicalDeviceProperties( VulkanContext::Get().GetPhysicalDevice(), &Properties );

		SamplerCreateInfo.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;

		SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerCreateInfo.mipLodBias = 0.0f;
		SamplerCreateInfo.minLod = 0.0f;
		SamplerCreateInfo.maxLod = 0.0f;

		VK_CHECK( vkCreateSampler( VulkanContext::Get().GetDevice(), &SamplerCreateInfo, nullptr, &m_Sampler ) );

		m_DescriptorSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		m_DescriptorImageInfo = {};
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;
		
		pAllocator->DestroyBuffer( StagingBuffer );
	}

	void Texture2D::SetData( const void* pData )
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		VkDeviceSize ImageSize = m_Width * m_Height * 4;

		// Staging Buffer.
		VkBuffer StagingBuffer;

		VkBufferCreateInfo BufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = ImageSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto rBufferAlloc = pAllocator->AllocateBuffer( BufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, &StagingBuffer );

		void* pDstData = pAllocator->MapMemory< void >( rBufferAlloc );

		memcpy( pDstData, pData, ImageSize );

		pAllocator->UnmapMemory( rBufferAlloc );

		// Create the image.
		if( m_ImageMemory )
			vkFreeMemory( VulkanContext::Get().GetDevice(), m_ImageMemory, nullptr );

		if( m_Image )
			vkDestroyImage( VulkanContext::Get().GetDevice(), m_Image, nullptr );

		CreateImage( m_Width, m_Height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory );

		TransitionImageLayout( VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

		CopyBufferToImage( StagingBuffer );

		TransitionImageLayout( VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		// Create image views
		if( m_ImageView )
			vkDestroyImageView( VulkanContext::Get().GetDevice(), m_ImageView, nullptr );

		m_ImageView = CreateImageView( m_Image, VK_FORMAT_R8G8B8A8_SRGB );

		// Create sampler
		VkSamplerCreateInfo SamplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		switch( m_AddressingMode )
		{
			case AddressingMode::ClampToBorder:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			} break;

			case AddressingMode::Repeat:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			} break;

			case AddressingMode::MirroredRepeat:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			} break;

			case AddressingMode::ClampToEdge:
			{
				SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			} break;
		}

		SamplerCreateInfo.anisotropyEnable = VK_TRUE;

		// We don't know the max anisotropy level, so we'll need to get it from the properties of the physical device.
		// We do this as this is the best way to get the max anisotropy level as it can be different on other devices.
		VkPhysicalDeviceProperties Properties = {};
		vkGetPhysicalDeviceProperties( VulkanContext::Get().GetPhysicalDevice(), &Properties );

		SamplerCreateInfo.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;

		SamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerCreateInfo.mipLodBias = 0.0f;
		SamplerCreateInfo.minLod = 0.0f;
		SamplerCreateInfo.maxLod = 0.0f;

		if( m_Sampler )
			vkDestroySampler( VulkanContext::Get().GetDevice(), m_Sampler, nullptr );

		VK_CHECK( vkCreateSampler( VulkanContext::Get().GetDevice(), &SamplerCreateInfo, nullptr, &m_Sampler ) );

		m_DescriptorImageInfo = {};
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;

		m_DescriptorSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
	}
}