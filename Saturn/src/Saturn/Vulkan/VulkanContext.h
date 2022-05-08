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

	class VulkanContext
	{
		SINGLETON( VulkanContext );

	public:
		VulkanContext() { }
		~VulkanContext() { Terminate(); }

		void Init();
		void ResizeEvent();

		uint32_t GetMemoryType( uint32_t TypeFilter, VkMemoryPropertyFlags Properties );
		
	public:

		VkFormat FindSupportedFormat( const std::vector<VkFormat>& Formats, VkImageTiling Tiling, VkFormatFeatureFlags Features );
		VkFormat FindDepthFormat();
		bool HasStencilComponent( VkFormat Format );
		
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands( VkCommandBuffer CommandBuffer );

	public:
		
		VkInstance& GetInstance() { return m_Instance; }

		VkDevice& GetDevice() { return m_LogicalDevice; }

		VkSurfaceKHR& GetSurface() { return m_Surface; }
		VkSurfaceFormatKHR& GetSurfaceFormat() { return m_SurfaceFormat; }

		SwapchainCreationData GetSwapchainCreationData();

		QueueFamilyIndices& GetQueueFamilyIndices() { return m_Indices; };

		VkCommandPool& GetCommandPool() { return m_CommandPool; }

		VkDescriptorPool& GetDescriptorPool() { return m_DescriptorPool; }

		VkQueue& GetGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue& GetPresentQueue() { return m_PresentQueue; }

		VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }

		Swapchain& GetSwapchain() { return m_SwapChain; }

		ImGuiVulkan* GetImGuiVulkan() { return m_pImGuiVulkan; }

		std::vector< PhysicalDeviceProperties > GetPhysicalDeviceProperties() { return m_DeviceProps; }
		std::vector< PhysicalDeviceProperties > const GetPhysicalDeviceProperties() const { return m_DeviceProps; }

		void OnEvent( Event& e );

	private:
		void Terminate();

		void CreateInstance();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain();

		bool CheckValidationLayerSupport();

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

		VulkanDebugMessenger* m_pDebugMessenger;

		VkQueue m_GraphicsQueue, m_PresentQueue;

		VkSurfaceFormatKHR m_SurfaceFormat;

		QueueFamilyIndices m_Indices;

		ImGuiVulkan* m_pImGuiVulkan = nullptr;

		std::vector<VkImage>       m_SwapChainImages;
		std::vector<VkImageView>   m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;
		
		std::vector<PhysicalDeviceProperties> m_DeviceProps;

		std::vector<const char*> DeviceExtensions  ={ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME };

		std::vector<const char*> ValidationLayers ={ "VK_LAYER_KHRONOS_validation" };

	private:
		friend class Swapchain;
		friend class VulkanDebug;
	};
}