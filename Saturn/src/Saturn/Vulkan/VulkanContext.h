#pragma once

#include "Base.h"
#include "Saturn/Core/Window.h"
#include "Saturn/Core/App.h"
#include "Saturn/Core/Renderer/EditorCamera.h"
#include "Shader.h"

#include "SwapChain.h"
#include "Mesh.h"
#include "Pass.h"

#include "VulkanVertexBuffer.h"

#include <glm/glm.hpp>

#include <vulkan.h>
#include <vector>
#include <optional>

//class Swapchain;
namespace Saturn {

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
		
		VkInstance& GetInstance() { return m_Instance; }

		VkDevice& GetDevice() { return m_LogicalDevice; }

		VkSurfaceKHR& GetSurface() { return m_Surface; }
		VkSurfaceFormatKHR& GetSurfaceFormat() { return m_SurfaceFormat; }

		SwapchainCreationData GetSwapchainCreationData();

		QueueFamilyIndices& GetQueueFamilyIndices() { return m_Indices; };

		Pass& GetRenderPass() { return m_RenderPass; }
		VkCommandPool& GetCommandPool() { return m_CommandPool; }

		VkQueue& GetGraphicsQueue() { return m_GraphicsQueue; }

		Swapchain& GetSwapchain() { return m_SwapChain; }

		void OnEvent( Event& e );

		void SetWindowIconifed( bool inconifed ) { m_WindowIconifed = inconifed; }

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

		void CreateRenderpass();

		void CreatePipeline();

		bool CheckValidationLayerSupport();

		VkInstance m_Instance;
		VkSurfaceKHR m_Surface;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_LogicalDevice;
		Swapchain m_SwapChain ={};
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkExtent2D m_SwapChainExtent;
		VkCommandPool m_CommandPool;
		//VkRenderPass m_RenderPass;
		VkPipeline m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		Pass m_RenderPass;

		VulkanDebugMessenger* m_pDebugMessenger;

		VkSemaphore m_SubmitSemaphore, m_AcquireSemaphore;

		VkQueue m_GraphicsQueue, m_PresentQueue;

		VkSurfaceFormatKHR m_SurfaceFormat;

		QueueFamilyIndices m_Indices;

		Ref<Mesh> m_Mesh;
		
		VertexBuffer m_Buffer;
		IndexBuffer m_IndexBuffer;
		EditorCamera m_Camera;

		std::vector<VkImage>       m_SwapChainImages;
		std::vector<VkImageView>   m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		std::vector<PhysicalDeviceProperties> m_DeviceProps;

		std::vector<VkFence> m_FightFences;

		uint32_t m_ImageCount;

		bool m_WindowIconifed = false;

		std::vector<const char*> DeviceExtensions  ={ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME };
		std::vector<const char*> ValidationLayers ={ "VK_LAYER_KHRONOS_validation" };
	};
}