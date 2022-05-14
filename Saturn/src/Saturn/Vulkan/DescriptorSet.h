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
#include <vector>

namespace Saturn {

	class DescriptorPool
	{
	public:
		DescriptorPool() {}
		DescriptorPool( std::vector< VkDescriptorPoolSize > PoolSizes, uint32_t MaxSets );
		~DescriptorPool();
		
		operator VkDescriptorPool&() { return m_Pool; }
		operator const VkDescriptorPool&() const { return m_Pool; }

		// Copy assignment.
		DescriptorPool& operator=( const DescriptorPool& other ) 
		{
			if( this == &other )
				return *this;
			
			m_Pool = other.m_Pool;

			return *this;
		}

		// Move assignment.
		DescriptorPool& operator=( DescriptorPool&& other ) noexcept
		{
			if( this == &other )
				return *this;

			m_Pool = other.m_Pool;

			other.m_Pool = nullptr;

			return *this;
		}

		// Copy constructor.
		DescriptorPool( const DescriptorPool& other )
		{
			m_Pool = other.m_Pool;	
		}

		// Move constructor.
		DescriptorPool( DescriptorPool&& other ) noexcept
		{
			m_Pool = other.m_Pool;
			other.m_Pool = nullptr;
		}

	private:
		VkDescriptorPool m_Pool = nullptr;
	};	
	
	enum class DescriptorType
	{
		// Should match with vulkan's VkDescriptorType enum
		UNKNOWN = -1,
		SAMPLER = 0,
		COMBINED_IMAGE_SAMPLER = 1,
		SAMPLED_IMAGE = 2,
		STORAGE_IMAGE = 3,
		UNIFORM_BUFFER = 6,
		STORAGE_BUFFER = 7,
	};
	
	struct DescriptorSetLayout
	{
		void Create();

		VkDescriptorSetLayout VulkanLayout = nullptr;
		std::vector< VkDescriptorSetLayoutBinding > Bindings;
	};

	struct DescriptorSetSpecification
	{		
		DescriptorSetSpecification() {}
		~DescriptorSetSpecification() {}
		
		Ref< DescriptorPool > Pool;
		DescriptorSetLayout Layout;
	};

	class DescriptorSet
	{
	public:
		DescriptorSet() {}
		DescriptorSet( DescriptorSetSpecification Spec );
		~DescriptorSet();

		void Terminate();

		void Write( VkDescriptorBufferInfo BufferInfo, VkDescriptorImageInfo ImageInfo );
		void Write( std::vector< VkWriteDescriptorSet > WriteDescriptorSets );


		void Bind( VkCommandBuffer CommandBuffer, VkPipelineLayout PipelineLayout );
		
		bool operator == ( const DescriptorSet& other ) const
		{
			return ( m_Set == other.m_Set );
		}
		
		VkDescriptorSet GetVulkanSet() { return m_Set; }

		operator VkDescriptorSet() { return m_Set; }
		operator VkDescriptorSet&() { return m_Set; }
		
	private:
		
		void Allocate();

	private:

		VkDescriptorSet m_Set = nullptr;
		
		DescriptorSetSpecification m_Specification = {};
	};
}