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

#include "Base.h"
#include "Buffer.h"

#include <filesystem>

namespace Saturn {
	
	extern void CreateImage( 
						uint32_t Width, 
						uint32_t Height,
						VkFormat Format, 
						VkImageTiling Tiling,
						VkImageUsageFlags Usage, 
						VkMemoryPropertyFlags MemProps,
						VkImage& rImage, VkDeviceMemory& rDeviceMemory );

	extern VkImageView CreateImageView( 
						VkImage Image, 
						VkFormat Format );
	
	extern VkImageView CreateImageView(
					VkImage Image,
					VkFormat Format,
					VkImageAspectFlags AspectFlags );

	extern void TransitionImageLayout( VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout );

	enum class AddressingMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};

	class Texture
	{
	public:
		Texture() {}
		Texture( std::filesystem::path Path, AddressingMode Mode ) : m_Path( Path ), m_AddressingMode( Mode ) {}
		~Texture() { Terminate(); }
		
		virtual void Terminate() = 0;

		void TransitionImageLayout( VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout );

		void CopyBufferToImage( Buffer& rBuffer );

	public:
		
		VkSampler& GetSampler() { return m_Sampler; }
		VkImageView& GetImageView() { return m_ImageView; }
		VkImage& GetImage() { return m_Image; }

		int Width() { return m_Width; }
		int Height() { return m_Height; }

	public:

		virtual void CreateTextureImage() = 0;

	public:

		std::filesystem::path m_Path = "";
		
		VkImage m_Image = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		
		bool m_HDR = false;

		AddressingMode m_AddressingMode = AddressingMode::Repeat;

		int m_Width = 0;
		int m_Height = 0;
	};
	
	class Texture2D : public Texture
	{
	public:
		Texture2D() : Texture() {}
		Texture2D( std::filesystem::path Path, AddressingMode Mode ) : Texture( Path, Mode ) { CreateTextureImage(); }
		~Texture2D() { Terminate(); }
		
		void Terminate() override;

	private:

		void CreateTextureImage() override;
	};

	class CubeMapTexture : public Texture
	{
	public:
		CubeMapTexture() : Texture() {}
		CubeMapTexture( std::filesystem::path Path, AddressingMode Mode ) : Texture( Path, Mode ) { CreateTextureImage(); }
		~CubeMapTexture() { Terminate(); }

		void Terminate() override;

	private:

		void CreateTextureImage() override;
	};
}