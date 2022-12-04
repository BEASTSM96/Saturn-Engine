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

#include "Image2D.h"

namespace Saturn {
	
	struct PassSpecification
	{
		std::vector< ImageFormat > Attachments = {};
		std::string Name = "";
		bool IsSwapchainTarget = false;

		bool LoadColor = false;
		bool LoadDepth = false;
	};

	class Pass : public CountedObj
	{
	public:
		 Pass() { }
		 Pass( PassSpecification PassSpec );
		~Pass();
		
		void Terminate();
		void Recreate();

		void BeginPass( VkCommandBuffer CommandBuffer, VkFramebuffer Framebuffer, VkExtent2D Extent );
		void EndPass();
		
		operator VkRenderPass() { return m_Pass; }
		VkRenderPass GetVulkanPass() { return m_Pass; }

		size_t GetColorAttachmetSize() { return m_ColorAttacments.size(); }

		std::string& GetName() { return m_PassSpec.Name; }
		const std::string& GetName() const { return m_PassSpec.Name; }

	private:

		void Create( PassSpecification PassSpec );

	private:
		PassSpecification m_PassSpec;

		VkAttachmentReference m_DepthAttacment = {};
		std::vector< VkAttachmentReference > m_ColorAttacments;

		std::vector<VkClearValue> m_ClearValues;

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkRenderPass m_Pass = VK_NULL_HANDLE;
	};
}