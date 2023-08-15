/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "Layer.h"

#include "Saturn/Core/Renderer/RenderThread.h"

#include "Saturn/ImGui/Styles.h"
#include "Saturn/Vulkan/VulkanContext.h"
#include "Saturn/Vulkan/Renderer.h"
#include "Saturn/Vulkan/VulkanDebug.h"
#include "Window.h"

#include "ImGuizmo/ImGuizmo.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

namespace Saturn {
	
	Layer::Layer()
	{

	}

	Layer::~Layer()
	{

	}

	//////////////////////////////////////////////////////////////////////////

	ImGuiLayer::ImGuiLayer()
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach( void )
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		// ImGui Theme

		ImGuiIO& io = ImGui::GetIO(); ( void ) io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			style.WindowRounding = 0.0f;
			style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
		}

		auto* pFont = io.Fonts->AddFontFromFileTTF( "content\\Fonts\\NotoSans-Regular.ttf", 18.0f );
		io.FontDefault = io.Fonts->Fonts.back();
		
		auto* pBold = io.Fonts->AddFontFromFileTTF( "content\\Fonts\\NotoSans-Bold.ttf", 18.0f );
		auto* pItalic = io.Fonts->AddFontFromFileTTF( "content\\Fonts\\NotoSans-Italic.ttf", 18.0f );

		Styles::Dark();

		ImGui_ImplGlfw_InitForVulkan( ( GLFWwindow* ) Window::Get().NativeWindow(), true );

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

			VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
			PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			PoolCreateInfo.maxSets = 1000;
			PoolCreateInfo.poolSizeCount = ( uint32_t ) PoolSizes.size();
			PoolCreateInfo.pPoolSizes = PoolSizes.data();

			VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_DescriptorPool ) );
		}
		
		ImGui_ImplVulkan_InitInfo ImGuiInitInfo = {};
		ImGuiInitInfo.Instance = VulkanContext::Get().GetInstance();
		ImGuiInitInfo.PhysicalDevice = VulkanContext::Get().GetPhysicalDevice();
		ImGuiInitInfo.Device = VulkanContext::Get().GetDevice();
		ImGuiInitInfo.Queue = VulkanContext::Get().GetGraphicsQueue();
		ImGuiInitInfo.DescriptorPool = m_DescriptorPool;
		ImGuiInitInfo.MinImageCount = 2;
		ImGuiInitInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
		ImGuiInitInfo.MSAASamples = VulkanContext::Get().GetMaxUsableMSAASamples();

		ImGuiInitInfo.CheckVkResultFn = _VkCheckResult;

		ImGui_ImplVulkan_Init( &ImGuiInitInfo, VulkanContext::Get().GetDefaultVulkanPass() );
		
		VkCommandBuffer CommandBuffer;
		CommandBuffer = VulkanContext::Get().BeginSingleTimeCommands();

		{
			ImGui_ImplVulkan_CreateFontsTexture( CommandBuffer );
		}
		
		VulkanContext::Get().EndSingleTimeCommands( CommandBuffer );

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void ImGuiLayer::OnDetach( void )
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_DescriptorPool, nullptr );

		m_DescriptorPool = nullptr;
	}

	void ImGuiLayer::Begin()
	{
		if( Application::Get().HasFlag( ApplicationFlags::GameDist ) )
			return;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( ( float ) Window::Get().Width(), ( float ) Window::Get().Height() );
	}

	void ImGuiLayer::End( VkCommandBuffer CommandBuffer )
	{
		if( Application::Get().HasFlag( ApplicationFlags::GameDist ) )
			return;

		Swapchain& rSwapchain = VulkanContext::Get().GetSwapchain();

		ImGui::Render();

		std::vector<VkClearValue> ClearColor;
		ClearColor.push_back( { .color = { { 0.1f, 0.1f, 0.1f, 1.0f } } } );
		if( VulkanContext::Get().GetMaxUsableMSAASamples() > VK_SAMPLE_COUNT_1_BIT )
			ClearColor.push_back( { .color = { { 0.1f, 0.1f, 0.1f, 1.0f } } } );
		ClearColor.push_back( { .depthStencil = { 1.0f, 0 } } );

		VkRenderPassBeginInfo RenderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		RenderPassBeginInfo.renderPass = VulkanContext::Get().GetDefaultVulkanPass();
		RenderPassBeginInfo.framebuffer = rSwapchain.GetFramebuffers()[ Renderer::Get().GetImageIndex() ];
		RenderPassBeginInfo.renderArea.offset = { 0, 0 };
		RenderPassBeginInfo.renderArea.extent = { ( uint32_t ) Window::Get().Width(), ( uint32_t ) Window::Get().Height() };
		RenderPassBeginInfo.clearValueCount = (uint32_t)ClearColor.size();
		RenderPassBeginInfo.pClearValues = ClearColor.data();

		// Begin swap chain pass
		CmdBeginDebugLabel( CommandBuffer, "Swap chain pass" );
		vkCmdBeginRenderPass( CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = 0;
		Viewport.width = ( float ) Window::Get().Width();
		Viewport.height = ( float ) Window::Get().Height();
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;

		VkRect2D Scissor = { { 0, 0 }, { ( uint32_t ) Window::Get().Width(), ( uint32_t ) Window::Get().Height() } };

		vkCmdSetViewport( CommandBuffer, 0, 1, &Viewport );
		vkCmdSetScissor( CommandBuffer, 0, 1, &Scissor );

		ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), CommandBuffer );

		ImGuiIO& rIO = ImGui::GetIO();
		if( rIO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		vkCmdEndRenderPass( CommandBuffer );
		CmdEndDebugLabel( CommandBuffer );
	}

	void ImGuiLayer::OnImGuiRender( void )
	{

	}

}