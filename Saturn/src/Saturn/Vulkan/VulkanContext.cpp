#include "sppch.h"
#include "VulkanContext.h"

#include "VulkanDebugMessenger.h"

#include "VulkanDebug.h"

// ImGui
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "ImGuiVulkan.h"
#include "SceneRenderer.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <vulkan.h>
#include <cassert>
#include <set>
#include <iostream>
#include <string>

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace Saturn {
	
	void VulkanContext::Init()
	{
		CreateInstance();
		m_pDebugMessenger = new VulkanDebugMessenger( m_Instance );

		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateCommandPool();
		CreateSyncObjects();

		// Init a theoretical swap chain.
		SwapchainCreationData Data = GetSwapchainCreationData();
		Data ={};
		
		CreateRenderpass();
		CreateDepthResources();
		CreateFramebuffers();

		CreateDescriptorSetLayout();

		ShaderWorker::Get();
		
		Shader* pShader = new Shader( "Triangle/Shader", "assets/shaders/shader_new.glsl" );

		ShaderWorker::Get().AddShader( pShader );
		ShaderWorker::Get().CompileShader( pShader );

		m_Camera = EditorCamera( glm::perspective( glm::radians( 45.0f ), ( float )Window::Get().Width() / ( float )Window::Get().Height(), 0.1f, 10000.0f ) );

		CreatePipeline();
		CreateOffscreenImages();

		m_pImGuiVulkan = new ImGuiVulkan();
		SceneRenderer::Get();
	}

	void VulkanContext::Terminate()
	{
		delete m_pImGuiVulkan;

		m_SwapChain.Terminate();
		m_RenderPass.Terminate();
	
		if( m_UniformBuffers.size() > 1 ) 
		{
			for( auto& [uid, buffer] : m_UniformBuffers )
			{
				m_UniformBuffers[ uid ].Terminate();
				m_UniformBuffers.erase( uid );
			}
		}
		
		m_UniformBuffers.clear();

		if( m_UniformBuffersMemory.size() > 1 )
		{
			for( auto& [uid, mem] : m_UniformBuffersMemory )
			{
				vkFreeMemory( m_LogicalDevice, m_UniformBuffersMemory[ uid ], nullptr );
				m_UniformBuffersMemory.erase( uid );
			}
		}

		m_UniformBuffersMemory.clear();

		vkFreeMemory( m_LogicalDevice, m_DepthImageMemory, nullptr );
		vkDestroyImageView( m_LogicalDevice, m_DepthImageView, nullptr );
		vkDestroyImage( m_LogicalDevice, m_DepthImage, nullptr );
		
		{
			vkFreeMemory( m_LogicalDevice, m_OffscreenColorMem, nullptr );
			vkFreeMemory( m_LogicalDevice, m_OffscreenDepthMem, nullptr );

			vkDestroyImageView( m_LogicalDevice, m_OffscreenColorImageView, nullptr );
			vkDestroyImageView( m_LogicalDevice, m_OffscreenDepthImageView, nullptr );

			vkDestroyImage( m_LogicalDevice, m_OffscreenColorImage, nullptr );
			vkDestroyImage( m_LogicalDevice, m_OffscreenDepthImage, nullptr );

			vkDestroySampler( m_LogicalDevice, m_OffscreenColorSampler, nullptr );
			vkDestroySampler( m_LogicalDevice, m_OffscreenDepthSampler, nullptr );

			vkDestroyFramebuffer( m_LogicalDevice, m_OffscreenFramebuffer, nullptr );

			vkDestroyRenderPass( m_LogicalDevice, m_OffscreenPass, nullptr );
		}

		vkDestroyDescriptorPool( m_LogicalDevice, m_DescriptorPool, nullptr );

		vkDestroyDescriptorSetLayout( m_LogicalDevice, m_DescriptorSetLayouts, nullptr );
		
		vkDestroyCommandPool( m_LogicalDevice, m_CommandPool, nullptr );
		vkDestroySemaphore( m_LogicalDevice, m_SubmitSemaphore, nullptr );
		vkDestroySemaphore( m_LogicalDevice, m_AcquireSemaphore, nullptr );

		for( auto& rFence : m_FlightFences )
		{
			vkDestroyFence( m_LogicalDevice, rFence, nullptr );
		}

		for( auto& rImageView : m_SwapChainImageViews )
		{
			vkDestroyImageView( m_LogicalDevice, rImageView, nullptr );
		}

		for( auto& rFramebuffer : m_SwapChainFramebuffers )
		{
			vkDestroyFramebuffer( m_LogicalDevice, rFramebuffer, nullptr );
		}

		m_Pipeline.Terminate();

		vkDestroyDevice( m_LogicalDevice, nullptr );

		delete m_pDebugMessenger;
		m_pDebugMessenger = nullptr;

		vkDestroySurfaceKHR( m_Instance, m_Surface, nullptr );
		vkDestroyInstance( m_Instance, nullptr );
	}

	void VulkanContext::CreateInstance()
	{
		CheckValidationLayerSupport();

		VkApplicationInfo AppInfo  ={ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		AppInfo.pApplicationName   = "Vulkan Engine";
		AppInfo.pEngineName        = "Vulkan Engine";
		AppInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
		AppInfo.engineVersion      = VK_MAKE_VERSION( 1, 0, 0 );
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
			InstanceInfo.pNext = ( VkDebugUtilsMessengerCreateInfoEXT* )&DebugCreateInfo;
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

		if( DeviceCount == 0 )
		{
			assert( 0 ); // No device found that supports Vulkan.
		}

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
			vkGetPhysicalDeviceProperties( m_PhysicalDevice, &m_DeviceProps[ 0 ].DeviceProps );

			SAT_CORE_INFO( "===== Vulkan Device {0} Properties ===== ", i );
			SAT_CORE_INFO( " Device Name: {0}", m_DeviceProps[ 0 ].DeviceProps.deviceName );
			SAT_CORE_INFO( " Driver Version: {0}", m_DeviceProps[ 0 ].DeviceProps.driverVersion );
			SAT_CORE_INFO( " API Version: {0}", m_DeviceProps[ 0 ].DeviceProps.apiVersion );

			{
				uint32_t Count;
				vkEnumerateInstanceExtensionProperties( nullptr, &Count, nullptr );
				std::vector<VkExtensionProperties> Extensions( Count );
				vkEnumerateInstanceExtensionProperties( nullptr, &Count, Extensions.data() );

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
		// It's very unlikey for a modern GPU to not support 'samplerAnisotropy' but just in case we check.
		VkPhysicalDeviceFeatures Features;
		vkGetPhysicalDeviceFeatures( m_PhysicalDevice, &Features );

		SAT_CORE_ASSERT( Features.samplerAnisotropy, "The GPU does not support anisotropic filtering." );

		Features.samplerAnisotropy = VK_TRUE;

		DeviceExtensions.push_back( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );
		//DeviceExtensions.push_back( "VK_EXT_debug_report" );

		VkDeviceCreateInfo DeviceInfo      ={ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		DeviceInfo.enabledExtensionCount   = DeviceExtensions.size();
		DeviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
		DeviceInfo.pQueueCreateInfos       = QueueCreateInfos.data();
		DeviceInfo.queueCreateInfoCount    = QueueCreateInfos.size();
		DeviceInfo.pEnabledFeatures = &Features;

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
		m_Camera.OnEvent( e );
	}

	void VulkanContext::CreateSwapChain()
	{
		m_SwapChain = Swapchain();
		m_SwapChain.Create();
	}

	void VulkanContext::CreateCommandPool()
	{
		VkCommandPoolCreateInfo CommandPoolInfo ={ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		CommandPoolInfo.queueFamilyIndex = m_Indices.GraphicsFamily.value();

		VK_CHECK( vkCreateCommandPool( m_LogicalDevice, &CommandPoolInfo, nullptr, &m_CommandPool ) );
		SetDebugUtilsObjectName( "Command Pool", ( uint64_t )m_CommandPool, VK_OBJECT_TYPE_COMMAND_POOL );
	}

	void VulkanContext::CreateSyncObjects()
	{
		VkSemaphoreCreateInfo SemaphoreCreateInfo ={ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo     FenceCreateInfo     ={ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		FenceCreateInfo.flags                     = VK_FENCE_CREATE_SIGNALED_BIT;

		m_FlightFences.resize( MAX_FRAMES_IN_FLIGHT );

		for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			VK_CHECK( vkCreateFence( m_LogicalDevice, &FenceCreateInfo, nullptr, &m_FlightFences[ i ] ) );
		}

		VK_CHECK( vkCreateSemaphore( m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_AcquireSemaphore ) );
		VK_CHECK( vkCreateSemaphore( m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_SubmitSemaphore ) );

		SetDebugUtilsObjectName( "Semaphore", ( uint64_t )m_AcquireSemaphore, VK_OBJECT_TYPE_SEMAPHORE );
		SetDebugUtilsObjectName( "Semaphore", ( uint64_t )m_SubmitSemaphore, VK_OBJECT_TYPE_SEMAPHORE );
	}

	void VulkanContext::CreateFramebuffers()
	{
		m_SwapChain.CreateFramebuffers();
	}

	void VulkanContext::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding UBOLayoutBinding = {};
		UBOLayoutBinding.binding = 0;
		UBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UBOLayoutBinding.descriptorCount = 1;
		
		UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		UBOLayoutBinding.pImmutableSamplers = nullptr;
		
		VkDescriptorSetLayoutBinding SamplerLayoutBinding = {};
		SamplerLayoutBinding.binding = 1;
		SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		SamplerLayoutBinding.descriptorCount = 1;
		SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector< VkDescriptorSetLayoutBinding > Bindings = { UBOLayoutBinding, SamplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo LayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		LayoutCreateInfo.bindingCount = Bindings.size();
		LayoutCreateInfo.pBindings = Bindings.data();	

		VK_CHECK( vkCreateDescriptorSetLayout( m_LogicalDevice, &LayoutCreateInfo, nullptr, &m_DescriptorSetLayouts ) );
	}

	void VulkanContext::CreateDescriptorPool()
	{
		if( m_DescriptorPool )
			vkDestroyDescriptorPool( m_LogicalDevice, m_DescriptorPool, nullptr );

		m_DescriptorPool = nullptr;

		std::vector< VkDescriptorPoolSize > PoolSizes = {};
		
		for( int i = 0; i < m_UniformBuffers.size(); i++ )
		{
			PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000 } );

			PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );
		}

		VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.poolSizeCount = PoolSizes.size();
		PoolCreateInfo.pPoolSizes = PoolSizes.data();
		PoolCreateInfo.maxSets = 10000;
		PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;		

		VK_CHECK( vkCreateDescriptorPool( m_LogicalDevice, &PoolCreateInfo, nullptr, &m_DescriptorPool ) );
	}

	void VulkanContext::CreateDescriptorSet( UUID uuid, Ref< Texture > rTexture )
	{
		std::vector< VkDescriptorSetLayout > Layouts( 10000, m_DescriptorSetLayouts );

		VkDescriptorSetAllocateInfo AllocateInfo ={ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		AllocateInfo.descriptorPool = m_DescriptorPool;
		AllocateInfo.descriptorSetCount = m_UniformBuffers.size();
		AllocateInfo.pSetLayouts = Layouts.data();

		VK_CHECK( vkAllocateDescriptorSets( m_LogicalDevice, &AllocateInfo, &m_DescriptorSets[ uuid ] ) );

		VkDescriptorBufferInfo BufferInfo ={};
		BufferInfo.buffer = m_UniformBuffers[ uuid ].GetBuffer();
		BufferInfo.offset = 0;
		BufferInfo.range = sizeof( UniformBufferObject );
		
		VkDescriptorImageInfo ImageInfo ={};
		ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ImageInfo.imageView = rTexture->GetImageView();
		ImageInfo.sampler = rTexture->GetSampler();

		std::vector< VkWriteDescriptorSet > DescriptorWrites;
		
		DescriptorWrites.push_back( {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_DescriptorSets[ uuid ],
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &ImageInfo,
			.pBufferInfo = nullptr,
			.pTexelBufferView = nullptr } );

		DescriptorWrites.push_back( {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_DescriptorSets[ uuid ],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = &BufferInfo,
			.pTexelBufferView = nullptr } );

		vkUpdateDescriptorSets( m_LogicalDevice, DescriptorWrites.size(), DescriptorWrites.data(), 0, nullptr );
	}

	void VulkanContext::DestoryDescriptorPool()
	{
		vkDestroyDescriptorPool( m_LogicalDevice, m_DescriptorPool, nullptr );
	}

	void VulkanContext::DestoryDescriptorSets()
	{
		if( !m_DescriptorSets.size() )
			return;

		vkDestroyDescriptorSetLayout( m_LogicalDevice, m_DescriptorSetLayouts, nullptr );
	}

	void VulkanContext::UpdateUniformBuffers( UUID uuid, Timestep ts, glm::mat4 Transform )
	{
		UniformBufferObject UBO ={};
		UBO.Model = Transform;

		UBO.View = m_Camera.ViewMatrix();
		UBO.Proj = m_Camera.ProjectionMatrix();
		UBO.Proj[ 1 ][ 1 ] *= -1;

		void* Data;
		VK_CHECK( vkMapMemory( m_LogicalDevice, m_UniformBuffersMemory[ uuid ], 0, sizeof( UBO ), 0, &Data ) );
		memcpy( Data, &UBO, sizeof( UBO ) );
		vkUnmapMemory( m_LogicalDevice, m_UniformBuffersMemory[ uuid ] );
	}

	void VulkanContext::AddUniformBuffer( UUID uuid )
	{
		VkDeviceSize BufferSize = sizeof( UniformBufferObject );

		m_UniformBuffersMemory[ uuid ] = nullptr;

		m_UniformBuffers[ uuid ].Create( nullptr, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffersMemory[ uuid ] );
	}

	void VulkanContext::CreateDepthResources()
	{
		vkDestroyImage( m_LogicalDevice, m_DepthImage, nullptr );
		vkFreeMemory( m_LogicalDevice, m_DepthImageMemory, nullptr );
		vkDestroyImageView( m_LogicalDevice, m_DepthImageView, nullptr );
		
		VkFormat DepthFormat = FindDepthFormat();
		
		CreateImage( 
			Window::Get().Width(), Window::Get().Height(),
			DepthFormat, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			m_DepthImage, 
			m_DepthImageMemory 
		);
		
		m_DepthImageView = CreateImageView( m_DepthImage, DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT );
	}

	void VulkanContext::CreateFramebuffer( VkFramebuffer* pFramebuffer )
	{
		VkFramebufferCreateInfo FramebufferCreateInfo ={ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		FramebufferCreateInfo.width = Window::Get().Width();
		FramebufferCreateInfo.height = Window::Get().Height();
		FramebufferCreateInfo.renderPass = m_RenderPass.GetRenderPass();
		FramebufferCreateInfo.layers = 1;
		FramebufferCreateInfo.attachmentCount = 1;

		for( uint32_t i = 0; i < m_ImageCount; i++ )
		{
			FramebufferCreateInfo.pAttachments = &m_SwapChainImageViews[ i ];
		}

		VK_CHECK( vkCreateFramebuffer( m_LogicalDevice, &FramebufferCreateInfo, nullptr, pFramebuffer ) );
		SetDebugUtilsObjectName( "Framebuffer", ( uint64_t )pFramebuffer, VK_OBJECT_TYPE_FRAMEBUFFER );
	}

	void VulkanContext::CreateRenderpass()
	{
		m_RenderPass = Pass( nullptr, "MainRenderPass");
	}

	void VulkanContext::CreatePipeline()
	{
		
		VkPushConstantRange PushConstantRage ={};
		PushConstantRage.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		PushConstantRage.offset = 0;
		PushConstantRage.size = sizeof( PushConstant );

		PipelineSpecification Spec;
		
		Spec.Width = Window::Get().Width();
		Spec.Height = Window::Get().Height();
		Spec.Name = "MainPipeline";
		Spec.RenderPass = m_RenderPass.GetRenderPass();
		Spec.pShader = ShaderWorker::Get().GetShader( "Triangle/Shader" );
		Spec.UseDepthTest = true;
		Spec.Layout.PushConstants = { { PushConstantRage } };
		Spec.Layout.SetLayouts = { { m_DescriptorSetLayouts }  };

		m_Pipeline.Terminate();
		m_Pipeline = Pipeline( Spec );
	}

	void VulkanContext::ResizeEvent()
	{
		if( m_WindowIconifed )
			return;

		vkDeviceWaitIdle( m_LogicalDevice );

		//CreateRenderpass();
		CreateDepthResources();
		//CreateFramebuffers();

		m_SwapChain.Recreate();

		m_Pipeline.Terminate();
		
		CreateOffscreenImages();

		m_pImGuiVulkan->RecreateImages();

		CreatePipeline();
//		CreateUniformBuffers();
//		CreateCommandPool();
//		CreateDescriptorPool();
//		CreateDescriptorSetLayout();
//		CreateDescriptorSets();
		
		m_Camera.SetViewportSize( Window::Get().Width(), Window::Get().Height() );
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

	void VulkanContext::CreateOffscreenImages()
	{
		if( m_OffscreenColorImage )
			vkDestroyImage( m_LogicalDevice, m_OffscreenColorImage, nullptr );

		if( m_OffscreenColorMem )
			vkFreeMemory( m_LogicalDevice, m_OffscreenColorMem, nullptr );
		
		if( m_OffscreenDepthImage )
			vkDestroyImage( m_LogicalDevice, m_OffscreenDepthImage, nullptr );

		if( m_OffscreenDepthMem )
			vkFreeMemory( m_LogicalDevice, m_OffscreenDepthMem, nullptr );
		
		if( m_OffscreenDepthImageView )
			vkDestroyImageView( m_LogicalDevice, m_OffscreenDepthImageView, nullptr );
		
		if( m_OffscreenColorImageView )
			vkDestroyImageView( m_LogicalDevice, m_OffscreenColorImageView, nullptr );

		if( m_OffscreenColorSampler )
			vkDestroySampler( m_LogicalDevice, m_OffscreenColorSampler, nullptr );
		
		if( m_OffscreenDepthSampler )
			vkDestroySampler( m_LogicalDevice, m_OffscreenDepthSampler, nullptr );

		if( m_OffscreenFramebuffer )
			vkDestroyFramebuffer( m_LogicalDevice, m_OffscreenFramebuffer, nullptr );
		
		if( m_OffscreenPass )
			vkDestroyRenderPass( m_LogicalDevice, m_OffscreenPass, nullptr );

		// Create the color attachment.
		VkFormat ColorFormat = FindSupportedFormat( { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT );
		VkFormat DepthFormat = FindSupportedFormat( { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );

		VkImageCreateInfo Image = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		Image.imageType = VK_IMAGE_TYPE_2D;
		Image.extent.width = Window::Get().Width();
		Image.extent.height = Window::Get().Height();
		Image.extent.depth = 1;
		Image.mipLevels = 1;
		Image.arrayLayers = 1;
		Image.format = VK_FORMAT_B8G8R8A8_UNORM;
		Image.samples = VK_SAMPLE_COUNT_1_BIT;
		Image.tiling = VK_IMAGE_TILING_OPTIMAL;
		Image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		
		VK_CHECK( vkCreateImage( m_LogicalDevice, &Image, nullptr, &m_OffscreenColorImage ) );

		VkMemoryRequirements MemReqs;
		vkGetImageMemoryRequirements( m_LogicalDevice, m_OffscreenColorImage, &MemReqs );
	
		VkMemoryAllocateInfo AllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		AllocInfo.allocationSize = MemReqs.size;
		AllocInfo.memoryTypeIndex = GetMemoryType( MemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		VK_CHECK( vkAllocateMemory( m_LogicalDevice, &AllocInfo, nullptr, &m_OffscreenColorMem ) );
		VK_CHECK( vkBindImageMemory( m_LogicalDevice, m_OffscreenColorImage, m_OffscreenColorMem, 0 ) );
		
		// Create the color attachment view.
		VkImageViewCreateInfo ColorView = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		ColorView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ColorView.format = VK_FORMAT_B8G8R8A8_UNORM;
		ColorView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ColorView.subresourceRange.baseMipLevel = 0;
		ColorView.subresourceRange.levelCount = 1;
		ColorView.subresourceRange.baseArrayLayer = 0;
		ColorView.subresourceRange.layerCount = 1;
		ColorView.image = m_OffscreenColorImage;
		
		VK_CHECK( vkCreateImageView( m_LogicalDevice, &ColorView, nullptr, &m_OffscreenColorImageView ) );

		// Create sampler.
		VkSamplerCreateInfo Sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		Sampler.magFilter = VK_FILTER_LINEAR;
		Sampler.minFilter = VK_FILTER_LINEAR;
		Sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		Sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		Sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		Sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		Sampler.mipLodBias = 0.0f;
		Sampler.maxAnisotropy = 1.0f;
		Sampler.minLod = 0.0f;
		Sampler.maxLod = 1.0f;
		Sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		
		VK_CHECK( vkCreateSampler( m_LogicalDevice, &Sampler, nullptr, &m_OffscreenColorSampler ) );

		// Create the depth attachment.
		Image.format = DepthFormat;
		Image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VK_CHECK( vkCreateImage( m_LogicalDevice, &Image, nullptr, &m_OffscreenDepthImage ) );
		
		vkGetImageMemoryRequirements( m_LogicalDevice, m_OffscreenDepthImage, &MemReqs );
		AllocInfo.allocationSize = MemReqs.size;
		AllocInfo.memoryTypeIndex = GetMemoryType( MemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		
		VK_CHECK( vkAllocateMemory( m_LogicalDevice, &AllocInfo, nullptr, &m_OffscreenDepthMem ) );
		VK_CHECK( vkBindImageMemory( m_LogicalDevice, m_OffscreenDepthImage, m_OffscreenDepthMem, 0 ) );

		// Create the depth attachment view.
		VkImageViewCreateInfo DepthView = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		DepthView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		DepthView.format = DepthFormat;
		DepthView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		DepthView.subresourceRange.baseMipLevel = 0;
		DepthView.subresourceRange.levelCount = 1;
		DepthView.subresourceRange.baseArrayLayer = 0;
		DepthView.subresourceRange.layerCount = 1;
		DepthView.image = m_OffscreenDepthImage;
		
		VK_CHECK( vkCreateImageView( m_LogicalDevice, &DepthView, nullptr, &m_OffscreenDepthImageView ) );

		std::vector< VkAttachmentDescription > AttchmentDescriptions = {};
	
		// Color attachment.
		AttchmentDescriptions.push_back( { 
			.flags = 0, 
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.samples = VK_SAMPLE_COUNT_1_BIT, 
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, 
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE, 
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, 
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, 
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, 
			.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 
		} );
		
		// Depth attachment.
		AttchmentDescriptions.push_back( {
			.flags = 0,
			.format = DepthFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		} );
		
		VkAttachmentReference ColorAttachmentRef = {};
		ColorAttachmentRef.attachment = 0;
		ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkAttachmentReference DepthAttachmentRef = {};
		DepthAttachmentRef.attachment = 1;
		DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription Subpass = {};
		Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		Subpass.colorAttachmentCount = 1;
		Subpass.pColorAttachments = &ColorAttachmentRef;
		Subpass.pDepthStencilAttachment = &DepthAttachmentRef;
		
		std::vector< VkSubpassDependency > Dependencies ={};
		
		Dependencies.push_back( {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		} );

		Dependencies.push_back( {
			.srcSubpass = 0,
			.dstSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		} );

		VkRenderPassCreateInfo RenderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		RenderPassInfo.attachmentCount = (uint32_t)AttchmentDescriptions.size();
		RenderPassInfo.pAttachments = AttchmentDescriptions.data();
		RenderPassInfo.subpassCount = 1;
		RenderPassInfo.pSubpasses = &Subpass;
		RenderPassInfo.dependencyCount = (uint32_t)Dependencies.size();
		RenderPassInfo.pDependencies = Dependencies.data();
		
		VK_CHECK( vkCreateRenderPass( m_LogicalDevice, &RenderPassInfo, nullptr, &m_OffscreenPass ) );

		// Create the framebuffer.

		VkImageView Attachments[] = { m_OffscreenColorImageView, m_OffscreenDepthImageView };

		VkFramebufferCreateInfo FramebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		FramebufferInfo.renderPass = m_OffscreenPass;
		FramebufferInfo.attachmentCount = 2;
		FramebufferInfo.pAttachments = Attachments;
		FramebufferInfo.width = Window::Get().Width();
		FramebufferInfo.height = Window::Get().Height();
		FramebufferInfo.layers = 1;
		
		VK_CHECK( vkCreateFramebuffer( m_LogicalDevice, &FramebufferInfo, nullptr, &m_OffscreenFramebuffer ) );
	}

	//////////////////////////////////////////////////////////////////////////
	// Render Loop
	//////////////////////////////////////////////////////////////////////////

	// Fence = CPU wait for GPU
	// Semaphore = Can wait for other commands before running a command

	void VulkanContext::Render()
	{
		// Wait for last frame.
		VK_CHECK( vkWaitForFences( m_LogicalDevice, 1, &m_FlightFences[ m_FrameCount ], VK_TRUE, UINT32_MAX ) );
		
		if( m_WindowIconifed )
			return;

		// Reset current fence.
		VK_CHECK( vkResetFences( m_LogicalDevice, 1, &m_FlightFences[ m_FrameCount ] ) );

		// Acquire next image.
		uint32_t ImageIndex;
		if( !m_SwapChain.AcquireNextImage( UINT64_MAX, m_AcquireSemaphore, VK_NULL_HANDLE, &ImageIndex ) )
			assert( 0 );

		if( ImageIndex == UINT32_MAX || ImageIndex == 3435973836 )
		{
			assert( 0 ); // Unable to get ImageIndex
		}

		m_DrawCalls = 0;
		
		{
			m_Camera.AllowEvents( true );
			m_Camera.SetActive( true );
			m_Camera.OnUpdate( Application::Get().Time() );
		}

		VkCommandBuffer CommandBuffer;

		// Geometry pass
		{
			VkCommandBufferAllocateInfo AllocateInfo ={ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
			AllocateInfo.commandBufferCount = 1;
			AllocateInfo.commandPool = m_CommandPool;

			VK_CHECK( vkAllocateCommandBuffers( m_LogicalDevice, &AllocateInfo, &CommandBuffer ) );
			SetDebugUtilsObjectName( "CommandBuffer:Render", ( uint64_t )CommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER );

			VkCommandBufferBeginInfo CommandPoolBeginInfo ={ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			CommandPoolBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VK_CHECK( vkBeginCommandBuffer( CommandBuffer, &CommandPoolBeginInfo ) );

			VkExtent2D Extent;
			Window::Get().GetSize( &Extent.width, &Extent.height );
			
			// First pass ~ offscreen rendering to a VkImage
			// This is draw to the surface but will get cleared when we do our ImGui pass, we store the texture in a VkDescriptorSet to render later on.
			{
				VkClearValue ClearColors[ 2 ];
				ClearColors[ 0 ].color ={ { 0.0f, 0.0f, 0.0f, 1.0f } };
				ClearColors[ 1 ].depthStencil ={ 1.0f, 0 };

				VkRenderPassBeginInfo RenderPassInfo ={ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
				RenderPassInfo.renderPass = m_OffscreenPass;
				RenderPassInfo.framebuffer = m_OffscreenFramebuffer;
				RenderPassInfo.renderArea.extent = Extent;
				RenderPassInfo.clearValueCount = 2;
				RenderPassInfo.pClearValues = ClearColors;	
			

				VkDebugMarkerMarkerInfoEXT MarkerInfo ={ VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
				float Color[ 4 ] ={ 0.0f, 1.0f, 0.0f, 1.0f };
				memcpy( MarkerInfo.color, &Color[ 0 ], sizeof( float ) * 4 );
				MarkerInfo.pMarkerName = "Off-screen render pass";

				CmdDebugMarkerBegin( CommandBuffer, &MarkerInfo );

				vkCmdBeginRenderPass( CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
				
				VkRect2D Scissor ={};
				Scissor.extent = Extent;

				VkViewport Viewport ={};
				Viewport.height = Extent.height;
				Viewport.width = Extent.width;
				Viewport.maxDepth = 1.0f;
				Viewport.minDepth = 0;

				vkCmdSetScissor( CommandBuffer, 0, 1, &Scissor );
				vkCmdSetViewport( CommandBuffer, 0, 1, &Viewport );
				
				vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline );

				//////////////////////////////////////////////////////////////////////////
				
				SceneRenderer::Get().SetRendererData( { CommandBuffer } );
				
				m_pImGuiVulkan->GetDockspace()->TryRenderScene();
			
				vkCmdEndRenderPass( CommandBuffer );
				CmdDebugMarkerEnd( CommandBuffer );
			}

			VkDebugMarkerMarkerInfoEXT MarkerInfo ={ VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
			float UIColor[ 4 ] ={ 0.0f, 0.0f, 0.0f, 1.0f };
			memcpy( MarkerInfo.color, &UIColor[ 0 ], sizeof( float ) * 4 );
			MarkerInfo.pMarkerName = "ImGui Pass/present pass";

			CmdDebugMarkerBegin( CommandBuffer, &MarkerInfo );

			m_RenderPass.BeginPass( CommandBuffer, VK_SUBPASS_CONTENTS_INLINE, ImageIndex );

			// ImGui Pass
			{
				m_pImGuiVulkan->BeginImGuiRender( CommandBuffer );

				m_pImGuiVulkan->ImGuiRender();

				m_pImGuiVulkan->EndImGuiRender();
			}

			m_RenderPass.EndPass();
			CmdDebugMarkerEnd( CommandBuffer );
			
			VK_CHECK( vkEndCommandBuffer( CommandBuffer ) );

			// Rendering Queue
			VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			VkSubmitInfo SubmitInfo ={ VK_STRUCTURE_TYPE_SUBMIT_INFO };
			SubmitInfo.commandBufferCount = 1;
			SubmitInfo.pCommandBuffers = &CommandBuffer;
			SubmitInfo.pWaitDstStageMask = &WaitStage;

			// SIGNAL the SubmitSemaphore
			SubmitInfo.pSignalSemaphores = &m_SubmitSemaphore;
			SubmitInfo.signalSemaphoreCount = 1;

			// WAIT for AcquireSemaphore
			SubmitInfo.pWaitSemaphores = &m_AcquireSemaphore;
			SubmitInfo.waitSemaphoreCount = 1;

			VK_CHECK( vkResetFences( m_LogicalDevice, 1, &m_FlightFences[ m_FrameCount ] ) );

			// Use current fence to be signaled.
			VK_CHECK( vkQueueSubmit( m_GraphicsQueue, 1, &SubmitInfo, m_FlightFences[ m_FrameCount ] ) );
		}

		// Present info.
		VkPresentInfoKHR PresentInfo ={ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		PresentInfo.pSwapchains = &m_SwapChain.GetSwapchain();
		PresentInfo.swapchainCount = 1;
		PresentInfo.pImageIndices = &ImageIndex;

		// WAIT for SubmitSemaphore
		PresentInfo.pWaitSemaphores = &m_SubmitSemaphore;
		PresentInfo.waitSemaphoreCount = 1;

		VK_CHECK( vkQueuePresentKHR( m_GraphicsQueue, &PresentInfo ) );

		// #TODO: This is bad.
		VkResult Result = vkDeviceWaitIdle( m_LogicalDevice );
		if( Result == VK_SUBOPTIMAL_KHR || Result == VK_ERROR_OUT_OF_DATE_KHR )
		{
			ResizeEvent();
		}
		else if( Result != VK_SUCCESS )
		{
			std::cout << "Failed to present swapchain image" << std::endl;
		}

		VK_CHECK( vkQueueWaitIdle( m_PresentQueue ) );

		vkFreeCommandBuffers( m_LogicalDevice, m_CommandPool, 1, &CommandBuffer );
		
		//////////////////////////////////////////////////////////////////////////
		// Destroy command sets & pools as we will recreate them.
		
		if( SceneRenderer::Get().GetDrawCmds().size() )
		{
			DestoryDescriptorSets();
			DestoryDescriptorPool();
		}

		//////////////////////////////////////////////////////////////////////////

		m_FrameCount = ( m_FrameCount + 1 ) % MAX_FRAMES_IN_FLIGHT;
	}

}