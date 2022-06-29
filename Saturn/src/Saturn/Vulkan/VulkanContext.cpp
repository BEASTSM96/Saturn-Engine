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
#include "VulkanContext.h"

#include "VulkanDebugMessenger.h"

#include "VulkanDebug.h"
#include "VulkanAllocator.h"

#include "Saturn/Core/Timer.h"

// ImGui
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "SceneRenderer.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <vulkan.h>
#include <glfw/glfw3.h>
#include <cassert>
#include <set>
#include <iostream>
#include <string>

namespace Saturn {
	
	void VulkanContext::Init()
	{
		CreateInstance();
		m_pDebugMessenger = new VulkanDebugMessenger( m_Instance );

		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateCommandPool();

		// Init a theoretical swap chain.
		SwapchainCreationData Data = GetSwapchainCreationData();
		Data ={};
		
		m_pAllocator = new VulkanAllocator();
	
		// Create default pass.
		PassSpecification Specification = {};
		Specification.Name = "Swapchain render pass";
		Specification.IsSwapchainTarget = true;
		
		// BGRA8 will be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as this is the swapchain target.
		Specification.Attachments = { ImageFormat::BGRA8, ImageFormat::Depth };

		m_DefaultPass = Pass( Specification );

		// Init Renderers.
		SceneRenderer::Get();
		Renderer::Get();

		CreateDepthResources();

		m_SwapChain.CreateFramebuffers();
	}

	void VulkanContext::Terminate()
	{		
		if( m_Terminated )
			return;

		vkDestroyCommandPool( m_LogicalDevice, m_CommandPool, nullptr );
		
		m_DefaultPass.Terminate();

		m_SwapChain.Terminate();
		
		for( auto& rFunc : m_TerminateResourceFuncs )
			rFunc();

		Renderer::Get().Terminate();
		SceneRenderer::Get().Terminate();

		vkDestroyImageView( m_LogicalDevice, m_DepthImageView, nullptr );
		vkDestroyImage( m_LogicalDevice, m_DepthImage, nullptr );
		vkFreeMemory( m_LogicalDevice, m_DepthImageMemory, nullptr );
		
		delete m_pAllocator;

		vkDestroyDevice( m_LogicalDevice, nullptr );

		delete m_pDebugMessenger;
		m_pDebugMessenger = nullptr;

		vkDestroySurfaceKHR( m_Instance, m_Surface, nullptr );
		vkDestroyInstance( m_Instance, nullptr );

		m_Terminated = true;
	}

