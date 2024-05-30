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
#include "DescriptorSet.h"

#include "VulkanContext.h"
#include "Renderer.h"

namespace Saturn {

	DescriptorPool::DescriptorPool( std::vector< VkDescriptorPoolSize > PoolSizes, uint32_t MaxSets )
	{
		VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.poolSizeCount = (uint32_t)PoolSizes.size();
		PoolCreateInfo.pPoolSizes = PoolSizes.data();
		PoolCreateInfo.maxSets = MaxSets;
		PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_Pool ) );
	}

	DescriptorPool::~DescriptorPool()
	{
		Terminate();
	}

	void DescriptorPool::Terminate()
	{
		if( m_Pool )
			vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_Pool, nullptr );

		m_Pool = nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	DescriptorSet::DescriptorSet( DescriptorSetSpecification Spec )
		: m_Specification( Spec )
	{
		Allocate();
	}

	void DescriptorSet::Allocate()
	{
		VkDescriptorSetAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		AllocateInfo.descriptorPool = m_Specification.Pool->GetVulkanPool();
		AllocateInfo.descriptorSetCount = 1;
		AllocateInfo.pSetLayouts = &m_Specification.Layout;

		VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &AllocateInfo, &m_Set ) );
	}

	void DescriptorSet::Terminate()
	{
		if( m_Set )
			vkFreeDescriptorSets( VulkanContext::Get().GetDevice(), m_Specification.Pool->GetVulkanPool(), 1, &m_Set );

		m_Set = nullptr;
		m_Specification = {};
	}

	DescriptorSet::~DescriptorSet()
	{
		Terminate();
	}

	void DescriptorSet::WriteDescriptor( const VkDescriptorBufferInfo& BufferInfo, VkDescriptorType descriptorType )
	{
		VkWriteDescriptorSet WriteDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		WriteDescriptorSet.dstSet = m_Set;
		WriteDescriptorSet.dstBinding = 0;
		WriteDescriptorSet.dstArrayElement = 0;
		WriteDescriptorSet.descriptorType = descriptorType;
		WriteDescriptorSet.descriptorCount = 1;
		WriteDescriptorSet.pBufferInfo = &BufferInfo;
		WriteDescriptorSet.pImageInfo = nullptr;

		m_PendingWriteDescriptors[ WriteDescriptorSet.dstBinding ] = WriteDescriptorSet;
	}

	void DescriptorSet::WriteDescriptor( const VkDescriptorImageInfo& ImageInfo, VkDescriptorType descriptorType )
	{
		VkWriteDescriptorSet WriteDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		WriteDescriptorSet.dstSet = m_Set;
		WriteDescriptorSet.dstBinding = 0;
		WriteDescriptorSet.dstArrayElement = 0;
		WriteDescriptorSet.descriptorType = descriptorType;
		WriteDescriptorSet.descriptorCount = 1;
		WriteDescriptorSet.pBufferInfo = nullptr;
		WriteDescriptorSet.pImageInfo = &ImageInfo;

		m_PendingWriteDescriptors[ WriteDescriptorSet.dstBinding ] = WriteDescriptorSet;
	}

	void DescriptorSet::WriteDescriptor( const VkWriteDescriptorSet& rWDS )
	{
		m_PendingWriteDescriptors[ rWDS.dstBinding ] = rWDS;
	}

	void DescriptorSet::WriteDescriptor( std::vector< VkWriteDescriptorSet > WriteDescriptorSets )
	{
		for( const auto& rWds : WriteDescriptorSets )
		{
			m_PendingWriteDescriptors[ rWds.dstBinding ] = rWds;
		}
	}

	void DescriptorSet::Bind( VkCommandBuffer CommandBuffer, VkPipelineLayout PipelineLayout, VkPipelineBindPoint BindPoint )
	{
		UpdateResidentWriteDescriptors();

		vkCmdBindDescriptorSets( CommandBuffer, BindPoint, PipelineLayout, 0, 1, &m_Set, 0, nullptr );
	}

	void DescriptorSet::UpdateResidentWriteDescriptors()
	{
		VkDescriptorImageInfo ImageInfo = {};
		ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		auto PinkTexture = Renderer::Get().GetPinkTexture();
		ImageInfo.imageView = PinkTexture->GetImageView();
		ImageInfo.sampler = PinkTexture->GetSampler();

		int i = 0;
		for( auto& [ binding, rWds ] : m_PendingWriteDescriptors ) 
		{
			// Size mismatch, immediately fall back to pending wds.
			if( m_ResidentWriteDescriptors.size() != m_PendingWriteDescriptors.size() )
			{
				if( rWds.pImageInfo )
				{
					rWds.pImageInfo = &ImageInfo;
				}

				vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &rWds, 0, nullptr );
				m_ResidentWriteDescriptors[ binding ] = rWds;

				i++;
				continue;
			}

			/*
			if( CompareWds( rWds, m_ResidentWriteDescriptors[ i ] ) ) 
			{
				vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &rWds, 0, nullptr );
				m_ResidentWriteDescriptors[ binding ] = rWds;

				i++;
				continue;
			}
			*/
		
			i++;
		}

		m_PendingWriteDescriptors.clear();
	}

	bool DescriptorSet::CompareWds( const VkWriteDescriptorSet& rSource, const VkWriteDescriptorSet& rTarget )
	{
		// Compare if a set has been changed.
		if( rSource.dstSet != rTarget.dstSet )
			return true;

		// Compare image data
		if( rSource.pImageInfo )
		{
			auto& rSrcImageInfo = rSource.pImageInfo;
			auto& rTargetImageInfo = rTarget.pImageInfo;

			if(    rSrcImageInfo->imageLayout != rTargetImageInfo->imageLayout 
				|| rSrcImageInfo->imageView != rTargetImageInfo->imageView
				|| rSrcImageInfo->sampler != rTargetImageInfo->sampler )
			{
				return true;
			}
		}

		if( rSource.pBufferInfo )
		{
			auto& rSrcBufferInfo = rSource.pBufferInfo;
			auto& rTargetBufferInfo = rTarget.pBufferInfo;

			if(    rSrcBufferInfo->buffer != rTargetBufferInfo->buffer
				|| rSrcBufferInfo->range != rTargetBufferInfo->range
				|| rSrcBufferInfo->offset != rTargetBufferInfo->offset )
			{
				return true;
			}
		}

		return false;
	}

}