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

	static VkFormat VulkanFormat( ImageFormat format )
	{
		switch( format )
		{
			case Saturn::ImageFormat::RGBA8:
				return VK_FORMAT_R8G8B8A8_UNORM;

			case Saturn::ImageFormat::RGBA16F:
				return VK_FORMAT_R16G16B16A16_UNORM;

			case Saturn::ImageFormat::RGBA32F:
				return VK_FORMAT_R32G32B32A32_SFLOAT;

			case ImageFormat::BGRA8:
				return VK_FORMAT_B8G8R8A8_UNORM;

			case Saturn::ImageFormat::DEPTH24STENCIL8:
			case Saturn::ImageFormat::DEPTH32F:
				return VK_FORMAT_D32_SFLOAT;
		}

		return VK_FORMAT_UNDEFINED;
	}

	static bool IsColorFormat( ImageFormat format )
	{
		switch( format )
		{
			case Saturn::ImageFormat::RGBA8:
			case Saturn::ImageFormat::RGBA16F:
			case Saturn::ImageFormat::RGBA32F:
			case Saturn::ImageFormat::RGB32F:
			case Saturn::ImageFormat::BGRA8:
				return true;
		}

		return false;
	}

	static bool IsDepthFormat( ImageFormat format )
	{
		switch( format )
		{
			case Saturn::ImageFormat::DEPTH32F:
			case Saturn::ImageFormat::DEPTH24STENCIL8:
				return true;
		}

		return false;
	}

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

		int i = 0;
		for ( auto attachment : m_PassSpec.Attachments )
		{
			if( IsColorFormat( attachment ) ) 
			{
				VkAttachmentReference Attachment = { .attachment = ( uint32_t ) i, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

				m_ColorAttacments.push_back( Attachment );
			}
			else
				m_DepthAttacment = { .attachment = ( uint32_t ) i, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

			i++;
		}

		if ( m_DepthAttacment.layout == VK_IMAGE_LAYOUT_UNDEFINED || m_DepthAttacment.layout == VK_IMAGE_LAYOUT_MAX_ENUM )
		{
			SAT_CORE_ASSERT( false, "Render pass must have a depth attachment!" );
		}

		if( m_ColorAttacments.size() ) 
		{
			DefaultSubpass.pColorAttachments = m_ColorAttacments.data();
			DefaultSubpass.colorAttachmentCount = m_ColorAttacments.size();
		}

		DefaultSubpass.pDepthStencilAttachment = &m_DepthAttacment;

		DefaultSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		// Dependencies
		std::vector< VkSubpassDependency > Dependencies;

		Dependencies.push_back( {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		} );

		Dependencies.push_back( {
			.srcSubpass = 0,
			.dstSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		} );

		// Attachment Description

		std::vector< VkAttachmentDescription > AttachmentDescriptions;
		
		for ( auto attachment : m_PassSpec.Attachments )
		{
			if( m_PassSpec.IsSwapchainTarget )
			{
				AttachmentDescriptions.push_back(
				{
					.flags = 0,
					.format = VulkanFormat( attachment ),
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = IsColorFormat( attachment ) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR  : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				} );
			}
			else
			{
				AttachmentDescriptions.push_back(
				{
					.flags = 0,
					.format = VulkanFormat( attachment ),
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = IsColorFormat( attachment ) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
				} );
			}
		}

		VkRenderPassCreateInfo RenderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		RenderPassCreateInfo.pAttachments = AttachmentDescriptions.data();
		RenderPassCreateInfo.attachmentCount = AttachmentDescriptions.size();
		RenderPassCreateInfo.pSubpasses = &DefaultSubpass;
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pDependencies = Dependencies.data();
		RenderPassCreateInfo.dependencyCount =Dependencies.size();

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