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

	Pass::Pass( VkCommandBuffer CommandBuffer, std::string Name )
	{
		m_CommandBuffer = CommandBuffer;
		m_Name = Name;
		
		std::vector< VkAttachmentDescription > Attachments;

		Attachments.push_back( {
			.flags = 0,
			.format = VulkanContext::Get().GetSurfaceFormat().format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		} );

		Attachments.push_back( {
			.format = VulkanContext::Get().FindDepthFormat(),
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		} );
		
		std::vector< VkAttachmentReference > AttactmentRefs ={};
		
		AttactmentRefs.push_back( {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		} );

		AttactmentRefs.push_back( {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		} );

		VkSubpassDescription DefaultSubpass ={};
		DefaultSubpass.pColorAttachments = &AttactmentRefs[ 0 ];
		DefaultSubpass.colorAttachmentCount = 1;
		DefaultSubpass.pDepthStencilAttachment = &AttactmentRefs[ 1 ];
		DefaultSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		
		VkSubpassDependency Dependency = {};
		Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		Dependency.dstSubpass = 0;
		Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo RenderPassCreateInfo ={ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		RenderPassCreateInfo.pAttachments = Attachments.data();
		RenderPassCreateInfo.attachmentCount = Attachments.size();
		RenderPassCreateInfo.pSubpasses = &DefaultSubpass;
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pDependencies = &Dependency;
		RenderPassCreateInfo.dependencyCount = 1;
		
		VK_CHECK( vkCreateRenderPass( VulkanContext::Get().GetDevice(), &RenderPassCreateInfo, nullptr, &m_Pass ) );

		SetDebugUtilsObjectName( m_Name, ( uint64_t )m_Pass, VK_OBJECT_TYPE_RENDER_PASS );
	}

	Pass::~Pass()
	{
	}

	void Pass::Terminate()
	{
		if( m_Pass )
			vkDestroyRenderPass( VulkanContext::Get().GetDevice(), m_Pass, nullptr );

		m_Pass = nullptr;
	}
	
	void Pass::Recreate( VkCommandBuffer CommandBuffer /*=nullptr*/ )
	{
		vkDestroyRenderPass( VulkanContext::Get().GetDevice(), m_Pass, nullptr );

		m_CommandBuffer = CommandBuffer;
		m_Name = m_Name;
		
		std::vector< VkAttachmentDescription > Attachments;

		Attachments.push_back( {
			.flags = 0,
			.format = VulkanContext::Get().GetSurfaceFormat().format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		} );

		Attachments.push_back( {
			.flags = 0,
			.format = VulkanContext::Get().FindDepthFormat(),
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		} );

		std::vector< VkAttachmentReference > AttactmentRefs ={};

		AttactmentRefs.push_back( {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		} );

		AttactmentRefs.push_back( {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		} );

		VkSubpassDescription DefaultSubpass ={};
		DefaultSubpass.pColorAttachments = &AttactmentRefs[ 0 ];
		DefaultSubpass.colorAttachmentCount = 1;
		DefaultSubpass.pDepthStencilAttachment = &AttactmentRefs[ 1 ];
		DefaultSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkSubpassDependency Dependency ={};
		Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		Dependency.dstSubpass = 0;
		Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo RenderPassCreateInfo ={ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		RenderPassCreateInfo.pAttachments = Attachments.data();
		RenderPassCreateInfo.attachmentCount = Attachments.size();
		RenderPassCreateInfo.pSubpasses = &DefaultSubpass;
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pDependencies = &Dependency;
		RenderPassCreateInfo.dependencyCount = 1;

		VK_CHECK( vkCreateRenderPass( VulkanContext::Get().GetDevice(), &RenderPassCreateInfo, nullptr, &m_Pass ) );
		SetDebugUtilsObjectName( m_Name, ( uint64_t )m_Pass, VK_OBJECT_TYPE_RENDER_PASS );
	}

	void Pass::BeginPass( VkCommandBuffer CommandBuffer /* = nullptr */, VkSubpassContents Contents /* = VK_SUBPASS_CONTENTS_INLINE*/, uint32_t ImageIndex /* = 0 */ )
	{
		m_CommandBuffer = CommandBuffer;

		std::array<VkClearValue, 2> ClearColors{};
		ClearColors[ 0 ].color ={ { 0.0f, 0.0f, 0.0f, 1.0f } };
		ClearColors[ 1 ].depthStencil ={ 1.0f, 0 };

		VkRenderPassBeginInfo RenderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		RenderPassBeginInfo.renderPass = m_Pass;
		RenderPassBeginInfo.pClearValues = ClearColors.data();
		RenderPassBeginInfo.clearValueCount = ClearColors.size();
		RenderPassBeginInfo.framebuffer = VulkanContext::Get().GetSwapchain().GetFramebuffers()[ ImageIndex ];
		
		VkExtent2D extent;
		Window::Get().GetSize( &extent.width, &extent.height );

		RenderPassBeginInfo.renderArea.extent = extent;
		
		vkCmdBeginRenderPass( m_CommandBuffer, &RenderPassBeginInfo, Contents );
	}

	void Pass::EndPass()
	{
		vkCmdEndRenderPass( m_CommandBuffer );
	}

}