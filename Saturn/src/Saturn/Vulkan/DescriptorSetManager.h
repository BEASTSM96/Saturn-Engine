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

#include "Saturn/Core/Ref.h"

#include <unordered_map>
#include <vulkan.h>

namespace Saturn {

	class DescriptorSet;
	class DescriptorPool;
	class Material;

	class DescriptorSetManager : public RefTarget
	{
		// MATERIAL (NAME) -> FRAMES IN FLIGHT -> SETS
		using MaterialDSMap = std::unordered_map<std::string, std::vector<std::vector<Ref<DescriptorSet>>>>;
		
		// Materialess sets
		// SET -> FRAMES IN FLIGHT -> SETS
		using DSMap = std::unordered_map<uint32_t, std::vector<std::vector<Ref<DescriptorSet>>>>;
	public:
		DescriptorSetManager();
		~DescriptorSetManager();

		Ref<DescriptorSet> AllocateDescriptorSet( uint32_t set, VkDescriptorSetLayout layout, Ref<Material> material = nullptr );

		void InitialiseForNextFrame( uint32_t frameIndex );

		void WriteDescriptor( VkWriteDescriptorSet& rWriteDescriptorSet, Ref<DescriptorSet>& rSet );

		Ref<DescriptorSet> FindSet( uint32_t set, uint32_t frameIndex, Ref<Material> material = nullptr );
	private:
		void ResizeAndResideSet( const std::string& rName, const Ref<DescriptorSet>& rSet, uint32_t frameIndex );
		Ref<DescriptorSet> AllocateInternal( uint32_t set, VkDescriptorSetLayout layout, Ref<Material> material = nullptr );

	private:
		uint32_t m_TotalAllocatedSets = 0;

		Ref<DescriptorPool> m_DescriptorPools[ MAX_FRAMES_IN_FLIGHT ];
		Ref<DescriptorPool> m_CurrentDescriptorPool = nullptr;

		MaterialDSMap m_MaterialDescriptorSets;
		DSMap m_DescriptorSets;
	};
}