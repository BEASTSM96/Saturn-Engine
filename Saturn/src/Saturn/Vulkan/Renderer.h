/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "EnvironmentMap.h"
#include "StorageBufferSet.h"

namespace Saturn {

	class Renderer : public CountedObj
	{
	public:
		static inline Renderer& Get() { return *SingletonStorage::Get().GetSingleton<Renderer>(); }
	public:
		Renderer();
		~Renderer();

		void SubmitFullscreenQuad( VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, Ref< DescriptorSet >& rDescriptorSet, IndexBuffer* pIndexBuffer, VertexBuffer* pVertexBuffer );

		// Render pass helpers.
		void BeginRenderPass( VkCommandBuffer CommandBuffer, Pass& rPass );
		void EndRenderPass( VkCommandBuffer CommandBuffer );

		void RenderMeshWithoutMaterial( VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, Ref<StaticMesh> mesh, const glm::mat4 transform, Buffer additionalData = Buffer() );

		// Static mesh
		void RenderSubmesh( VkCommandBuffer CommandBuffer, Ref<Saturn::Pipeline> Pipeline, Ref< StaticMesh > mesh, Submesh& rSubmsh, const glm::mat4 transform );

		void SubmitMesh( VkCommandBuffer CommandBuffer, Ref< Saturn::Pipeline > Pipeline, Ref< StaticMesh > mesh, Ref<StorageBufferSet>& rStorageBufferSet, const glm::mat4 transform, uint32_t SubmeshIndex );

		const std::vector<VkWriteDescriptorSet>& GetStorageBufferWriteDescriptors( Ref<StorageBufferSet>& rStorageBufferSet, Ref<MaterialAsset>& rMaterialAsset );

		void SetSceneEnvironment( Ref<Image2D> ShadowMap, Ref<EnvironmentMap> Environment, Ref<Texture2D> BDRF );

		// Allocate command buffer.
		VkCommandBuffer AllocateCommandBuffer( VkCommandPool CommandPool );
		VkCommandBuffer AllocateCommandBuffer( VkCommandBufferLevel CmdLevel );

		//////////////////////////////////////////////////////////////////////////
		// FRAME BEGINGING AND ENDING.
		//////////////////////////////////////////////////////////////////////////
		
		void BeginFrame();
		void EndFrame();
		
		uint32_t GetImageIndex() { return m_ImageIndex; }
		uint32_t GetCurrentFrame() { return m_FrameCount; }

		std::pair< float, float > GetFrameTimings() { return std::make_pair( m_BeginFrameTime, m_EndFrameTime ); }
		float GetQueuePresentTime() { return m_QueuePresentTime; }

		void SubmitTerminateResource( std::function<void()>&& rrFunction );

		Ref< Texture2D >   GetPinkTexture() { return m_PinkTexture; }
		Ref< TextureCube > GetPinkTextureCube() { return m_PinkTextureCube; }

		void CreateFullscreenQuad( VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer );
		
		Ref<DescriptorPool> GetDescriptorPool() { return m_RendererDescriptorPools[ m_FrameCount ]; }

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
		
		std::vector< std::function<void()> > m_TerminateResourceFuncs;
		
		VkSemaphore m_AcquireSemaphore;
		VkSemaphore m_SubmitSemaphore;

		VkCommandBuffer m_CommandBuffer;

		Timer m_BeginFrameTimer;
		float m_BeginFrameTime;

		Timer m_EndFrameTimer;
		float m_EndFrameTime;

		Timer m_QueuePresentTimer;
		float m_QueuePresentTime;
		
		Ref< Texture2D > m_PinkTexture;
		Ref< TextureCube > m_PinkTextureCube;

		VkDescriptorSet m_RendererDescriptorSets[MAX_FRAMES_IN_FLIGHT];

		Ref< DescriptorPool > m_RendererDescriptorPools[ MAX_FRAMES_IN_FLIGHT ];

		// frame -> shader name -> set
		std::unordered_map< uint32_t, std::unordered_map< std::string, std::vector<VkWriteDescriptorSet>>> m_StorageBufferSets;

		// StorageBufferSet -> frame -> shader -> set
		//std::unordered_map < Ref<StorageBufferSet>, std::unordered_map<uint32_t, std::unordered_map< Ref<Shader>, std::vector<VkWriteDescriptorSet>>>> a;

	private:
		friend class VulkanContext;
	};
}