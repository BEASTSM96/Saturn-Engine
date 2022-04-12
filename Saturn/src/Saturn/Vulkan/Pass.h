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
#include "Saturn/Core/Base.h"

namespace Saturn {
	
	class Pass
	{
	public:
		 Pass() { }
		 Pass( VkCommandBuffer CommandBuffer, std::string Name );
		~Pass();
		
		void Terminate();

		void Recreate( VkCommandBuffer CommandBuffer = nullptr );

		VkRenderPass& GetRenderPass() { return m_Pass; }

		void BeginPass( VkCommandBuffer CommandBuffer = nullptr, VkSubpassContents Contents = VK_SUBPASS_CONTENTS_INLINE, uint32_t ImageIndex = 0 );
		void EndPass();

	private:
		std::string m_Name = "";
		
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkRenderPass m_Pass = VK_NULL_HANDLE;
	};
}