#pragma once

#include "Base.h"
#include "Saturn/Core/Window.h"
#include "Saturn/Core/App.h"
#include "Saturn/Core/Renderer/EditorCamera.h"
#include "Shader.h"

#include "IndexBuffer.h"
#include "VertexBuffer.h"

#include "SwapChain.h"
#include "Mesh.h"
#include "Pass.h"
#include "Texture.h"
#include "Pipeline.h"

#include <glm/glm.hpp>

#include <vulkan.h>
#include <vector>
#include <optional>

namespace Saturn {

	class ImGuiVulkan;
	class VulkanDebugMessenger;

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool Complete() { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
	};

	struct SwapchainCreationData
	{
		uint32_t FormatCount = 0;
		uint32_t ImageCount = 0;

		std::vector<VkSurfaceFormatKHR> SurfaceFormats;

		VkSurfaceFormatKHR CurrentFormat;

		VkSurfaceCapabilitiesKHR SurfaceCaps ={};
	};

	struct PhysicalDeviceProperties
	{
		VkPhysicalDeviceProperties DeviceProps ={};
	};

	struct PushConstant
	{
		glm::mat4 Transfrom{ 1.f };
		glm::mat4 VPM{ 1.f };
	};

	struct UniformBufferObject
	{
		alignas( 16 ) glm::mat4 Model;
		alignas( 16 ) glm::mat4 View;
		alignas( 16 ) glm::mat4 Proj;
		alignas( 16 ) glm::mat4 VP;
	};

	class VulkanContext
	{
		SINGLETON( VulkanContext );

	public:
		VulkanContext() { }
		~VulkanContext() { Terminate(); }

		void Render();

		void CreateFramebuffer( VkFramebuffer* pFramebuffer );

		void Init();
		void ResizeEvent();
		uint32_t GetMemoryType( uint32_t TypeFilter, VkMemoryPropertyFlags Properties );
		
	public:

		VkFormat FindSupportedFormat( const std::vector<VkFormat>& Formats, VkImageTiling Tiling, VkFormatFeatureFlags Features );
		VkFormat FindDepthFormat();
		bool HasStencilComponent( VkFormat Format );
		
		void RenderImGui() {}

		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands( VkCommandBuffer CommandBuffer );

	public:
		
		VkInstance& GetInstance() { return m_Instance; }

		VkDevice& GetDevice() { return m_LogicalDevice; }

		VkSurfaceKHR& GetSurface() { return m_Surface; }
		VkSurfaceFormatKHR& GetSurfaceFormat() { return m_SurfaceFormat; }

		SwapchainCreationData GetSwapchainCreationData();

		QueueFamilyIndices& GetQueueFamilyIndices() { return m_Indices; };

		Pass& GetRenderPass() { return m_RenderPass; }
		VkCommandPool& GetCommandPool() { return m_CommandPool; }

		VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }

		VkQueue& GetGraphicsQueue() { return m_GraphicsQueue; }

		VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }

		Swapchain& GetSwapchain() { return m_SwapChain; }

		ImGuiVulkan* GetImGuiVulkan() { return m_pImGuiVulkan; }

		uint32_t GetImageCount() { return m_ImageCount; }

		VkFence& GetCurrentFlightFence() { return m_FlightFences[ m_FrameCount ]; }

		void OnEvent( Event& e );

		void SetWindowIconifed( bool inconifed ) { m_WindowIconifed = inconifed; }
		
		VkImageView& GetOffscreenColorView() { return m_OffscreenColorImageView; }
		VkImageView& GetOffscreenDepthView() { return m_OffscreenDepthImageView; }

		VkSampler& GetOffscreenColorSampler() { return m_OffscreenColorSampler; }
		VkSampler& GetOffscreenDepthSampler() { return m_OffscreenDepthSampler; }

		Pipeline& GetPipeline() { return m_Pipeline; }
		std::unordered_map< UUID, VkDescriptorSet >& GetDescriptorSets() { return m_DescriptorSets; }

		void UpdateUniformBuffers( UUID uuid, Timestep ts, glm::mat4 Transform );
		void AddUniformBuffer( UUID uuid );
		
		// Descriptor

		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateDescriptorSet( UUID uuid, Ref< Texture > rTexture );
		
		void DestoryDescriptorPool();
		void DestoryDescriptorSets();

		// --
		
		void CreatePipeline();

	private:
		void Terminate();

		void CreateInstance();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateCommandPool();
		void CreateSyncObjects();
		void CreateFramebuffers();
		
		// Descriptor
		
		void CreateDescriptorSetLayout();
		
		// --

		void CreateDepthResources();

		void CreateRenderpass();

		bool CheckValidationLayerSupport();

		// Offscreen rendering
		void CreateOffscreenFramebuffer() {}
		void CreateOffscreenImages();

		VkInstance m_Instance;
		VkSurfaceKHR m_Surface;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_LogicalDevice;
		Swapchain m_SwapChain ={};
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkExtent2D m_SwapChainExtent;
		VkCommandPool m_CommandPool;
		VkDescriptorPool m_DescriptorPool;
		
		VkDescriptorSetLayout m_DescriptorSetLayouts;

		Pipeline m_Pipeline;

		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;
		
		Texture m_TestTexture;

		Pass m_RenderPass;
		VkRenderPass m_OffscreenPass;
		VkFramebuffer m_OffscreenFramebuffer;

		VkImage m_OffscreenColorImage, m_OffscreenDepthImage;
		VkDeviceMemory m_OffscreenColorMem, m_OffscreenDepthMem;
		VkImageView m_OffscreenColorImageView, m_OffscreenDepthImageView;
		VkSampler m_OffscreenColorSampler, m_OffscreenDepthSampler;

		VulkanDebugMessenger* m_pDebugMessenger;

		VkSemaphore m_SubmitSemaphore, m_AcquireSemaphore;

		VkQueue m_GraphicsQueue, m_PresentQueue;

		VkSurfaceFormatKHR m_SurfaceFormat;

		QueueFamilyIndices m_Indices;

		EditorCamera m_Camera;

		uint32_t m_ImageCount;
		uint32_t m_FrameCount = 0;

		bool m_WindowIconifed = false;

		int m_DrawCalls;
		
		ImGuiVulkan* m_pImGuiVulkan = nullptr;

		std::vector<VkImage>       m_SwapChainImages;
		std::vector<VkImageView>   m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		
		std::unordered_map< UUID, Buffer > m_UniformBuffers;
		std::unordered_map< UUID, VkDeviceMemory> m_UniformBuffersMemory;

		std::vector<PhysicalDeviceProperties> m_DeviceProps;

		std::vector<VkFence> m_FlightFences;

		std::unordered_map< UUID, VkDescriptorSet > m_DescriptorSets;

		std::vector<const char*> DeviceExtensions  ={ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME };
		std::vector<const char*> ValidationLayers ={ "VK_LAYER_KHRONOS_validation" };

	private:
		friend class Swapchain;
	};
}