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
#include "Texture.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

namespace Saturn {

	void ImGuiVulkan::Init()
	{
		ImGui_ImplGlfw_InitForVulkan( ( GLFWwindow* )Window::Get().NativeWindow(), true );
		
		// Create ImGui Descriptor Pool
		{
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


			VkDescriptorPoolCreateInfo PoolCreateInfo ={ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
			PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			PoolCreateInfo.maxSets = 1000;
			PoolCreateInfo.poolSizeCount = PoolSizes.size();
			PoolCreateInfo.pPoolSizes = PoolSizes.data();

			VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_DescriptorPool ) );
		}

		ImGui_ImplVulkan_InitInfo ImGuiInitInfo ={};
		ImGuiInitInfo.Instance = VulkanContext::Get().GetInstance();
		ImGuiInitInfo.PhysicalDevice = VulkanContext::Get().GetPhysicalDevice();
		ImGuiInitInfo.Device = VulkanContext::Get().GetDevice();
		ImGuiInitInfo.Queue = VulkanContext::Get().GetGraphicsQueue();
		ImGuiInitInfo.DescriptorPool = m_DescriptorPool;
		ImGuiInitInfo.MinImageCount = 2;
		ImGuiInitInfo.ImageCount = 2;
		ImGuiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGuiInitInfo.CheckVkResultFn = _VkCheckResult;

		ImGui_ImplVulkan_Init( &ImGuiInitInfo, VulkanContext::Get().GetRenderPass().GetRenderPass() );

		VkCommandBuffer CommandBuffer;
		CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		{
			ImGui_ImplVulkan_CreateFontsTexture( CommandBuffer );
		}

		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );

		ImGui_ImplVulkan_DestroyFontUploadObjects();

		m_OffscreenID = ImGui_ImplVulkan_AddTexture( VulkanContext::Get().GetOffscreenColorSampler(), VulkanContext::Get().GetOffscreenColorView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		m_pDockspace = new ImGuiDockspace();
	}
	
	void ImGuiVulkan::Terminate()
	{
		delete m_pDockspace;

		if( m_DescriptorPool )
			vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_DescriptorPool, nullptr );

		m_DescriptorPool = nullptr;


		ImGui_ImplVulkan_Shutdown();
	}

	void ImGuiVulkan::BeginImGuiRender( VkCommandBuffer CommandBuffer )
	{
		m_CommandBuffer = CommandBuffer;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( ( float )Window::Get().Width(), ( float )Window::Get().Height() );
	}

	void ImGuiVulkan::ImGuiRender()
	{
		m_pDockspace->Draw();
	}

	void ImGuiVulkan::EndImGuiRender()
	{
		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), m_CommandBuffer );

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void ImGuiVulkan::RecreateImages()
	{
		m_OffscreenID = ImGui_ImplVulkan_AddTexture( VulkanContext::Get().GetOffscreenColorSampler(), VulkanContext::Get().GetOffscreenColorView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
	}

	void ImGuiVulkan::CreatePipeline()
	{
	}
}