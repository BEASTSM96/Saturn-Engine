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

		DEPTH32F = 6,
		DEPTH24STENCIL8 = 7,

		Depth = DEPTH32F
	};

	class Image2D : public CountedObj
	{
	public:
		Image2D( ImageFormat Format, uint32_t Width, uint32_t Height );
		~Image2D();

		void Resize( uint32_t Width, uint32_t Height );

		VkDescriptorImageInfo& GetDescriptorInfo() { return m_DescriptorImageInfo; }

		VkImage GetImage() { return m_Image; }
		VkImageView GetImageView() { return m_ImageView; }
		VkSampler GetSampler() { return m_Sampler; }

	private:
		void Create();

		uint32_t m_Width;
		uint32_t m_Height;

		ImageFormat m_Format;

		VkImage m_Image;
		VkImageView m_ImageView;
		VkSampler m_Sampler;
		VkDeviceMemory m_Memory;

		VkDescriptorImageInfo m_DescriptorImageInfo;
	};
}