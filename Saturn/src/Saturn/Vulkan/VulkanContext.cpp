#include "sppch.h"
#include "VulkanContext.h"

#include "VulkanDebugMessenger.h"

#include <vulkan.h>
#include <cassert>
#include <set>
#include <iostream>
#include <string>

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace Saturn {
	
	const std::vector<Vertex> Vertices{   // left face (white)
	  {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
	  {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
	  {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
	  {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

	  // right face (yellow)
	  {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
	  {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
	  {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
	  {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

	  // top face (orange, remember y axis points down)
	  {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
	  {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
	  {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
	  {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

	  // bottom face (red)
	  {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
	  {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
	  {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
	  {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

	  // nose face (blue)
	  {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
	  {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
	  {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
	  {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

	  // tail face (green)
	  {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
	  {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
	  {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
	  {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
	};

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
		CreateFramebuffers();

		ShaderWorker::Get();

		Shader* pTriangleVertexShader = new Shader( "Triangle/VertexShader", "assets/shaders/shader.vert" );
		Shader* pTriangleFragShader = new Shader( "Triangle/FragShader", "assets/shaders/shader.frag" );

		ShaderWorker::Get().AddShader( pTriangleVertexShader );
		ShaderWorker::Get().AddShader( pTriangleFragShader );

		ShaderWorker::Get().CompileShader( pTriangleVertexShader );
		ShaderWorker::Get().CompileShader( pTriangleFragShader );

		//m_Buffer = VertexBuffer( Vertices );
		//m_Buffer.CreateBuffer();

		m_Mesh = Ref<Mesh>::Create( "assets/meshes/cerberus/cerberus.fbx" );

		m_Camera = EditorCamera( glm::perspectiveFov( glm::radians( 45.0f ), 1280.0f, 720.0f, 0.1f, 10000.0f ) );
		
		CreatePipeline();
	}

	void VulkanContext::Terminate()
	{
		m_Mesh.Delete();
		
		m_SwapChain.Terminate();
		m_RenderPass.Terminate();

		vkDestroyPipeline( m_LogicalDevice, m_Pipeline, nullptr );
		vkDestroyCommandPool( m_LogicalDevice, m_CommandPool, nullptr );
		vkDestroySemaphore( m_LogicalDevice, m_SubmitSemaphore, nullptr );
		vkDestroySemaphore( m_LogicalDevice, m_AcquireSemaphore, nullptr );

		for( auto& rFence : m_FightFences )
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

		vkDestroyPipelineLayout( m_LogicalDevice, m_PipelineLayout, nullptr );

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

		VkDeviceCreateInfo DeviceInfo      ={ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		DeviceInfo.enabledExtensionCount   = DeviceExtensions.size();
		DeviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
		DeviceInfo.pQueueCreateInfos       = QueueCreateInfos.data();
		DeviceInfo.queueCreateInfoCount    = QueueCreateInfos.size();
		//DeviceInfo.pNext = &Features;

		VK_CHECK( vkCreateDevice( m_PhysicalDevice, &DeviceInfo, nullptr, &m_LogicalDevice ) );

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
			if( rFormat.format == VK_FORMAT_B8G8R8A8_SRGB && rFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
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
	}

	void VulkanContext::CreateSyncObjects()
	{
		VkSemaphoreCreateInfo SemaphoreCreateInfo ={ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo     FenceCreateInfo     ={ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		FenceCreateInfo.flags                     = VK_FENCE_CREATE_SIGNALED_BIT;

		m_FightFences.resize( MAX_FRAMES_IN_FLIGHT );

		for( int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ )
		{
			VK_CHECK( vkCreateFence( m_LogicalDevice, &FenceCreateInfo, nullptr, &m_FightFences[ i ] ) );
		}

		VK_CHECK( vkCreateSemaphore( m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_AcquireSemaphore ) );
		VK_CHECK( vkCreateSemaphore( m_LogicalDevice, &SemaphoreCreateInfo, nullptr, &m_SubmitSemaphore ) );
	}

	void VulkanContext::CreateFramebuffers()
	{
		m_SwapChain.CreateFramebuffers();
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

		{
			VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo ={ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
			PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
			PipelineLayoutCreateInfo.pPushConstantRanges = &PushConstantRage;

			VK_CHECK( vkCreatePipelineLayout( m_LogicalDevice, &PipelineLayoutCreateInfo, nullptr, &m_PipelineLayout ) );
		}

		VkShaderModule VertexShader;
		VkShaderModule FragmentShader;

		VkShaderModuleCreateInfo VertexCreateInfo ={ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

		VertexCreateInfo.codeSize = 4 * ShaderWorker::Get().GetShaderCode( "Triangle/VertexShader" ).size();
		VertexCreateInfo.pCode = reinterpret_cast< const uint32_t* >( ShaderWorker::Get().GetShaderCode( "Triangle/VertexShader" ).data() );

		VK_CHECK( vkCreateShaderModule( m_LogicalDevice, &VertexCreateInfo, nullptr, &VertexShader ) );

		VkShaderModuleCreateInfo FragCreateInfo ={ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		FragCreateInfo.codeSize = 4 * ShaderWorker::Get().GetShaderCode( "Triangle/FragShader" ).size();
		FragCreateInfo.pCode = reinterpret_cast< const uint32_t* >( ShaderWorker::Get().GetShaderCode( "Triangle/FragShader" ).data() );

		VK_CHECK( vkCreateShaderModule( m_LogicalDevice, &FragCreateInfo, nullptr, &FragmentShader ) );

		VkPipelineShaderStageCreateInfo VertexStage ={ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		VertexStage.pName = "main";
		VertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VertexStage.module = VertexShader;

		VkPipelineShaderStageCreateInfo FragmentStage ={ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		FragmentStage.pName = "main";
		FragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragmentStage.module = FragmentShader;

		std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
		ShaderStages.push_back( VertexStage );
		ShaderStages.push_back( FragmentStage );

		VkPipelineShaderStageCreateInfo ShaderNStages[ 2 ];
		ShaderNStages[ 0 ] = VertexStage;
		ShaderNStages[ 1 ] = FragmentStage;

		auto BindDesc = VertexBuffer::GetBindingDescriptions();
		auto AttributeDesc = VertexBuffer::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo VertexInputState ={ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VertexInputState.vertexBindingDescriptionCount = BindDesc.size();
		VertexInputState.pVertexBindingDescriptions = BindDesc.data();
		VertexInputState.vertexAttributeDescriptionCount = AttributeDesc.size();
		VertexInputState.pVertexAttributeDescriptions = AttributeDesc.data();

		VkPipelineColorBlendAttachmentState ColorBlendAttachment ={};
		ColorBlendAttachment.blendEnable = VK_FALSE;
		ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo ColorBlendState ={ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		ColorBlendState.pAttachments = &ColorBlendAttachment;
		ColorBlendState.attachmentCount = 1;

		VkPipelineRasterizationStateCreateInfo RasterizationStageCreateInfo ={ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		RasterizationStageCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		// Cull back bit i.e. don't see stuff from the back
		RasterizationStageCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		// Same as glPolygonMode
		RasterizationStageCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizationStageCreateInfo.lineWidth = 1.0f;

		// Multisampling - disabled
		VkPipelineMultisampleStateCreateInfo PipelineMultisampleSCInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		PipelineMultisampleSCInfo.sampleShadingEnable = VK_FALSE;
		PipelineMultisampleSCInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineInputAssemblyStateCreateInfo AssemblyStateCreateInfo ={ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		// A series of separate point primitvies.
		AssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		AssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkRect2D Scissor ={};

		VkViewport Viewport{};
		Viewport.x = 0.0f;
		Viewport.y = 0.0f;
		Viewport.width  = ( float )Window::Get().Width();
		Viewport.height = ( float )Window::Get().Height();
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;

		std::vector< VkDynamicState > DynamicStates;
		DynamicStates.push_back( VK_DYNAMIC_STATE_VIEWPORT );
		DynamicStates.push_back( VK_DYNAMIC_STATE_SCISSOR );

		VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		PipelineDynamicStateCreateInfo.dynamicStateCount = DynamicStates.size();
		PipelineDynamicStateCreateInfo.pDynamicStates = DynamicStates.data();

		VkPipelineViewportStateCreateInfo PipelineViewportState ={ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		PipelineViewportState.pScissors = &Scissor;
		PipelineViewportState.scissorCount = 1;
		PipelineViewportState.pViewports = &Viewport;
		PipelineViewportState.viewportCount = 1;

		VkGraphicsPipelineCreateInfo PipelineCreateInfo ={ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		PipelineCreateInfo.layout = m_PipelineLayout;
		PipelineCreateInfo.renderPass = m_RenderPass.GetRenderPass();
		PipelineCreateInfo.pColorBlendState = &ColorBlendState;
		PipelineCreateInfo.pVertexInputState = &VertexInputState;
		PipelineCreateInfo.stageCount = 2;
		PipelineCreateInfo.pStages = ShaderNStages;
		PipelineCreateInfo.pRasterizationState = &RasterizationStageCreateInfo;
		PipelineCreateInfo.pViewportState = &PipelineViewportState;
		PipelineCreateInfo.pDynamicState = &PipelineDynamicStateCreateInfo;
		PipelineCreateInfo.pMultisampleState = &PipelineMultisampleSCInfo;
		PipelineCreateInfo.pInputAssemblyState = &AssemblyStateCreateInfo;

		// We may need more pipelines, but for now we only need one and we really should only need one as creating them is really expensive.
		VK_CHECK( vkCreateGraphicsPipelines( m_LogicalDevice, 0, 1, &PipelineCreateInfo, nullptr, &m_Pipeline ) );

		vkDestroyShaderModule( m_LogicalDevice, VertexShader, nullptr );
		vkDestroyShaderModule( m_LogicalDevice, FragmentShader, nullptr );
	}

	void VulkanContext::ResizeEvent()
	{
		if( m_WindowIconifed )
			return;

		m_SwapChain.Recreate();

		vkDestroyPipeline( m_LogicalDevice, m_Pipeline, nullptr );
		vkDestroyPipelineLayout( m_LogicalDevice, m_PipelineLayout, nullptr );

		CreatePipeline();
		
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

	//////////////////////////////////////////////////////////////////////////
	// Render Loop
	//////////////////////////////////////////////////////////////////////////

	// Fence = CPU wait for GPU
	// Semaphore = Can wait for other commands before running a command

	static int s_FrameCount = 0;

	void VulkanContext::Render()
	{
		// Wait for last frame.
		VK_CHECK( vkWaitForFences( m_LogicalDevice, 1, &m_FightFences[ s_FrameCount ], VK_TRUE, UINT32_MAX ) );
		
		if( m_WindowIconifed )
			return;

		// Reset current fence.
		VK_CHECK( vkResetFences( m_LogicalDevice, 1, &m_FightFences[ s_FrameCount ] ) );

		// Acquire next image.
		uint32_t ImageIndex;
		if( !m_SwapChain.AcquireNextImage( UINT64_MAX, m_AcquireSemaphore, VK_NULL_HANDLE, &ImageIndex ) )
			assert( 0 );

		if( ImageIndex == UINT32_MAX || ImageIndex == 3435973836 )
		{
			assert( 0 ); // Unable to get ImageIndex
		}
		
		m_Camera.AllowEvents( true );
		m_Camera.SetActive( true );
		m_Camera.OnUpdate( Application::Get().Time() );
		m_Camera.SetViewportSize( Window::Get().Width(), Window::Get().Height() );
		m_Camera.SetProjectionMatrix( glm::perspectiveFov( glm::radians( 45.0f ), ( float )Window::Get().Width(), ( float )Window::Get().Height(), 0.1f, 1000.0f ) );

		VkCommandBuffer CommandBuffer;
		VkCommandBufferAllocateInfo AllocateInfo ={ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		AllocateInfo.commandBufferCount = 1;
		AllocateInfo.commandPool = m_CommandPool;

		VK_CHECK( vkAllocateCommandBuffers( m_LogicalDevice, &AllocateInfo, &CommandBuffer ) );

		VkCommandBufferBeginInfo CommandPoolBeginInfo ={ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		CommandPoolBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK( vkBeginCommandBuffer( CommandBuffer, &CommandPoolBeginInfo ) );

		VkExtent2D Extent;
		Window::Get().GetSize( &Extent.width, &Extent.height );

		m_RenderPass.BeginPass( CommandBuffer, VK_SUBPASS_CONTENTS_INLINE, ImageIndex );
		
		// Rendering Commands
		{
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

			TransformComponent Comp;
 			Comp.Position = glm::vec3( 0, 0, 0 );
			Comp.Rotation = glm::vec3( 0, 0, 0 );
			Comp.Scale = glm::vec3( 1, 1, 1 );
			
			m_Mesh->GetVertexBuffer()->Bind( CommandBuffer );
			m_Mesh->GetIndexBuffer()->Bind( CommandBuffer );
		
			glm::mat4 ViewProj = m_Camera.ViewProjection();

			{
				PushConstant PushC;
				PushC.VPM = ViewProj;
				PushC.Transfrom = Comp.GetTransform();
				
				vkCmdPushConstants( CommandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( PushConstant ), &PushC );

				m_Mesh->GetIndexBuffer()->Draw( CommandBuffer );
			}
		}
		
		m_RenderPass.EndPass();

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

		VK_CHECK( vkResetFences( m_LogicalDevice, 1, &m_FightFences[ s_FrameCount ] ) );

		// Use current fence to be signaled.
		VK_CHECK( vkQueueSubmit( m_GraphicsQueue, 1, &SubmitInfo, m_FightFences[ s_FrameCount ] ) );

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

		s_FrameCount = ( s_FrameCount + 1 ) % MAX_FRAMES_IN_FLIGHT;
	}

}