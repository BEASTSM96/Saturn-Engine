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

namespace Saturn {

	inline void SetDebugUtilsObjectName( VkDebugUtilsObjectNameInfoEXT* pInfo )
	{
		PFN_vkSetDebugUtilsObjectNameEXT Function = ( PFN_vkSetDebugUtilsObjectNameEXT )vkGetInstanceProcAddr( VulkanContext::Get().GetInstance(), "vkSetDebugUtilsObjectNameEXT" );

		if ( Function )
		{
			Function( VulkanContext::Get().GetDevice(), pInfo );
		}
	}

	inline void SetDebugUtilsObjectName( std::string Name, uint64_t Handle, VkObjectType ObjectType )
	{
		PFN_vkSetDebugUtilsObjectNameEXT Function = ( PFN_vkSetDebugUtilsObjectNameEXT )vkGetInstanceProcAddr( VulkanContext::Get().GetInstance(), "vkSetDebugUtilsObjectNameEXT" );

		VkDebugUtilsObjectNameInfoEXT Info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		Info.objectHandle = ( uint64_t )Handle;
		Info.objectType = ObjectType;
		Info.pObjectName = Name.c_str();

		if( Function )
		{
			Function( VulkanContext::Get().GetDevice(), &Info );
		}
	}

	inline void SetDebugUtilsObjectName( const char* pName, uint64_t Handle, VkObjectType ObjectType )
	{
		PFN_vkSetDebugUtilsObjectNameEXT Function = ( PFN_vkSetDebugUtilsObjectNameEXT )vkGetInstanceProcAddr( VulkanContext::Get().GetInstance(), "vkSetDebugUtilsObjectNameEXT" );

		VkDebugUtilsObjectNameInfoEXT Info ={ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
		Info.objectHandle = ( uint64_t )Handle;
		Info.objectType = ObjectType;
		Info.pObjectName = pName;

		if( Function )
		{
			Function( VulkanContext::Get().GetDevice(), &Info );
		}
	}
}