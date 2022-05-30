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

#include "sppch.h"
#include "Pass.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

namespace Saturn {

	Pass::Pass( PassSpecification PassSpec )
	{
		Create( PassSpec );
	}

	Pass::~Pass()
	{
	}

	void Pass::Create( PassSpecification PassSpec )
	{
		m_PassSpec = PassSpec;

		VkSubpassDescription DefaultSubpass = {};
		DefaultSubpass.pColorAttachments = &m_PassSpec.ColorAttachmentRef;
		DefaultSubpass.colorAttachmentCount = 1;
		DefaultSubpass.pDepthStencilAttachment = &m_PassSpec.DepthAttachmentRef;
		DefaultSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkSubpassDependency Dependency = {};
		Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		Dependency.dstSubpass = 0;
		Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo RenderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		RenderPassCreateInfo.pAttachments = m_PassSpec.Attachments.data();
		RenderPassCreateInfo.attachmentCount = m_PassSpec.Attachments.size();
		RenderPassCreateInfo.pSubpasses = &DefaultSubpass;
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pDependencies = &Dependency;
		RenderPassCreateInfo.dependencyCount = 1;

		VK_CHECK( vkCreateRenderPass( VulkanContext::Get().GetDevice(), &RenderPassCreateInfo, nullptr, &m_Pass ) );
		SetDebugUtilsObjectName( m_PassSpec.Name, ( uint64_t ) m_Pass, VK_OBJECT_TYPE_RENDER_PASS );
		
	}

	void Pass::Terminate()
	{
		if( m_Pass )
			vkDestroyRenderPass( VulkanContext::Get().GetDevice(), m_Pass, nullptr );

		m_Pass = nullptr;
	}
	
	void Pass::Recreate()
	{
		Terminate();

		Create( m_PassSpec );
	}

	void Pass::BeginPass( VkCommandBuffer CommandBuffer, VkFramebuffer Framebuffer, VkExtent2D Extent )
	{
		m_CommandBuffer = CommandBuffer;
		
		std::array<VkClearValue, 2> ClearColors{};
		ClearColors[ 0 ].color ={ { 0.0f, 0.0f, 0.0f, 1.0f } };
		ClearColors[ 1 ].depthStencil ={ 1.0f, 0 };

		VkRenderPassBeginInfo RenderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		RenderPassBeginInfo.renderPass = m_Pass;
		RenderPassBeginInfo.framebuffer = Framebuffer;
		RenderPassBeginInfo.renderArea.extent = Extent;
		RenderPassBeginInfo.pClearValues = ClearColors.data();
		RenderPassBeginInfo.clearValueCount = ClearColors.size();
		
		vkCmdBeginRenderPass( m_CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
	}

	void Pass::EndPass()
	{
		vkCmdEndRenderPass( m_CommandBuffer );
	}
}