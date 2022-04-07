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
#include "VulkanTexture.h"

#include <stb_image.h>

namespace Saturn {

	VulkanTexture::VulkanTexture(
		std::filesystem::path& rPath,
		VkFormat Format,
		VkImageTiling Tiling,
		VkImageUsageFlags Usage,
		VkMemoryPropertyFlags MemoryProps )
	{
		int TextureWidth, TextureHeight, TextureChannels;

		stbi_uc* pTextureData = stbi_load( rPath.string().c_str(), &TextureWidth, &TextureHeight, &TextureChannels, STBI_default );

		if( pTextureData == nullptr )
			return;

		// Create staging buffer to store texture data in Vulkan.
		void* pPixelData = pTextureData;

		VkDeviceSize ImageSize = TextureWidth * TextureHeight * 4;

		VkMemoryAllocateInfo MemoryAllocateInfo ={ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		MemoryAllocateInfo.allocationSize = ImageSize;
		MemoryAllocateInfo.memoryTypeIndex = 0;

		VkImageCreateInfo ImageCreateInfo ={ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageCreateInfo.extent.width = m_Width;
		ImageCreateInfo.extent.height = m_Height;
		ImageCreateInfo.extent.depth = 1;
		ImageCreateInfo.mipLevels = 1;
		ImageCreateInfo.arrayLayers = 1;
		ImageCreateInfo.format = Format;
		ImageCreateInfo.tiling = Tiling;
		ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageCreateInfo.usage = Usage;

		VkMemoryRequirements MemoryRequirements;
		VkBuffer Buffer;
		VkDeviceMemory Memory;

		VkBufferCreateInfo BufferCreateInfo ={ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferCreateInfo.size = ImageSize;
		BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK( vkCreateBuffer( VulkanContext::Get().GetDevice(), &BufferCreateInfo, nullptr, &Buffer ) );

	}

	VulkanTexture::~VulkanTexture()
	{
	}

	void VulkanTexture::Init()
	{

	}
}