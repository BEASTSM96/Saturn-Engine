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

#pragma once

#include <vulkan.h>

namespace Saturn {

	enum class ImageFormat
	{
		None = 0,

		// Color
		RGBA8 = 1,
		RGBA16F = 2,
		RGBA32F = 3,
		RGB32F = 4,
		BGRA8 = 5,
		RED8 = 6,

		// Depth
		DEPTH32F = 7,
		DEPTH24STENCIL8 = 8,

		Depth = DEPTH32F
	};

	// Represents a Vulkan Image, ImageView and Sampler
	// This is different from the "Texture, Texture2D and TextureCube" classes because an Image2D and not be created from a file path.
	// So this should only be used as a memory only Image.
	// For images that require a file use Texture2D or TextureCube
	class Image2D : public RefTarget
	{
	public:
		Image2D( ImageFormat Format, uint32_t Width, uint32_t Height, uint32_t ArrayLevels = 1, uint32_t MSAASamples = 1, void* pData = nullptr, size_t size = 0 );
		~Image2D();

		void SetDebugName( const std::string& rName );

		void Resize( uint32_t Width, uint32_t Height );

		VkDescriptorImageInfo& GetDescriptorInfo() { return m_DescriptorImageInfo; }

		VkImage GetImage() { return m_Image; }
		VkImageView GetImageView( size_t index = 0 ) { return m_ImageViewes[ index ]; }
		VkSampler GetSampler() { return m_Sampler; }

		ImageFormat GetImageFormat() { return m_Format; }

	private:
		void Create();
		void CopyBufferToImage( VkBuffer Buffer );
		void TransitionImageLayout( VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout );

	private:
		uint32_t m_Width;
		uint32_t m_Height;

		VkSampleCountFlagBits m_MSAASamples;

		ImageFormat m_Format;

		std::vector<VkImageView> m_ImageViewes;

		VkImage m_Image;
		VkImageView m_ImageView;
		VkSampler m_Sampler;
		VkDeviceMemory m_Memory;

		uint32_t m_ArrayLevels;

		void* m_pData;
		size_t m_DataSize;

		VkDescriptorImageInfo m_DescriptorImageInfo;
	};
}