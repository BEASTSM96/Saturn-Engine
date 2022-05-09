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
#include "SceneRenderer.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

#include <glm/gtx/matrix_decompose.hpp>

#define M_PI 3.14159265358979323846

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::Init()
	{
		// Create command pool.

		VkCommandPoolCreateInfo CommandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		CommandPoolInfo.queueFamilyIndex = VulkanContext::Get().GetQueueFamilyIndices().GraphicsFamily.value();
		CommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		VK_CHECK( vkCreateCommandPool( VulkanContext::Get().GetDevice(), &CommandPoolInfo, nullptr, &m_RendererData.CommandPool ) );
		
		//////////////////////////////////////////////////////////////////////////
		// Geometry 
		//////////////////////////////////////////////////////////////////////////

		InitGeometryPass();

		//////////////////////////////////////////////////////////////////////////

		// Create grid.
		CreateGridComponents();

		// Create skybox.
		CreateSkyboxComponents();
	}

	void SceneRenderer::Terminate()
	{
		// Destroy grid.
		DestroyGridComponents();

		DestroySkyboxComponents();

		m_pSence = nullptr;
		
		m_DrawList.clear();
		
		m_RendererData = {};
	}

	void SceneRenderer::InitGeometryPass()
	{
		// Create render pass.

		// Start by creating the color resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, { .width = ( uint32_t ) Window::Get().Width(), .height = ( uint32_t ) Window::Get().Height(), .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &m_RendererData.GeometryPassColor.Image, &m_RendererData.GeometryPassColor.Memory );

			Renderer::Get().CreateImageView( m_RendererData.GeometryPassColor, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_RendererData.GeometryPassColor.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.GeometryPassColor.Sampler );
		}

		// Then, create the depth resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, { .width = ( uint32_t ) Window::Get().Width(), .height = ( uint32_t ) Window::Get().Height(), .depth = 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_RendererData.GeometryPassDepth.Image, &m_RendererData.GeometryPassDepth.Memory );

			Renderer::Get().CreateImageView( m_RendererData.GeometryPassDepth, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, &m_RendererData.GeometryPassDepth.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.GeometryPassDepth.Sampler );
		}

		PassSpecification PassSpec = {};
		PassSpec.Name = "Geometry Pass";

		PassSpec.Attachments = {
			// Color
			{
				.flags = 0,
				.format = VK_FORMAT_B8G8R8A8_UNORM,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			},
			// Depth
			{
				.flags = 0,
				.format = VK_FORMAT_D32_SFLOAT,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			}
		};

		PassSpec.ColorAttachmentRef = { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		PassSpec.DepthAttachmentRef = { .attachment = 1, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		PassSpec.Dependencies = {
			{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			},
			{
				.srcSubpass = 0,
				.dstSubpass = VK_SUBPASS_EXTERNAL,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			}
		};

		m_RendererData.GeometryPass = Pass( PassSpec );

		// Create geometry framebuffer.

		Renderer::Get().CreateFramebuffer( m_RendererData.GeometryPass, { .width = ( uint32_t ) Window::Get().Width(), .height = ( uint32_t ) Window::Get().Height() }, { m_RendererData.GeometryPassColor.ImageView, m_RendererData.GeometryPassDepth.ImageView }, &m_RendererData.GeometryFramebuffer );

		// Create the geometry pipeline.
		// Load the shader
		m_RendererData.StaticMeshShader = Ref< Shader >::Create( "shader_new", "assets/shaders/shader_new.glsl" );
		ShaderLibrary::Get().Add( m_RendererData.StaticMeshShader );
		
		VertexBufferLayout Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Bitangent" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};

		// Create the uniform buffer.
		VkDeviceSize BufferSize = sizeof( RendererData::StaticMeshMatrices );
		m_RendererData.SM_MatricesUBO = UniformBuffer();	

		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Normal ) } );
		AttributeDescriptions.push_back( { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Tangent ) } );
		AttributeDescriptions.push_back( { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Binormal ) } );
		AttributeDescriptions.push_back( { 4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( MeshVertex, Texcoord ) } );

		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );

		std::vector< VkDescriptorPoolSize > PoolSizes;

		// TODO: I want a nice API where I can use the same layout for the pool and the descriptor set layout.

		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );

		m_RendererData.GeometryDescriptorPool = DescriptorPool( PoolSizes, 100000 );

		// u_Matrices
		m_RendererData.SM_DescriptorSetLayout.Bindings.push_back( { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL } );
		
		// u_AlbedoTexture, u_NormalTexture, u_MetallicTexture, u_RoughnessTexture
		m_RendererData.SM_DescriptorSetLayout.Bindings.push_back( { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT } );
		m_RendererData.SM_DescriptorSetLayout.Bindings.push_back( { 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT } );
		m_RendererData.SM_DescriptorSetLayout.Bindings.push_back( { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT } );
		m_RendererData.SM_DescriptorSetLayout.Bindings.push_back( { 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT } );

		m_RendererData.SM_DescriptorSetLayout.Create();
		
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = Window::Get().Width();
		PipelineSpec.Height = Window::Get().Height();
		PipelineSpec.Name = "Static Meshes";
		PipelineSpec.pShader = m_RendererData.StaticMeshShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.SM_DescriptorSetLayout.VulkanLayout } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		
		m_RendererData.StaticMeshPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::RenderGrid()
	{
		// RENDER GRID
		{
			// BIND THE PIPELINE
			{
				vkCmdBindPipeline( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.GridPipeline.GetPipeline() );
			}

			// BIND THE DESCRIPTOR SET
			{
				vkCmdBindDescriptorSets( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.GridPipeline.GetPipelineLayout(), 0, 1, &m_RendererData.GridDescriptorSet, 0, nullptr );
			}

			// UPDATE UNIFORM BUFFERS
			{
				auto trans = glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) * glm::scale( glm::mat4( 1.0f ), glm::vec3( 16.0f ) );

				RendererData::GridMatricesObject GridMatricesObject = {};
				GridMatricesObject.Transform = trans;
				//GridMatricesObject.ViewProjection = VulkanContext::Get().GetEditorCamera().ViewProjection();

				GridMatricesObject.Res = 0.025f;
				GridMatricesObject.Scale = 16.025f;

				void* Data;

				m_RendererData.GridUniformBuffer.Map( &Data, sizeof( GridMatricesObject ) );

				memcpy( Data, &GridMatricesObject, sizeof( GridMatricesObject ) );
				
				m_RendererData.GridUniformBuffer.Unmap();
			}

			// DRAW
			{
				m_RendererData.GridVertexBuffer->Bind( m_RendererData.CommandBuffer );
				m_RendererData.GridIndexBuffer->Bind( m_RendererData.CommandBuffer );

				m_RendererData.GridIndexBuffer->Draw( m_RendererData.CommandBuffer );
			}
		}
	}

	void SceneRenderer::RenderSkybox()
	{
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;
		
		Entity SkylightEntity;

		auto view = m_pSence->GetAllEntitiesWith< SkylightComponent >();

		for ( const auto e : view )
		{
			SkylightEntity = { e, m_pSence };
		}

		if( SkylightEntity )
		{
			auto& Skylight = SkylightEntity.GetComponent< SkylightComponent >();
			
			if( !Skylight.DynamicSky )
				return;

			// BIND THE PIPELINE
			{
				vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.SkyboxPipeline.GetPipeline() );
			}

			// BIND THE DESCRIPTOR SET
			{
				vkCmdBindDescriptorSets( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.SkyboxPipeline.GetPipelineLayout(), 0, 1, &m_RendererData.SkyboxDescriptorSet, 0, nullptr );
			}

			// UPDATE UNIFORM BUFFERS
			{
				RendererData::SkyboxMatricesObject SkyboxMatricesObject = {};

				//SkyboxMatricesObject.View = VulkanContext::Get().GetEditorCamera().ViewMatrix();
				//SkyboxMatricesObject.Projection = VulkanContext::Get().GetEditorCamera().ProjectionMatrix();
				SkyboxMatricesObject.Turbidity = Skylight.Turbidity;
				SkyboxMatricesObject.Azimuth = Skylight.Azimuth;
				SkyboxMatricesObject.Inclination = Skylight.Inclination;

				void* Data;

				m_RendererData.SkyboxUniformBuffer.Map( &Data, sizeof( SkyboxMatricesObject ) );

				memcpy( Data, &SkyboxMatricesObject, sizeof( SkyboxMatricesObject ) );

				m_RendererData.SkyboxUniformBuffer.Unmap();
			}

			// DRAW
			{
				m_RendererData.SkyboxVertexBuffer->Bind( CommandBuffer );
				m_RendererData.SkyboxIndexBuffer->Bind( CommandBuffer );

				m_RendererData.SkyboxIndexBuffer->Draw( CommandBuffer );
			}
		}
	}

	void SceneRenderer::CreateGridComponents()
	{
		VertexBufferLayout Layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		// Create fullscreen quad.
		CreateFullscreenQuad( &m_RendererData.GridVertexBuffer, &m_RendererData.GridIndexBuffer );
		
		m_RendererData.GridShader = Ref< Shader >::Create( "Grid", "assets/shaders/Grid.glsl" );
		
		ShaderLibrary::Get().Add( m_RendererData.GridShader );

		// Create uniform buffer.
		VkDeviceSize BufferSize = sizeof( RendererData::GridMatricesObject );
		m_RendererData.GridUniformBuffer = Buffer();

		m_RendererData.GridUniformBuffer.Create( nullptr, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		
		// Create descriptor set layout.
		// TODO: Move to vulkan context.
		VkDescriptorSetLayoutBinding UBOLayoutBinding = {};
		UBOLayoutBinding.binding = 0;
		UBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UBOLayoutBinding.descriptorCount = 1;
		UBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		
		VkDescriptorSetLayoutCreateInfo LayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		LayoutCreateInfo.bindingCount = 1;
		LayoutCreateInfo.pBindings = &UBOLayoutBinding;
		
		VK_CHECK( vkCreateDescriptorSetLayout( VulkanContext::Get().GetDevice(), &LayoutCreateInfo, nullptr, &m_RendererData.GridDescriptorSetLayout ) );

		// Create descriptor pool.
		// TODO: Move to vulkan context as we may only need one pool.
		VkDescriptorPoolSize PoolSize = {};
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = 1;
		
		VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.poolSizeCount = 1;
		PoolCreateInfo.pPoolSizes = &PoolSize;
		PoolCreateInfo.maxSets = 1;
		
		VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_RendererData.GridDescriptorPool ) );

		// Create descriptor set.
		VkDescriptorSetAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		AllocateInfo.descriptorPool = m_RendererData.GridDescriptorPool;
		AllocateInfo.descriptorSetCount = 1;
		AllocateInfo.pSetLayouts = &m_RendererData.GridDescriptorSetLayout;
		
		VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &AllocateInfo, &m_RendererData.GridDescriptorSet ) );
		
		// Create descriptor write.
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = m_RendererData.GridUniformBuffer;
		BufferInfo.offset = 0;
		BufferInfo.range = BufferSize;
		
		VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		Write.dstSet = m_RendererData.GridDescriptorSet;
		Write.dstBinding = 0;
		Write.dstArrayElement = 0;
		Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Write.descriptorCount = 1;
		Write.pBufferInfo = &BufferInfo;
		
		vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &Write, 0, nullptr );

		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( GridVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( GridVertex, Texcoord ) } );

		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );

		// Grid pipeline spec.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = Window::Get().Width();
		PipelineSpec.Height = Window::Get().Height();
		PipelineSpec.Name = "Grid";
		PipelineSpec.pShader = m_RendererData.GridShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.GridDescriptorSetLayout } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		PipelineSpec.UseDepthTest = true;
		
		m_RendererData.GridPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::DestroyGridComponents()
	{
		// Destroy grid pipeline.
		m_RendererData.GridPipeline.Terminate();

		// Destroy uniform buffer.
		m_RendererData.GridUniformBuffer.Terminate();

		// Destroy grid index buffer.
		m_RendererData.GridIndexBuffer->Terminate();

		// Destroy grid vertex buffer.
		m_RendererData.GridVertexBuffer->Terminate();

		// Destroy descriptor set layout.
		vkDestroyDescriptorSetLayout( VulkanContext::Get().GetDevice(), m_RendererData.GridDescriptorSetLayout, nullptr );

		// Destroy descriptor pool.
		vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_RendererData.GridDescriptorPool, nullptr );
	}

	//////////////////////////////////////////////////////////////////////////
	// Skybox
	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::CreateSkyboxComponents()
	{		
		VertexBufferLayout Layout = 
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};

		// Create fullscreen quad.
		CreateFullscreenQuad( &m_RendererData.SkyboxVertexBuffer, &m_RendererData.SkyboxIndexBuffer );

		// Create skybox shader.
		m_RendererData.SkyboxShader = Ref<Shader>::Create( "Skybox", "assets/shaders/Skybox.glsl" );

		ShaderLibrary::Get().Add( m_RendererData.SkyboxShader );

		// Create uniform buffer.
		VkDeviceSize BufferSize = sizeof( RendererData::SkyboxMatricesObject );

		m_RendererData.SkyboxUniformBuffer = Buffer();

		m_RendererData.SkyboxUniformBuffer.Create( nullptr, BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		
		// Create descriptor set layout.
		VkDescriptorSetLayoutBinding BindingLayout = {};
		BindingLayout.binding = 0;
		BindingLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		BindingLayout.descriptorCount = 1;
		BindingLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector< VkDescriptorSetLayoutBinding > Bindings = { BindingLayout };
		
		VkDescriptorSetLayoutCreateInfo LayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		LayoutInfo.bindingCount = Bindings.size();
		LayoutInfo.pBindings = Bindings.data();
		
		VK_CHECK( vkCreateDescriptorSetLayout( VulkanContext::Get().GetDevice(), &LayoutInfo, nullptr, &m_RendererData.SkyboxDescriptorSetLayout ) );

		// Create descriptor pool.
		VkDescriptorPoolSize PoolSize = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo PoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.poolSizeCount = 1;
		PoolCreateInfo.pPoolSizes = &PoolSize;
		PoolCreateInfo.maxSets = 1;
		
		VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_RendererData.SkyboxDescriptorPool ) );

		// Create descriptor set.
		VkDescriptorSetAllocateInfo AllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		AllocateInfo.descriptorPool = m_RendererData.SkyboxDescriptorPool;
		AllocateInfo.descriptorSetCount = 1;
		AllocateInfo.pSetLayouts = &m_RendererData.SkyboxDescriptorSetLayout;

		VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &AllocateInfo, &m_RendererData.SkyboxDescriptorSet ) );

		// Create descriptor write.
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = m_RendererData.SkyboxUniformBuffer;
		BufferInfo.offset = 0;
		BufferInfo.range = BufferSize;
		
		VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		Write.dstSet = m_RendererData.SkyboxDescriptorSet;
		Write.dstBinding = 0;
		Write.dstArrayElement = 0;
		Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Write.descriptorCount = 1;
		Write.pBufferInfo = &BufferInfo;
		
		vkUpdateDescriptorSets( VulkanContext::Get().GetDevice(), 1, &Write, 0, nullptr );
		
		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;

		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( BaseVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( BaseVertex, Texcoord ) } );
		
		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );
		
		// Create pipeline.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = Window::Get().Width();
		PipelineSpec.Height = Window::Get().Height();
		PipelineSpec.Name = "Skybox";
		PipelineSpec.pShader = m_RendererData.SkyboxShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.SkyboxDescriptorSetLayout } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		
		m_RendererData.SkyboxPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::DestroySkyboxComponents()
	{
		// Destroy Skybox pipeline.
		m_RendererData.SkyboxPipeline.Terminate();

		// Destroy uniform buffer.
		m_RendererData.SkyboxUniformBuffer.Terminate();

		// Destroy Skybox index buffer.
		m_RendererData.SkyboxIndexBuffer->Terminate();

		// Destroy Skybox vertex buffer.
		m_RendererData.SkyboxVertexBuffer->Terminate();

		// Destroy descriptor set layout.
		vkDestroyDescriptorSetLayout( VulkanContext::Get().GetDevice(), m_RendererData.SkyboxDescriptorSetLayout, nullptr );

		// Destroy descriptor pool.
		vkDestroyDescriptorPool( VulkanContext::Get().GetDevice(), m_RendererData.SkyboxDescriptorPool, nullptr );
	}

	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::CreateFullscreenQuad( VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer )
	{
		CreateFullscreenQuad( -1, -1, 2, 2, ppVertexBuffer, ppIndexBuffer );
	}

	void SceneRenderer::CreateFullscreenQuad( float x, float y, float w, float h, 
		VertexBuffer** ppVertexBuffer, IndexBuffer** ppIndexBuffer )
	{
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[ 4 ];

		data[ 0 ].Position = glm::vec3( x, y, 0.1f );
		data[ 0 ].TexCoord = glm::vec2( 0, 0 );

		data[ 1 ].Position = glm::vec3( x + w, y, 0.1f );
		data[ 1 ].TexCoord = glm::vec2( 1, 0 );

		data[ 2 ].Position = glm::vec3( x + w, y + h, 0.1f );
		data[ 2 ].TexCoord = glm::vec2( 1, 1 );

		data[ 3 ].Position = glm::vec3( x, y + h, 0.1f );
		data[ 3 ].TexCoord = glm::vec2( 0, 1 );

		*ppVertexBuffer = new VertexBuffer( data, 4 * sizeof( QuadVertex ) );

		uint32_t indices[ 6 ] = { 0, 1, 2,
								  2, 3, 0, };

		*ppIndexBuffer = new IndexBuffer( indices, 6 );
	}

	void SceneRenderer::AddDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		m_DrawList.push_back( { entity, mesh, transform } );
	}

	void SceneRenderer::RenderDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		auto& uuid = entity.GetComponent<IdComponent>().ID;

		//vkCmdBindPipeline( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanContext::Get().GetPipeline().GetPipeline() );
		
		// Render a draw command.

		// Bind vertex and index buffers.
		mesh->GetVertexBuffer()->Bind( m_RendererData.CommandBuffer );
		mesh->GetIndexBuffer()->Bind( m_RendererData.CommandBuffer );

		mesh->GetMaterial()->Bind( nullptr );

		// Bind the descriptor sets.
		//vkCmdBindDescriptorSets( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, //VulkanContext::Get().GetPipeline().GetPipelineLayout(), 0, 1, &VulkanContext::Get().GetDescriptorSets()[ uuid ], 0, nullptr );
		

		// Update uniform buffers.
		//VulkanContext::Get().UpdateUniformBuffers( entity.GetComponent<IdComponent>().ID, Application::Get().Time(), entity.GetComponent<TransformComponent>().GetTransform() );

		// Draw.
		mesh->GetIndexBuffer()->Draw( m_RendererData.CommandBuffer );
	}

	void SceneRenderer::FlushDrawList()
	{
		m_DrawList.clear();
	}

	void SceneRenderer::GeometryPass()
	{
		VkExtent2D Extent = { Window::Get().Width(), Window::Get().Height() };

		// Begin geometry pass.
		m_RendererData.GeometryPass.BeginPass( m_RendererData.CommandBuffer, m_RendererData.GeometryFramebuffer, Extent );
		
		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = 0;
		Viewport.width = ( float ) Window::Get().Width();
		Viewport.height = ( float ) Window::Get().Height();
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;
		
		VkRect2D Scissor = { .offset = { 0, 0 }, .extent = Extent };
		
		vkCmdSetViewport( m_RendererData.CommandBuffer, 0, 1, &Viewport );
		vkCmdSetScissor( m_RendererData.CommandBuffer, 0, 1, &Scissor );

		//////////////////////////////////////////////////////////////////////////
		// Actual geometry pass.
		//////////////////////////////////////////////////////////////////////////

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Skybox" );

		RenderSkybox();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Grid" );

		RenderGrid();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Static meshes" );

		// Render static meshes.
		for( auto& Cmd : m_DrawList )
		{
			auto& uuid = Cmd.entity.GetComponent<IdComponent>().ID;

			// Update uniform buffers.

			RendererData::StaticMeshMatrices u_Matrices = {};
			
			u_Matrices.Transform = Cmd.Transform;
			u_Matrices.ViewProjection = m_RendererData.EditorCamera.ViewProjection();
			u_Matrices.UseAlbedoTexture = true;
			
			m_RendererData.SM_MatricesUBO.Update( &u_Matrices, sizeof( u_Matrices ) );

			Renderer::Get().RenderStaticMesh( 
				m_RendererData.CommandBuffer, 
				m_RendererData.StaticMeshPipeline, 
				uuid, Cmd.Mesh, Cmd.Transform, 
				m_RendererData.SM_MatricesUBO );
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		//////////////////////////////////////////////////////////////////////////
		
		m_RendererData.GeometryPass.EndPass();

		// End geometry pass.
	}

	void SceneRenderer::RenderScene()
	{
		Renderer::Get().BeginFrame();

		// Allocate a default command buffer.
		m_RendererData.CommandBuffer = Renderer::Get().AllocateCommandBuffer( m_RendererData.CommandPool );

		GeometryPass();

		Renderer::Get().EndFrame( m_RendererData.CommandBuffer );

		vkFreeCommandBuffers( VulkanContext::Get().GetDevice(), m_RendererData.CommandPool, 1, &m_RendererData.CommandBuffer );

		FlushDrawList();
	}

	void SceneRenderer::AddDescriptorSet( const DescriptorSet& rDescriptorSet )
	{
		// Check if the descriptor set already exists.
		auto res = std::find( std::begin( m_RendererData.StaticMeshDescriptorSets ), std::end( m_RendererData.StaticMeshDescriptorSets ), rDescriptorSet );
	
		if( res == std::end( m_RendererData.StaticMeshDescriptorSets ) )
		{
			m_RendererData.StaticMeshDescriptorSets.push_back( rDescriptorSet );
		}
	}

}