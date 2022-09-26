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

	static VkSamplerAddressMode SaturnToVulkanAdressingMode( AddressingMode mode ) 
	{
		switch( mode )
		{
			case Saturn::AddressingMode::Repeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case Saturn::AddressingMode::MirroredRepeat:
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case Saturn::AddressingMode::ClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case Saturn::AddressingMode::ClampToBorder:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			default:
				break;
		}

		return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
	}

	static void ImagePipelineBarrier( 
		VkCommandBuffer CommandBuffer, VkImage Image, VkAccessFlags SrcMask, VkAccessFlags DstMask, VkImageLayout OldImageLayout, VkImageLayout NewImageLayout, VkPipelineStageFlags SrcStage, VkPipelineStageFlags DstStage, VkImageSubresourceRange Range )
	{
		VkImageMemoryBarrier ImageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		ImageMemoryBarrier.image = Image;
		ImageMemoryBarrier.subresourceRange = Range;
		ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ImageMemoryBarrier.srcAccessMask = SrcMask;
		ImageMemoryBarrier.dstAccessMask = DstMask;
		ImageMemoryBarrier.oldLayout = OldImageLayout;
		ImageMemoryBarrier.newLayout = NewImageLayout;

		vkCmdPipelineBarrier( CommandBuffer, SrcStage, DstStage, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier );
	}

	//////////////////////////////////////////////////////////////////////////

	Texture::Texture( uint32_t width, uint32_t height, VkFormat Format, const void* pData )
		: m_Width( width ), m_Height( height ), m_ImageFormat( Format )
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
		else if( OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ) 
		{
			ImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			ImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			DstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		vkCmdPipelineBarrier( CommandBuffer, SrcStage, DstStage, 0, 0, nullptr, 0, nullptr, 1, &ImageBarrier );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	void Texture::TransitionImageLayout( VkImageSubresourceRange& rCommand, VkImageLayout OldLayout, VkImageLayout NewLayout )
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
		ImageBarrier.subresourceRange = rCommand;

		/*
		switch( OldLayout )
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				ImageBarrier.srcAccessMask = 0;
				break;
			case VK_IMAGE_LAYOUT_GENERAL:
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				ImageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
			case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR:
			case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR:
				break;
		}

		switch( NewLayout )
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
			case VK_IMAGE_LAYOUT_GENERAL:
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				ImageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				ImageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
			case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
			case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
			case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR:
			case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR:
				break;
		}
		*/

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
		else if( OldLayout == VK_IMAGE_LAYOUT_UNDEFINED  && NewLayout == VK_IMAGE_LAYOUT_GENERAL ) 
		{
			ImageBarrier.srcAccessMask = 0;

			SrcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			DstStage = SrcStage;
		}

		vkCmdPipelineBarrier( CommandBuffer, SrcStage, DstStage, 0, 0, nullptr, 0, nullptr, 1, &ImageBarrier );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	uint32_t Texture::GetMipMapLevels()
	{
		// Based from https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/ch06lev2sec20.html
		return std::floor( std::log2( glm::min( m_Width, m_Height ) ) ) + 1;
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
		VkImageType ImageType,
		VkImageTiling Tiling,
		VkImageUsageFlags Usage,
		VkMemoryPropertyFlags MemProps,
		VkImage& rImage, VkDeviceMemory& rDeviceMemory, uint32_t MipLevels, uint32_t ArrayLevels, VkImageCreateFlags Flags )
	{
		VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageCreateInfo.imageType = ImageType;
		ImageCreateInfo.extent.width = Width;
		ImageCreateInfo.extent.height = Height;
		ImageCreateInfo.extent.depth = 1;
		ImageCreateInfo.mipLevels = MipLevels;
		ImageCreateInfo.arrayLayers = ArrayLevels;
		ImageCreateInfo.format = Format;
		ImageCreateInfo.tiling = Tiling;
		ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageCreateInfo.usage = Usage;
		ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageCreateInfo.flags = Flags;

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
		ImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		VkImageView ImageView;
		VK_CHECK( vkCreateImageView( VulkanContext::Get().GetDevice(), &ImageViewCreateInfo, nullptr, &ImageView ) );

		return ImageView;
	}

	VkImageView CreateImageView( const VkImageSubresourceRange& rRange, VkImage Image, VkFormat Format )
	{
		VkImageViewCreateInfo ImageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewCreateInfo.image = Image;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ImageViewCreateInfo.format = Format;
		ImageViewCreateInfo.subresourceRange = rRange;
		ImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

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
	void Texture2D::CreateTextureImage( bool flip )
	{
		if( !std::filesystem::exists( m_Path ) )
			SAT_CORE_ASSERT( false, "Path does not exist!" );

		int Width, Height, Channels;

		// Flip texture
		stbi_set_flip_vertically_on_load( flip );

		stbi_uc* pTextureData;

		if( stbi_is_hdr( m_Path.string().c_str() ) )
		{
			SAT_CORE_INFO( "Loading HDR texture {0}", m_Path.string() );
			pTextureData = ( uint8_t* ) stbi_loadf( m_Path.string().c_str(), &Width, &Height, &Channels, 4 );

			m_HDR = true;
		}
		else
		{
			SAT_CORE_INFO( "Loading texture {0}", m_Path.string() );

			pTextureData = stbi_load( m_Path.string().c_str(), &Width, &Height, &Channels, 4 );

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

		VkDeviceSize ImageSize = m_Width * m_Height * 4;

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		auto MipCount = GetMipMapLevels();

		// Staging Buffer.
		VkBuffer StagingBuffer;

		VkBufferCreateInfo BufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = ImageSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto rBufferAlloc = pAllocator->AllocateBuffer( BufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, &StagingBuffer );

		void* pDstData = pAllocator->MapMemory< void* >( rBufferAlloc );

		memcpy( pDstData, pTextureData, ImageSize );

		pAllocator->UnmapMemory( rBufferAlloc );

		stbi_image_free( pTextureData );

		// Create the image.
		CreateImage( Width, Height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory, MipCount, 1 );

		TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

		CopyBufferToImage( StagingBuffer );

		if( MipCount > 1 )
			TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );
		else
			TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		// Create image views
		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;
		range.layerCount = 1;
		range.levelCount = MipCount;

		m_ImageView = CreateImageView( range, m_Image, VK_FORMAT_R8G8B8A8_UNORM );

		// Create sampler
		VkSamplerCreateInfo SamplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		SamplerCreateInfo.addressModeU = SaturnToVulkanAdressingMode( m_AddressingMode );
		SamplerCreateInfo.addressModeV = SaturnToVulkanAdressingMode( m_AddressingMode );
		SamplerCreateInfo.addressModeW = SaturnToVulkanAdressingMode( m_AddressingMode );

		SamplerCreateInfo.anisotropyEnable = VK_FALSE;

		// We don't know the max anisotropy level, so we'll need to get it from the properties of the physical device.
		// We do this as this is the best way to get the max anisotropy level as it can be different on other devices.
		VkPhysicalDeviceProperties Properties = {};
		vkGetPhysicalDeviceProperties( VulkanContext::Get().GetPhysicalDevice(), &Properties );

		SamplerCreateInfo.maxAnisotropy = Properties.limits.maxSamplerAnisotropy;
		SamplerCreateInfo.maxAnisotropy = 1.0f;

		SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerCreateInfo.mipLodBias = 0.0f;
		SamplerCreateInfo.minLod = 0.0f;
		SamplerCreateInfo.maxLod = MipCount;

		VK_CHECK( vkCreateSampler( VulkanContext::Get().GetDevice(), &SamplerCreateInfo, nullptr, &m_Sampler ) );

		m_DescriptorSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		m_DescriptorImageInfo = {};
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;
		
		pAllocator->DestroyBuffer( StagingBuffer );

		CreateMips();
	}

	void Texture2D::CreateMips()
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		uint32_t mips = GetMipMapLevels();

		VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.image = m_Image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		for( size_t i = 1; i < mips; i++ )
		{
			VkImageBlit imageBlit{};

			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[ 1 ].x = int32_t( m_Width >> ( i - 1 ) );
			imageBlit.srcOffsets[ 1 ].y = int32_t( m_Height >> ( i - 1 ) );
			imageBlit.srcOffsets[ 1 ].z = 1;

			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[ 1 ].x = int32_t( m_Width >> i );
			imageBlit.dstOffsets[ 1 ].y = int32_t( m_Height >> i );
			imageBlit.dstOffsets[ 1 ].z = 1;

			VkImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			ImagePipelineBarrier( CommandBuffer, m_Image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange );

			vkCmdBlitImage( CommandBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR );

			ImagePipelineBarrier( CommandBuffer, m_Image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange );
		}

		VkImageSubresourceRange range =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = mips,
			.baseArrayLayer = 0,
			.layerCount = 1
		};

		ImagePipelineBarrier( CommandBuffer, m_Image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, range );

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );
	}

	void Texture2D::SetData( const void* pData )
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		VkDeviceSize ImageSize = m_Width * m_Height * 4;

		auto MipCount = GetMipMapLevels();

		// Staging Buffer.
		VkBuffer StagingBuffer;

		VkBufferCreateInfo BufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = ImageSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		auto rBufferAlloc = pAllocator->AllocateBuffer( BufferCreateInfo, VMA_MEMORY_USAGE_CPU_ONLY, &StagingBuffer );

		void* pDstData = pAllocator->MapMemory< void* >( rBufferAlloc );

		memcpy( pDstData, pData, ImageSize );

		pAllocator->UnmapMemory( rBufferAlloc );

		// Create the image.
		if( m_ImageMemory )
			vkFreeMemory( VulkanContext::Get().GetDevice(), m_ImageMemory, nullptr );

		if( m_Image )
			vkDestroyImage( VulkanContext::Get().GetDevice(), m_Image, nullptr );

		CreateImage( m_Width, m_Height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory, MipCount, 1 );

		TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

		CopyBufferToImage( StagingBuffer );

		if( MipCount > 1 )
			TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );
		else
			TransitionImageLayout( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		// Create image views
		if( m_ImageView )
			vkDestroyImageView( VulkanContext::Get().GetDevice(), m_ImageView, nullptr );

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;
		range.layerCount = 1;
		range.levelCount = MipCount;

		m_ImageView = CreateImageView( range, m_Image, VK_FORMAT_R8G8B8A8_UNORM );

		// Create sampler
		VkSamplerCreateInfo SamplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		SamplerCreateInfo.addressModeU = SaturnToVulkanAdressingMode( m_AddressingMode );
		SamplerCreateInfo.addressModeV = SaturnToVulkanAdressingMode( m_AddressingMode );
		SamplerCreateInfo.addressModeW = SaturnToVulkanAdressingMode( m_AddressingMode );

		SamplerCreateInfo.anisotropyEnable = VK_FALSE;

		// We don't know the max anisotropy level, so we'll need to get it from the properties of the physical device.
		// We do this as this is the best way to get the max anisotropy level as it can be different on other devices.
		VkPhysicalDeviceProperties Properties = {};
		vkGetPhysicalDeviceProperties( VulkanContext::Get().GetPhysicalDevice(), &Properties );

		SamplerCreateInfo.maxAnisotropy = 1;

		SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
		SamplerCreateInfo.compareEnable = VK_FALSE;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerCreateInfo.mipLodBias = 0.0f;
		SamplerCreateInfo.minLod = 0.0f;
		SamplerCreateInfo.maxLod = MipCount;

		if( m_Sampler )
			vkDestroySampler( VulkanContext::Get().GetDevice(), m_Sampler, nullptr );

		VK_CHECK( vkCreateSampler( VulkanContext::Get().GetDevice(), &SamplerCreateInfo, nullptr, &m_Sampler ) );

		m_DescriptorImageInfo = {};
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorImageInfo.imageView = m_ImageView;
		m_DescriptorImageInfo.sampler = m_Sampler;

		m_DescriptorSet = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_Sampler, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		if( MipCount > 1 )
			CreateMips();
	}

	//////////////////////////////////////////////////////////////////////////
	// Texture Cube
	//////////////////////////////////////////////////////////////////////////

	TextureCube::TextureCube( ImageFormat Format, uint32_t width, uint32_t height, const void* pData /*= nullptr */ )
		: Texture( width, height, VulkanFormat( Format ), pData )
	{
		m_pData = ( void* ) pData;

		CreateTextureImage( false );
	}

	void TextureCube::CreateMips()
	{
		VkCommandBuffer CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		uint32_t mips = GetMipMapLevels();
		for( uint32_t i = 0; i < 6; i++ )
		{
			VkImageSubresourceRange range =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = i,
				.layerCount = 1
			};

			ImagePipelineBarrier( CommandBuffer, m_Image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, range );
		}

		for( int32_t i = 1; i < mips; i++ )
		{
			for( uint32_t j = 0; j < 6; j++ )
			{
				VkImageBlit imageBlit{};

				imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlit.srcSubresource.layerCount = 1;
				imageBlit.srcSubresource.mipLevel = i - 1;
				imageBlit.srcSubresource.baseArrayLayer = j;
				imageBlit.srcOffsets[ 1 ].x = int32_t( m_Width >> ( i - 1 ) );
				imageBlit.srcOffsets[ 1 ].y = int32_t( m_Height >> ( i - 1 ) );
				imageBlit.srcOffsets[ 1 ].z = 1;

				imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlit.dstSubresource.layerCount = 1;
				imageBlit.dstSubresource.mipLevel = i;
				imageBlit.dstSubresource.baseArrayLayer = j;
				imageBlit.dstOffsets[ 1 ].x = int32_t( m_Width >> i );
				imageBlit.dstOffsets[ 1 ].y = int32_t( m_Height >> i );
				imageBlit.dstOffsets[ 1 ].z = 1;

				VkImageSubresourceRange mipSubRange = {};
				mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				mipSubRange.baseMipLevel = i;
				mipSubRange.baseArrayLayer = j;
				mipSubRange.levelCount = 1;
				mipSubRange.layerCount = 1;

				ImagePipelineBarrier( CommandBuffer, m_Image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange );

				vkCmdBlitImage( CommandBuffer, m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR );

				ImagePipelineBarrier( CommandBuffer, m_Image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange );
			}
		}

		VkImageSubresourceRange range = 
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = mips,
			.baseArrayLayer = 0,
			.layerCount = 6
		};

		ImagePipelineBarrier( CommandBuffer, m_Image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, range );

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	}

	void TextureCube::Terminate()
	{
		Texture::Terminate();
	}

	void TextureCube::CreateTextureImage( bool flip )
	{
		// Create the image.
		CreateImage( m_Width, m_Height, m_ImageFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory, GetMipMapLevels(), 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT );

		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = GetMipMapLevels();
		subresourceRange.layerCount = 6;

		TransitionImageLayout( subresourceRange, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL );

		// Create image sampler.
		VkSamplerCreateInfo SamplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		SamplerCreateInfo.maxAnisotropy = 1.0f;
		SamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		SamplerCreateInfo.mipLodBias = 0.0f;
		SamplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		SamplerCreateInfo.minLod = 0.0f;
		SamplerCreateInfo.maxLod = GetMipMapLevels();
		SamplerCreateInfo.anisotropyEnable = VK_FALSE;
		SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK( vkCreateSampler( VulkanContext::Get().GetDevice(), &SamplerCreateInfo, nullptr, &m_Sampler ) );

		// Create image view

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;
		range.layerCount = 6;
		range.levelCount = GetMipMapLevels();

		VkImageViewCreateInfo ImageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ImageViewCreateInfo.image = m_Image;
		ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		ImageViewCreateInfo.format = m_ImageFormat;
		ImageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		ImageViewCreateInfo.subresourceRange = range;

		VK_CHECK( vkCreateImageView( VulkanContext::Get().GetDevice(), &ImageViewCreateInfo, nullptr, &m_ImageView ) );

		m_DescriptorImageInfo.sampler = m_Sampler;
		m_DescriptorImageInfo.imageView = m_ImageView;
	}

	void TextureCube::SetData( const void* pData )
	{

	}

}