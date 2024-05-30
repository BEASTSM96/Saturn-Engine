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
#include "DescriptorSetManager.h"

#include "Renderer.h"
#include "Material.h"
#include "DescriptorSet.h"

namespace Saturn {

	DescriptorSetManager::DescriptorSetManager()
	{
		std::vector<VkDescriptorPoolSize> PoolSizes;

		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 } );
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 } );

		for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			m_DescriptorPools[ i ] = Ref<DescriptorPool>::Create( PoolSizes, 100000 );
		}
	}

	DescriptorSetManager::~DescriptorSetManager()
	{
		m_CurrentDescriptorPool = nullptr;

		for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			m_DescriptorPools[ i ] = nullptr;
		}
	}

	void DescriptorSetManager::InitialiseForNextFrame( uint32_t frameIndex )
	{
		m_CurrentDescriptorPool = m_DescriptorPools[ frameIndex ];
	}

	Ref<DescriptorSet> DescriptorSetManager::AllocateDescriptorSet( uint32_t set, VkDescriptorSetLayout layout, Ref<Material> material )
	{
		uint32_t frameIndex = Renderer::Get().GetCurrentFrame();

		// Find or Allocate
		if( auto ds = FindSet( set, frameIndex, material ) ) 
		{
			return ds;
		}

		auto ds = AllocateInternal( set, layout, material );
		return ds;
	}

	Ref<DescriptorSet> DescriptorSetManager::FindSet( uint32_t set, uint32_t frameIndex, Ref<Material> material /*= nullptr */ )
	{
		if( material )
		{
			auto Itr = m_MaterialDescriptorSets.find( material->GetName() );

			if( Itr != m_MaterialDescriptorSets.end() )
			{
				// If the material exists we then move on to the frames in flight map.
				auto& rFrameMap = Itr->second;

				int i = 0;
				for( const auto& rFrameSets : rFrameMap )
				{
					for( const auto& rSet : rFrameSets )
					{
						if( rSet->GetSetIndex() == set && i == frameIndex )
							return rSet;
					}

					i++;
				}
			}
		}
		else
		{
			// Check if our set exists.
			auto Itr = m_DescriptorSets.find( set );

			if( Itr != m_DescriptorSets.end() )
			{
				auto& rFrameMap = Itr->second;

				int i = 0;
				for( const auto& rFrameSets : rFrameMap )
				{
					for( const auto& rSet : rFrameSets )
					{
						if( rSet->GetSetIndex() == set && i == frameIndex )
							return rSet;
					}

					i++;
				}
			}
		}

		return nullptr;
	}

	void DescriptorSetManager::BindDescriptorSets( VkCommandBuffer CommandBuffer, VkPipelineBindPoint BindPoint, VkPipelineLayout Layout, uint32_t firstSet, std::vector<Ref<DescriptorSet>>& rDescriptorSets )
	{
		std::vector< VkDescriptorSet > descriptorSets;

		for( auto& rSet : rDescriptorSets )
		{
			rSet->UpdateResidentWriteDescriptors();
			descriptorSets.push_back( rSet->GetVulkanSet() );
		}

		vkCmdBindDescriptorSets( CommandBuffer, BindPoint, Layout, firstSet, descriptorSets.size(), descriptorSets.data(), 0, nullptr );
	}

	Ref<DescriptorSet> DescriptorSetManager::AllocateInternal( uint32_t set, VkDescriptorSetLayout layout, Ref<Material> material /*= nullptr */ )
	{
		Ref<DescriptorSet> currentFrameDS = nullptr;

		for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			DescriptorSetSpecification spec{};
			spec.SetIndex = set;
			spec.Layout = layout;
			spec.Pool = m_DescriptorPools[ i ];

			Ref<DescriptorSet> ds = Ref<DescriptorSet>::Create( spec );

			if( material )
			{
				ResizeAndResideSet( material->GetName(), ds, i );
			}
			else
			{
				auto& rFrameMap = m_DescriptorSets[ set ];

				if( rFrameMap.size() != MAX_FRAMES_IN_FLIGHT )
					rFrameMap.resize( MAX_FRAMES_IN_FLIGHT );

				rFrameMap[ i ].push_back( ds );
			}

			if( Renderer::Get().GetCurrentFrame() == i )
				currentFrameDS = ds;
		}

		m_TotalAllocatedSets += MAX_FRAMES_IN_FLIGHT;
		
		return currentFrameDS;
	}

	void DescriptorSetManager::ResizeAndResideSet( const std::string& rName, const Ref<DescriptorSet>& rSet, uint32_t frameIndex )
	{
		auto& frameMap = m_MaterialDescriptorSets[ rName ];

		if( frameMap.size() != MAX_FRAMES_IN_FLIGHT )
			frameMap.resize( MAX_FRAMES_IN_FLIGHT );

		// Reside new set for each frame in flight.
		frameMap[ frameIndex ].push_back( rSet );
	}

	void DescriptorSetManager::WriteDescriptor( VkWriteDescriptorSet& rWriteDescriptorSet, Ref<DescriptorSet>& rSet )
	{
		vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &rWriteDescriptorSet, 0, nullptr );
	}
}