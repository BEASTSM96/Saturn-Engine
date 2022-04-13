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
#include "ImGuiVulkan.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

namespace Saturn {

	void ImGuiVulkan::BeginImGuiRender( VkCommandBuffer CommandBuffer )
	{
		m_CommandBuffer = CommandBuffer;

		//VkCommandBufferAllocateInfo AllocateInfo = /{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		//AllocateInfo.commandPool = VulkanContext::Get().GetCommandPool();
		//AllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		//AllocateInfo.commandBufferCount = 1;
		
		//VK_CHECK( vkAllocateCommandBuffers( VulkanContext::Get().GetDevice(), &AllocateInfo, &m_CommandBuffer ) );
		
		//SetDebugUtilsObjectName( "CommandBuffer:ImGui", ( uint64_t )m_CommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER );

		//VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		//BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		//BeginInfo.pInheritanceInfo = nullptr;
		
		//VK_CHECK( vkBeginCommandBuffer( m_CommandBuffer, &BeginInfo ) );

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( ( float )Window::Get().Width(), ( float )Window::Get().Height() );

		//m_ImGuiPass.BeginPass( m_CommandBuffer, VK_SUBPASS_CONTENTS_INLINE, ImageIndex );
	}

	void ImGuiVulkan::ImGuiRender()
	{
		ImGui::ShowDemoWindow();
	}

	void ImGuiVulkan::EndImGuiRender()
	{
		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), m_CommandBuffer );

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		//m_ImGuiPass.EndPass();

		//VK_CHECK( vkEndCommandBuffer( m_CommandBuffer ) );

		//VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		//VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		//SubmitInfo.commandBufferCount = 1;
		//SubmitInfo.pCommandBuffers = &m_CommandBuffer;
		//SubmitInfo.pWaitDstStageMask = &WaitStage;

		//VK_CHECK( vkQueueSubmit( VulkanContext::Get().GetGraphicsQueue(), 1, &SubmitInfo, VulkanContext::Get().GetCurrentFlightFence() ) );

		//VK_CHECK( vkQueueWaitIdle( VulkanContext::Get().GetGraphicsQueue() ) );

		//vkFreeCommandBuffers( VulkanContext::Get().GetDevice(), /VulkanContext::Get().GetCommandPool(), 1, &m_CommandBuffer );
	}

	void ImGuiVulkan::Init()
	{
		//m_ImGuiPass = Pass( VK_NULL_HANDLE, "ImGui Pass" );

		// Create ImGui Descriptor Pool
		std::vector< VkDescriptorPoolSize > PoolSizes;

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
		PoolSizes.push_back( { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } );
		
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		PoolCreateInfo.maxSets = 1000;
		PoolCreateInfo.poolSizeCount = std::size( pool_sizes );
		PoolCreateInfo.pPoolSizes = pool_sizes;
		
		VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_DescriptorPool ) );

		// Select Surface Format
		const VkFormat requestSurfaceImageFormat[] ={ VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

		ImGui_ImplVulkanH_Window SWD;

		ImGui_ImplVulkanH_Window* WD = &SWD;
		WD->Surface = VulkanContext::Get().GetSurface();

		ImGui_ImplVulkanH_SelectSurfaceFormat( VulkanContext::Get().GetPhysicalDevice(), WD->Surface, requestSurfaceImageFormat, IM_ARRAYSIZE( requestSurfaceImageFormat ), requestSurfaceColorSpace );
		
		VkPresentModeKHR PresentMode[] ={ VK_PRESENT_MODE_FIFO_KHR };
		
		WD->PresentMode = ImGui_ImplVulkanH_SelectPresentMode( VulkanContext::Get().GetPhysicalDevice(), WD->Surface, &PresentMode[0], IM_ARRAYSIZE( PresentMode ) );
		
		ImGui_ImplVulkanH_CreateOrResizeWindow( 
			VulkanContext::Get().GetInstance(), 
			VulkanContext::Get().GetPhysicalDevice(), 
			VulkanContext::Get().GetDevice(), 
			WD, 
			VulkanContext::Get().GetQueueFamilyIndices().GraphicsFamily.value(), 
			nullptr, 
			Window::Get().Width(), 
			Window::Get().Height(), 2 );
	}

	void ImGuiVulkan::CreatePipeline()
	{

	}

	void ImGuiVulkan::Terminate()
	{
		if( m_DescriptorPool )
			vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_DescriptorPool, nullptr );

		m_DescriptorPool = nullptr;

		m_ImGuiPass.Terminate();

		ImGui_ImplVulkan_Shutdown();
	}
}