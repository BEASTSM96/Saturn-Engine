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

#include "VulkanContext.h"

#include "UniformBuffer.h"

namespace Saturn {

	// I don't want to use the class on a TextureXX, as I feel like there is really no point.
	class Resource
	{
	public:
		//void Create( const VkImageCreateInfo* ImageInfo, const VkImageViewCreateInfo* ImageViewCreateInfo, const VkSamplerCreateInfo* SamplerCreateInfo );

		operator VkImage() const { return Image; }
		operator VkImageView() const { return ImageView; }
		operator VkSampler() const { return Sampler; }
		operator VkDeviceMemory() const { return Memory; }

	public:
		
		VkImage Image;
		VkImageView ImageView;
		VkSampler Sampler;
		VkDeviceMemory Memory;
	};

	class Renderer
	{
		SINGLETON( Renderer );
	public:
		Renderer();
		~Renderer();

		void SubmitFullscrenQuad( VkCommandBuffer CommandBuffer, Saturn::Pipeline Pipeline );
		void SubmitFullscrenQuad( VkCommandBuffer CommandBuffer, Saturn::Pipeline Pipeline, VkDescriptorSet DescriptorSet, void* UBO );
		
		void SubmitFullscrenQuad( VkCommandBuffer CommandBuffer, Saturn::Pipeline Pipeline, VkDescriptorSet DescriptorSet, UniformBuffer* UBO, IndexBuffer* pIndexBuffer, VertexBuffer* pVertexBuffer );

		// Render pass helpers.
		void BeginRenderPass( VkCommandBuffer CommandBuffer, Pass& rPass );
		void BeginRenderPass( VkCommandBuffer CommandBuffer);

		void RenderMeshWithMaterial();

		// Static mesh
		void RenderStaticMesh( VkCommandBuffer CommandBuffer, Saturn::Pipeline Pipeline, UUID uuid, Ref< Mesh > mesh, const glm::mat4 transform, UniformBuffer& rUBO );
		
		// Allocate command buffer.
		VkCommandBuffer AllocateCommandBuffer( VkCommandPool CommandPool );

		// Helpers.
		void CreateFramebuffer( VkRenderPass RenderPass, VkExtent2D Extent, std::vector<VkImageView> Attachments, VkFramebuffer* pFramebuffer );

		void CreateImage( VkImageType Type, VkFormat Format, VkExtent3D Extent, VkImageUsageFlags Usage, VkImage* pImage, VkDeviceMemory* pMemory );
		
		void CreateImageView( VkImage Image, VkFormat Format, VkImageAspectFlags Aspect, VkImageView* pImageView );

		void CreateSampler( VkFilter Filter, VkSampler* pSampler );
		
		//////////////////////////////////////////////////////////////////////////
		// FRAME BEGINGING AND ENDING.
		//////////////////////////////////////////////////////////////////////////
		
		void BeginFrame();
		void EndFrame();
		
		uint32_t GetImageIndex() { return m_ImageIndex; }

		std::pair< float, float > GetFrameTimings() { return std::make_pair( m_BeginFrameTime, m_EndFrameTime ); }
		float GetQueuePresentTime() { return m_QueuePresentTime; }

	public:

		VkCommandBuffer ActiveCommandBuffer() { return m_CommandBuffer; };

	private:
		
		void Init();
		void Terminate();

	private:

		uint32_t m_ImageIndex;
		uint32_t m_ImageCount;
		uint32_t m_FrameCount = 0;

		std::vector<VkFence> m_FlightFences;
		
		VkSemaphore m_AcquireSemaphore;
		VkSemaphore m_SubmitSemaphore;

		VkCommandBuffer m_CommandBuffer;

		Timer m_BeginFrameTimer;
		float m_BeginFrameTime;

		Timer m_EndFrameTimer;
		float m_EndFrameTime;

		Timer m_QueuePresentTimer;
		float m_QueuePresentTime;
	};
}