	void VulkanContext::CreateInstance()
	{
		SAT_CORE_ASSERT( glfwVulkanSupported(), "GLFW must be able to support vulkan." );

		CheckValidationLayerSupport();

		VkApplicationInfo AppInfo  ={ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		AppInfo.pApplicationName = "Saturn Engine";
		AppInfo.pEngineName        = "Saturn Engine";
		AppInfo.applicationVersion = VK_MAKE_VERSION( 0, 0, 1 );
		AppInfo.engineVersion      = VK_MAKE_VERSION( 0, 0, 1 );
		AppInfo.apiVersion         = VK_API_VERSION_1_2;
		
		auto Extensions = Window::Get().GetRequiredExtensions();
		Extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

		VkInstanceCreateInfo InstanceInfo ={ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		InstanceInfo.pApplicationInfo = &AppInfo;

		// Make sure its outside of the scope or it will be destroyed before vkCreateInstance is called.
		VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};

		{
			// Include validation layer names and count.
			InstanceInfo.enabledLayerCount = static_cast< uint32_t >( ValidationLayers.size() );
			InstanceInfo.ppEnabledLayerNames = ValidationLayers.data();

			Helpers::CreateDebugMessengerInfo( &DebugCreateInfo );
			InstanceInfo.pNext = ( VkDebugUtilsMessengerCreateInfoEXT* ) &DebugCreateInfo;
		}

		InstanceInfo.enabledExtensionCount = Extensions.size();
		InstanceInfo.ppEnabledExtensionNames = Extensions.data();

		VK_CHECK( vkCreateInstance( &InstanceInfo, nullptr, &m_Instance ) );

		CreateSurface();
	}

	void VulkanContext::CreateSurface()
	{
		VK_CHECK( Window::Get().CreateWindowSurface( m_Instance, &m_Surface ) );
	}

	void VulkanContext::PickPhysicalDevice()
	{
		uint32_t DeviceCount = 0;
		VK_CHECK( vkEnumeratePhysicalDevices( m_Instance, &DeviceCount, nullptr ) );

		SAT_CORE_ASSERT( DeviceCount != 0, "No device found that supports Vulkan." ); 

		// Create a list of the physical devices.
		std::vector< VkPhysicalDevice > PhysicalDevices( DeviceCount );

		// Enumerate again but fill PhysicalDevices' data.
		VK_CHECK( vkEnumeratePhysicalDevices( m_Instance, &DeviceCount, PhysicalDevices.data() ) );

		// Query queue families and check if we can use render queue.
		for( const auto& rDevice : PhysicalDevices )
		{
			uint32_t FamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties( rDevice, &FamilyCount, 0 );

			std::vector< VkQueueFamilyProperties > QueueProps( FamilyCount );

			// Get the queue family properties again but fill QueueProps' data.
			vkGetPhysicalDeviceQueueFamilyProperties( rDevice, &FamilyCount, QueueProps.data() );

			for( uint32_t i = 0; i < FamilyCount; i++ )
			{
				if( QueueProps[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
				{
					// Save this bit as we need this to do graphics operations on.
					m_Indices.GraphicsFamily = i;
				}

				// Check if we can present images to the surface.
				VkBool32 PresentSupport = false;
				VK_CHECK( vkGetPhysicalDeviceSurfaceSupportKHR( rDevice, i, m_Surface, &PresentSupport ) );

				// Again save the bit as we will need it for presenting.
				if( PresentSupport )
					m_Indices.PresentFamily = i;

				if( m_Indices.Complete() )
				{
					m_PhysicalDevice = rDevice;

					break;
				}
			}
		}
		
		
		for ( int i = 0; i < PhysicalDevices.size(); i++ )
		{
			m_DeviceProps.push_back( {} );
			vkGetPhysicalDeviceProperties( m_PhysicalDevice, &m_DeviceProps[ i ].DeviceProps );

			SAT_CORE_INFO( "===== Vulkan Device {0} Properties ===== ", i );
			SAT_CORE_INFO( " Device Name: {0}", m_DeviceProps[ i ].DeviceProps.deviceName );
			SAT_CORE_INFO( " Driver Version: {0}", m_DeviceProps[ i ].DeviceProps.driverVersion );
			SAT_CORE_INFO( " API Version: {0}", m_DeviceProps[ i ].DeviceProps.apiVersion );

			{
				uint32_t Count;
				vkEnumerateDeviceExtensionProperties( m_PhysicalDevice, nullptr, &Count, nullptr );
				std::vector<VkExtensionProperties> Extensions( Count );
				vkEnumerateDeviceExtensionProperties( m_PhysicalDevice, nullptr, &Count, Extensions.data() );

				SAT_CORE_INFO( " Physical Device {0} has {1} extensions ", i, Count );
				SAT_CORE_INFO( "  Available extensions:" );

				for( const auto& rExtension : Extensions )
				{
					SAT_CORE_INFO( "   {0}", rExtension.extensionName );
				}
			}

		}

		SAT_CORE_INFO( "======================================== " );
	}

	void VulkanContext::CreateLogicalDevice()
	{
		float QueuePriority = 1.0f;

		std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
		std::set<uint32_t> UniqueQueueFamilies ={ m_Indices.GraphicsFamily.value(), m_Indices.PresentFamily.value() };

		for( uint32_t QueueFamily : UniqueQueueFamilies )
		{
			VkDeviceQueueCreateInfo QueueCreateInfo ={ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			QueueCreateInfo.queueFamilyIndex = m_Indices.GraphicsFamily.value();
			QueueCreateInfo.queueCount = 1;
			QueueCreateInfo.pQueuePriorities = &QueuePriority;
			QueueCreateInfos.push_back( QueueCreateInfo );
		}

		// Enable the device features.
		// It's very unlikely for a modern GPU to not support 'samplerAnisotropy' but just in case we check.
		VkPhysicalDeviceFeatures Features;
		vkGetPhysicalDeviceFeatures( m_PhysicalDevice, &Features );

		SAT_CORE_ASSERT( Features.samplerAnisotropy, "The GPU does not support anisotropic filtering." );

		Features.samplerAnisotropy = VK_TRUE;

		DeviceExtensions.push_back( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );
		DeviceExtensions.push_back( VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME );

		VkPhysicalDeviceInlineUniformBlockFeaturesEXT InlineUniformBlockFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT };
		InlineUniformBlockFeatures.inlineUniformBlock = VK_TRUE;

		VkDeviceCreateInfo DeviceInfo      ={ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		DeviceInfo.enabledExtensionCount   = DeviceExtensions.size();
		DeviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
		DeviceInfo.pQueueCreateInfos       = QueueCreateInfos.data();
		DeviceInfo.queueCreateInfoCount    = QueueCreateInfos.size();
		DeviceInfo.pEnabledFeatures = &Features;
		DeviceInfo.pNext = &InlineUniformBlockFeatures;

		VK_CHECK( vkCreateDevice( m_PhysicalDevice, &DeviceInfo, nullptr, &m_LogicalDevice ) );
		SetDebugUtilsObjectName( "Physical Device", ( uint64_t )m_LogicalDevice, VK_OBJECT_TYPE_DEVICE );

		// Assign a queue to each family.
		vkGetDeviceQueue( m_LogicalDevice, m_Indices.GraphicsFamily.value(), 0, &m_GraphicsQueue );
		vkGetDeviceQueue( m_LogicalDevice, m_Indices.PresentFamily.value(), 0, &m_PresentQueue );
	}

	// Get memory type.
	uint32_t VulkanContext::GetMemoryType( uint32_t TypeFilter, VkMemoryPropertyFlags Properties )
	{
		VkPhysicalDeviceMemoryProperties MemProperties;
		vkGetPhysicalDeviceMemoryProperties( m_PhysicalDevice, &MemProperties );

		for( uint32_t i = 0; i < MemProperties.memoryTypeCount; i++ )
		{
			if( ( TypeFilter & ( 1 << i ) ) &&
				( MemProperties.memoryTypes[ i ].propertyFlags & Properties ) == Properties )
			{
				return i;
			}
		}

		return 0;
	}

	// Data required for the swapchain to be created
	SwapchainCreationData VulkanContext::GetSwapchainCreationData()
	{
		SwapchainCreationData Data;

		// Get surface formats.

		VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( m_PhysicalDevice, m_Surface, &Data.FormatCount, 0 ) );

		Data.SurfaceFormats = std::vector<VkSurfaceFormatKHR>( Data.FormatCount );

		VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( m_PhysicalDevice, m_Surface, &Data.FormatCount, Data.SurfaceFormats.data() ) );

		for( VkSurfaceFormatKHR& rFormat : Data.SurfaceFormats )
		{
			// Use most common format type.
			if( rFormat.format == VK_FORMAT_B8G8R8A8_UNORM && rFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
			{
				// Set context's format.
				if( m_SurfaceFormat.format != rFormat.format)
					m_SurfaceFormat = rFormat;

				Data.CurrentFormat = m_SurfaceFormat;
				break;
			}
		}

		// Query Surface Capabilities
		VK_CHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_PhysicalDevice, m_Surface, &Data.SurfaceCaps ) );

		Data.ImageCount = Data.SurfaceCaps.minImageCount + 1;

		if( Data.SurfaceCaps.maxImageCount > 0 && Data.ImageCount > Data.SurfaceCaps.maxImageCount )
		{
			Data.ImageCount = Data.SurfaceCaps.maxImageCount;
		}

		return Data;
	}

	void VulkanContext::OnEvent( Event& e )
	{
	}

	void VulkanContext::CreateSwapChain()
	{
		m_SwapChain = Swapchain();
		m_SwapChain.Create();
	}

	void VulkanContext::ResizeEvent()
	{
		GetSwapchainCreationData();

		CreateDepthResources();

		m_SwapChain.Recreate();

		m_DefaultPass.Recreate();
	}

	bool VulkanContext::CheckValidationLayerSupport()
	{
		uint32_t LayerCount;

		vkEnumerateInstanceLayerProperties( &LayerCount, nullptr );

		std::vector<VkLayerProperties> AvailableLayers( LayerCount );

		vkEnumerateInstanceLayerProperties( &LayerCount, AvailableLayers.data() );

		// Check if all layers in VailationLayers exists in the AvailableLayers list.
		for( const char* pLayerName : ValidationLayers )
		{
			bool LayerFound = false;

			for( const auto& rLayerProps : AvailableLayers )
			{
				if( strcmp( pLayerName, rLayerProps.layerName ) == 0 )
				{
					LayerFound = true;
					break;
				}
			}

			if( !LayerFound )
			{
				return false;
			}
		}

		return true;
	}
	
	VkFormat VulkanContext::FindSupportedFormat( const std::vector<VkFormat>& Formats, VkImageTiling Tiling, VkFormatFeatureFlags Features )
	{
		for ( VkFormat Format : Formats )
		{
			VkFormatProperties FormatProps;
			vkGetPhysicalDeviceFormatProperties( m_PhysicalDevice, Format, &FormatProps );

			if ( ( Tiling == VK_IMAGE_TILING_LINEAR && ( FormatProps.linearTilingFeatures & Features ) == Features ) ||
				( Tiling == VK_IMAGE_TILING_OPTIMAL && ( FormatProps.optimalTilingFeatures & Features ) == Features ) )
			{
				return Format;
			}
		}
	}

	VkFormat VulkanContext::FindDepthFormat()
	{
		return FindSupportedFormat( 
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, 
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
	}

	bool VulkanContext::HasStencilComponent( VkFormat Format )
	{
		return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	VkCommandBuffer VulkanContext::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo AllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocInfo.commandPool = m_CommandPool;
		AllocInfo.commandBufferCount = 1;
		
		VkCommandBuffer CommandBuffer;
		VK_CHECK( vkAllocateCommandBuffers( m_LogicalDevice, &AllocInfo, &CommandBuffer ) );
		
		// Begin the command buffer.
		VkCommandBufferBeginInfo BeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		VK_CHECK( vkBeginCommandBuffer( CommandBuffer, &BeginInfo ) );
		
		return CommandBuffer;
	}

	void VulkanContext::EndSingleTimeCommands( VkCommandBuffer CommandBuffer )
	{
		VK_CHECK( vkEndCommandBuffer( CommandBuffer ) );

		// Submit the command buffer.
		VkSubmitInfo SubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer;

		VK_CHECK( vkQueueSubmit( m_GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE ) );
		VK_CHECK( vkQueueWaitIdle( m_GraphicsQueue ) );

		// Free the command buffer.
		vkFreeCommandBuffers( m_LogicalDevice, m_CommandPool, 1, &CommandBuffer );
	}

	void VulkanContext::CreateCommandPool()
	{
		VkCommandPoolCreateInfo PoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		PoolInfo.queueFamilyIndex = m_Indices.GraphicsFamily.value();
		PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		VK_CHECK( vkCreateCommandPool( m_LogicalDevice, &PoolInfo, nullptr, &m_CommandPool ) );

		SetDebugUtilsObjectName( "Context Command Pool", (uint64_t)m_CommandPool, VK_OBJECT_TYPE_COMMAND_POOL );
	}

	void VulkanContext::CreateDepthResources()
	{
		if( m_DepthImage )
		{
			vkDestroyImage( m_LogicalDevice, m_DepthImage, nullptr );
			vkFreeMemory( m_LogicalDevice, m_DepthImageMemory, nullptr );
		}

		if( m_DepthImageView )
		{
			vkDestroyImageView( m_LogicalDevice, m_DepthImageView, nullptr );
		}

		VkFormat DepthFormat = FindDepthFormat();

		Renderer::Get().CreateImage( VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT,
			{ .width = ( uint32_t )Window::Get().Width(), .height = ( uint32_t )Window::Get().Height(), .depth = 1 }, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_DepthImage, &m_DepthImageMemory );

		Renderer::Get().CreateImageView( VK_IMAGE_VIEW_TYPE_2D, m_DepthImage, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, &m_DepthImageView );
		
		SetDebugUtilsObjectName( "Context Depth Image", (uint64_t)m_DepthImage, VK_OBJECT_TYPE_IMAGE );
		SetDebugUtilsObjectName( "Context Depth Image View", (uint64_t)m_DepthImageView, VK_OBJECT_TYPE_IMAGE_VIEW );
	}

}