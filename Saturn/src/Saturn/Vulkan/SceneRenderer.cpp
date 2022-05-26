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
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <backends/imgui_impl_vulkan.h>

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

		// Create grid.
		CreateGridComponents();

		// Create skybox.
		CreateSkyboxComponents();

		InitSceneComposite();

		//////////////////////////////////////////////////////////////////////////
	}

	void SceneRenderer::CreateGeometryResult()
	{
		m_RendererData.RenderPassResult = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_RendererData.GeometryPassColor.Sampler, m_RendererData.GeometryPassColor.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

		m_RendererData.SceneCompositeResult = ( VkDescriptorSet ) ImGui_ImplVulkan_AddTexture( m_RendererData.SceneCompositeColor.Sampler, m_RendererData.SceneCompositeColor.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
	}

	void SceneRenderer::Terminate()
	{
		// Destroy grid.
		DestroyGridComponents();

		DestroySkyboxComponents();

		m_pSence = nullptr;
		
		m_DrawList.clear();
		
		m_RendererData.Terminate();
	}

	void SceneRenderer::InitGeometryPass()
	{
		// Create render pass.

		// Start by creating the color resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, { .width = ( uint32_t )m_RendererData.Width, .height = ( uint32_t )m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &m_RendererData.GeometryPassColor.Image, &m_RendererData.GeometryPassColor.Memory );

			Renderer::Get().CreateImageView( m_RendererData.GeometryPassColor, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_RendererData.GeometryPassColor.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.GeometryPassColor.Sampler );
		}

		// Then, create the depth resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, { .width = ( uint32_t )m_RendererData.Width, .height = ( uint32_t )m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_RendererData.GeometryPassDepth.Image, &m_RendererData.GeometryPassDepth.Memory );

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

		Renderer::Get().CreateFramebuffer( m_RendererData.GeometryPass, { .width = ( uint32_t )m_RendererData.Width, .height = ( uint32_t )m_RendererData.Height }, { m_RendererData.GeometryPassColor.ImageView, m_RendererData.GeometryPassDepth.ImageView }, &m_RendererData.GeometryFramebuffer );
		
		//////////////////////////////////////////////////////////////////////////
		// STATIC MESHES
		//////////////////////////////////////////////////////////////////////////

		// Create the static meshes pipeline.
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
		//m_RendererData.SM_MatricesUBO = UniformBuffer();	

		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Normal ) } );
		AttributeDescriptions.push_back( { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Tangent ) } );
		AttributeDescriptions.push_back( { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( MeshVertex, Binormal ) } );
		AttributeDescriptions.push_back( { 4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( MeshVertex, Texcoord ) } );

		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );


		std::vector< VkPushConstantRange > PushConstants;
		PushConstants.push_back( { .stageFlags = VK_SHADER_STAGE_ALL, .offset = 0, .size = sizeof( RendererData::StaticMeshMaterial ) } );

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width =m_RendererData.Width;
		PipelineSpec.Height =m_RendererData.Height;
		PipelineSpec.Name = "Static Meshes";
		PipelineSpec.pShader = m_RendererData.StaticMeshShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.StaticMeshShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.CullMode = VK_CULL_MODE_NONE;
		PipelineSpec.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		PipelineSpec.Layout.PushConstants = { PushConstants };

		m_RendererData.StaticMeshPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::InitSceneComposite()
	{
		// Start by creating the color resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, { .width = ( uint32_t )m_RendererData.Width, .height = ( uint32_t )m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &m_RendererData.SceneCompositeColor.Image, &m_RendererData.SceneCompositeColor.Memory );

			Renderer::Get().CreateImageView( m_RendererData.SceneCompositeColor, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_RendererData.SceneCompositeColor.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.SceneCompositeColor.Sampler );
		}

		// Then, create the depth resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, { .width = ( uint32_t )m_RendererData.Width, .height = ( uint32_t )m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_RendererData.SceneCompositeDepth.Image, &m_RendererData.SceneCompositeDepth.Memory );

			Renderer::Get().CreateImageView( m_RendererData.SceneCompositeDepth, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, &m_RendererData.SceneCompositeDepth.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.SceneCompositeDepth.Sampler );
		}


		// Create the scene composite render pass.
		PassSpecification PassSpec = {};
		PassSpec.Name = "Scene Composite";
		
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

		m_RendererData.SceneComposite = Pass( PassSpec );
		
		Renderer::Get().CreateFramebuffer( 
			m_RendererData.GeometryPass, 
			{ .width = m_RendererData.Width, .height = m_RendererData.Height },
			{ m_RendererData.SceneCompositeColor.ImageView, m_RendererData.SceneCompositeDepth.ImageView },
			&m_RendererData.SceneCompositeFramebuffer );

		/////////

		// Create vertex buffer layout.
		VertexBufferLayout Layout =
		{
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};
		
		// Create fullscreen quad.
		CreateFullscreenQuad( &m_RendererData.SC_VertexBuffer, &m_RendererData.SC_IndexBuffer );

		m_RendererData.SceneCompositeShader = Ref< Shader >::Create( "SceneComposite", "assets/shaders/SceneComposite.glsl" );

		ShaderLibrary::Get().Add( m_RendererData.SceneCompositeShader );
		
		DescriptorSetSpecification Spec;
		Spec.Layout = m_RendererData.SceneCompositeShader->GetSetLayout();
		Spec.Pool = m_RendererData.SceneCompositeShader->GetDescriptorPool();

		m_RendererData.SC_DescriptorSet = Ref< DescriptorSet >::Create( Spec );

		VkDescriptorImageInfo GeometryPassImageInfo = {};
		GeometryPassImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		GeometryPassImageInfo.imageView = m_RendererData.GeometryPassColor.ImageView;
		GeometryPassImageInfo.sampler = m_RendererData.GeometryPassColor.Sampler;

		std::vector< VkWriteDescriptorSet > DescriptorWrites;

		DescriptorWrites.push_back( {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_RendererData.SC_DescriptorSet->GetVulkanSet(),
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &GeometryPassImageInfo,
			.pBufferInfo = nullptr,
			.pTexelBufferView = nullptr } );

		m_RendererData.SC_DescriptorSet->Write( DescriptorWrites );

		// Create attribute descriptions & binding descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;

		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( BaseVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( BaseVertex, Texcoord ) } );
		
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Scene Composite";
		PipelineSpec.pShader = m_RendererData.SceneCompositeShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.SceneCompositeShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.SceneComposite;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.CullMode = VK_CULL_MODE_NONE;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;

		m_RendererData.SceneCompositePipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::RenderGrid()
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// RENDER GRID
		{
			// BIND THE PIPELINE
			{
				vkCmdBindPipeline( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.GridPipeline.GetPipeline() );
			}

			// BIND THE DESCRIPTOR SET
			{
				m_RendererData.GridDescriptorSet->Bind( m_RendererData.CommandBuffer, m_RendererData.GridPipeline.GetPipelineLayout() );
			}

			// UPDATE UNIFORM BUFFERS
			{
				glm::mat4 trans = glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) * glm::scale( glm::mat4( 1.0f ), glm::vec3( 16.0f ) );

				RendererData::GridMatricesObject GridMatricesObject = {};
				GridMatricesObject.Transform = trans;
				GridMatricesObject.ViewProjection = m_RendererData.EditorCamera.ViewProjection();
				
				GridMatricesObject.Res = 0.025f;
				GridMatricesObject.Scale = 16.025f;
				
				auto bufferAloc = pAllocator->GetAllocationFromBuffer( m_RendererData.GridUniformBuffer );

				void* dstData = pAllocator->MapMemory< void >( bufferAloc );

				memcpy( dstData, &GridMatricesObject, sizeof( GridMatricesObject ) );

				pAllocator->UnmapMemory( bufferAloc );
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
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		Entity SkylightEntity;

		if( !m_pSence )
			return;

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
				m_RendererData.SkyboxDescriptorSet->Bind( CommandBuffer, m_RendererData.SkyboxPipeline.GetPipelineLayout() );
			}

			// UPDATE UNIFORM BUFFERS
			{
				RendererData::SkyboxMatricesObject SkyboxMatricesObject = {};

				SkyboxMatricesObject.View = m_RendererData.EditorCamera.ViewMatrix();
				SkyboxMatricesObject.Projection = m_RendererData.EditorCamera.ProjectionMatrix();
				SkyboxMatricesObject.Turbidity = Skylight.Turbidity;
				SkyboxMatricesObject.Azimuth = Skylight.Azimuth;
				SkyboxMatricesObject.Inclination = Skylight.Inclination;

				auto bufferAloc = pAllocator->GetAllocationFromBuffer( m_RendererData.SkyboxUniformBuffer );

				void* Data = pAllocator->MapMemory< void >( bufferAloc );

				memcpy( Data, &SkyboxMatricesObject, sizeof( SkyboxMatricesObject ) );

				pAllocator->UnmapMemory( bufferAloc );
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
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

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

		VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferInfo.size = BufferSize;
		BufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		pAllocator->AllocateBuffer( BufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, &m_RendererData.GridUniformBuffer );
	
		DescriptorSetSpecification Spec = {};
		Spec.Layout = m_RendererData.GridShader->GetSetLayout();
		Spec.Pool = m_RendererData.GridShader->GetDescriptorPool();
		
		m_RendererData.GridDescriptorSet = Ref< DescriptorSet >::Create( Spec );

		VkDescriptorBufferInfo DescriptorBufferInfo = {};
		DescriptorBufferInfo.buffer = m_RendererData.GridUniformBuffer;
		DescriptorBufferInfo.offset = 0;
		DescriptorBufferInfo.range = BufferSize;

		m_RendererData.GridDescriptorSet->Write( DescriptorBufferInfo, {} );

		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( BaseVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( BaseVertex, Texcoord ) } );

		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );

		// Grid pipeline spec.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width =m_RendererData.Width;
		PipelineSpec.Height =m_RendererData.Height;
		PipelineSpec.Name = "Grid";
		PipelineSpec.pShader = m_RendererData.GridShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.GridShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		
		m_RendererData.GridPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::DestroyGridComponents()
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// Destroy grid pipeline.
		m_RendererData.GridPipeline.Terminate();

		// Destroy uniform buffer.
		pAllocator->DestroyBuffer( m_RendererData.GridUniformBuffer );

		// Destroy grid index buffer.
		m_RendererData.GridIndexBuffer->Terminate();

		// Destroy grid vertex buffer.
		m_RendererData.GridVertexBuffer->Terminate();

		m_RendererData.SkyboxDescriptorSet->Terminate();
	}

	//////////////////////////////////////////////////////////////////////////
	// Skybox
	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::CreateSkyboxComponents()
	{		
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

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
		
		VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferInfo.size = BufferSize;
		BufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		pAllocator->AllocateBuffer( BufferInfo, VMA_MEMORY_USAGE_CPU_ONLY, &m_RendererData.SkyboxUniformBuffer );
		
		DescriptorSetSpecification Spec = {};
		Spec.Layout = m_RendererData.SkyboxShader->GetSetLayout();
		Spec.Pool = m_RendererData.SkyboxShader->GetDescriptorPool();

		m_RendererData.SkyboxDescriptorSet = Ref< DescriptorSet >::Create( Spec );
		
		VkDescriptorBufferInfo DescriptorBufferInfo = {};
		DescriptorBufferInfo.buffer = m_RendererData.SkyboxUniformBuffer;
		DescriptorBufferInfo.offset = 0;
		DescriptorBufferInfo.range = BufferSize;

		m_RendererData.SkyboxDescriptorSet->Write( DescriptorBufferInfo, {} );
		
		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;

		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( BaseVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( BaseVertex, Texcoord ) } );
		
		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );
		
		// Create pipeline.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Skybox";
		PipelineSpec.pShader = m_RendererData.SkyboxShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.SkyboxShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.AttributeDescriptions = AttributeDescriptions;
		PipelineSpec.BindingDescriptions = BindingDescriptions;
		PipelineSpec.CullMode = VK_CULL_MODE_BACK_BIT;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;

		m_RendererData.SkyboxPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::DestroySkyboxComponents()
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// Destroy Skybox pipeline.
		m_RendererData.SkyboxPipeline.Terminate();

		// Destroy uniform buffer.
		pAllocator->DestroyBuffer( m_RendererData.SkyboxUniformBuffer );

		// Destroy Skybox index buffer.
		m_RendererData.SkyboxIndexBuffer->Terminate();

		// Destroy Skybox vertex buffer.
		m_RendererData.SkyboxVertexBuffer->Terminate();

		m_RendererData.SkyboxDescriptorSet->Terminate();
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

	void SceneRenderer::ImGuiRender()
	{
		ImGui::Begin( "Scene Renderer" );
		
		auto FrameTimings = Renderer::Get().GetFrameTimings();

		ImGui::Text( "Renderer::BeginFrame: %.2f ms", FrameTimings.first );
		
		ImGui::Text( "SceneRenderer::GeometryPass: %.2f ms", m_RendererData.GeometryPassTimer.ElapsedMilliseconds() );

		ImGui::Text( "Renderer::EndFrame - Queue Present: %.2f ms", Renderer::Get().GetQueuePresentTime() );

		ImGui::Text( "Renderer::EndFrame: %.2f ms", FrameTimings.second );
		
		ImGui::Text( "Total : %.2f ms", Application::Get().Time().Milliseconds() );
		
		ImGui::End();
	}

	void SceneRenderer::AddDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		m_DrawList.push_back( { entity, mesh, transform } );
	}

	void SceneRenderer::FlushDrawList()
	{
		m_DrawList.clear();
	}

	void SceneRenderer::Recreate()
	{
		VkDevice LogicalDevice = VulkanContext::Get().GetDevice();

		// Terminate render passes.
		m_RendererData.GeometryPass.Terminate();
		m_RendererData.SceneComposite.Terminate();

		// Terminate resources.
		m_RendererData.GeometryPassDepth.Terminate();
		m_RendererData.GeometryPassColor.Terminate();
		m_RendererData.SceneCompositeColor.Terminate();
		m_RendererData.SceneCompositeDepth.Terminate();

		vkDestroyFramebuffer( LogicalDevice, m_RendererData.GeometryFramebuffer, nullptr );
		vkDestroyFramebuffer( LogicalDevice, m_RendererData.SceneCompositeFramebuffer, nullptr );

		// Terminate Pipelines
		m_RendererData.StaticMeshPipeline.Terminate();
		m_RendererData.SceneCompositePipeline.Terminate();
		m_RendererData.GridPipeline.Terminate();
		m_RendererData.SkyboxPipeline.Terminate();

		/*

		// Create render passes.
		// Start by creating the color resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, { .width = ( uint32_t ) m_RendererData.Width, .height = ( uint32_t ) m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &m_RendererData.GeometryPassColor.Image, &m_RendererData.GeometryPassColor.Memory );

			Renderer::Get().CreateImageView( m_RendererData.GeometryPassColor, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_RendererData.GeometryPassColor.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.GeometryPassColor.Sampler );
		}

		// Then, create the depth resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, { .width = ( uint32_t ) m_RendererData.Width, .height = ( uint32_t ) m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_RendererData.GeometryPassDepth.Image, &m_RendererData.GeometryPassDepth.Memory );

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

		//
		// Start by creating the color resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, { .width = ( uint32_t ) m_RendererData.Width, .height = ( uint32_t ) m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &m_RendererData.SceneCompositeColor.Image, &m_RendererData.SceneCompositeColor.Memory );

			Renderer::Get().CreateImageView( m_RendererData.SceneCompositeColor, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_RendererData.SceneCompositeColor.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.SceneCompositeColor.Sampler );
		}

		// Then, create the depth resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, { .width = ( uint32_t ) m_RendererData.Width, .height = ( uint32_t ) m_RendererData.Height, .depth = 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_RendererData.SceneCompositeDepth.Image, &m_RendererData.SceneCompositeDepth.Memory );

			Renderer::Get().CreateImageView( m_RendererData.SceneCompositeDepth, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, &m_RendererData.SceneCompositeDepth.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.SceneCompositeDepth.Sampler );
		}

		Renderer::Get().CreateFramebuffer( m_RendererData.GeometryPass, { .width = ( uint32_t ) m_RendererData.Width, .height = ( uint32_t ) m_RendererData.Height }, { m_RendererData.GeometryPassColor.ImageView, m_RendererData.GeometryPassDepth.ImageView }, & m_RendererData.GeometryFramebuffer );


		// Create the scene composite render pass.
		PassSpecification PassSpec = {};
		PassSpec.Name = "Scene Composite";

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

		m_RendererData.SceneComposite = Pass( PassSpec );

		Renderer::Get().CreateFramebuffer(
			m_RendererData.GeometryPass,
			{ .width = m_RendererData.Width, .height = m_RendererData.Height },
			{ m_RendererData.SceneCompositeColor.ImageView, m_RendererData.SceneCompositeDepth.ImageView },
			& m_RendererData.SceneCompositeFramebuffer );
		
		// Create pipelines
		*/
	}

	void SceneRenderer::GeometryPass()
	{
		m_RendererData.GeometryPassTimer.Reset();

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		VkExtent2D Extent = { m_RendererData.Width, m_RendererData.Height };

		// Begin geometry pass.
		m_RendererData.GeometryPass.BeginPass( m_RendererData.CommandBuffer, m_RendererData.GeometryFramebuffer, Extent );
		
		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = 0;
		Viewport.width = ( float )m_RendererData.Width;
		Viewport.height = ( float )m_RendererData.Height;
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
		Ref< Shader > StaticMeshShader = m_RendererData.StaticMeshShader;
		for( auto& Cmd : m_DrawList )
		{
			auto& uuid = Cmd.entity.GetComponent<IdComponent>().ID;
			auto& rMaterial = Cmd.Mesh->GetMaterial();
			auto& UBs = StaticMeshShader->GetUniformBuffers();

			// u_Matrices
			RendererData::StaticMeshMatrices u_Matrices = {};
			u_Matrices.ViewProjection = m_RendererData.EditorCamera.ViewProjection();

			auto bufferAloc = pAllocator->GetAllocationFromBuffer( UBs[ ShaderType::All ][ 0 ].Buffer );

			void* pData = pAllocator->MapMemory< void >( bufferAloc );

			memcpy( pData, &u_Matrices, sizeof( u_Matrices ) );

			pAllocator->UnmapMemory( bufferAloc );
			
			for ( Submesh& rSubmesh : Cmd.Mesh->Submeshes() )
			{
				Ref< DescriptorSet > Set = Cmd.Mesh->GetDescriptorSets()[ rSubmesh ];
				StaticMeshShader->WriteAllUBs( Set );

				// Bind vertex and index buffers.
				Cmd.Mesh->GetVertexBuffer()->Bind( m_RendererData.CommandBuffer );
				Cmd.Mesh->GetIndexBuffer()->Bind( m_RendererData.CommandBuffer );

				m_RendererData.StaticMeshPipeline.Bind( m_RendererData.CommandBuffer );

				Set->Bind( m_RendererData.CommandBuffer, m_RendererData.StaticMeshPipeline.GetPipelineLayout() );
				
				// Write push constant data.
				RendererData::StaticMeshMaterial u_Materials = {};
				u_Materials.Transform = Cmd.Transform * rSubmesh.Transform;
				u_Materials.UseNormalTexture = rMaterial->Get<float>( "u_Materials.UseNormalTexture" ) ? 1.0f : 0.0f;
				u_Materials.UseMetallicTexture = rMaterial->Get<float>( "u_Materials.UseMetallicTexture" ) ? 1.0f : 0.0f;
				u_Materials.UseRoughnessTexture = rMaterial->Get<float>( "u_Materials.UseRoughnessTexture" ) ? 1.0f : 0.5f;
				u_Materials.UseAlbedoTexture = rMaterial->Get<float>( "u_Materials.UseAlbedoTexture" ) ? 1.0f : 0.0f;

				u_Materials.AlbedoColor = rMaterial->Get<glm::vec4>( "u_Materials.AlbedoColor" );
				u_Materials.MetallicColor = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
				u_Materials.RoughnessColor = glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );

				vkCmdPushConstants( m_RendererData.CommandBuffer, m_RendererData.StaticMeshPipeline.GetPipelineLayout(), VK_SHADER_STAGE_ALL, 0, sizeof( u_Materials ), &u_Materials );
				
				Renderer::Get().RenderSubmesh( m_RendererData.CommandBuffer, 
					m_RendererData.StaticMeshPipeline, 
					Cmd.Mesh, rSubmesh, u_Materials.Transform );
			}
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		//////////////////////////////////////////////////////////////////////////
		
		// End geometry pass.
		m_RendererData.GeometryPass.EndPass();

		m_RendererData.GeometryPassTimer.Stop();
	}

	void SceneRenderer::SceneCompositePass()
	{
		VkExtent2D Extent = {m_RendererData.Width,m_RendererData.Height };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		// Begin scene composite pass.
		m_RendererData.SceneComposite.BeginPass( CommandBuffer, m_RendererData.SceneCompositeFramebuffer, Extent );

		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = ( float )m_RendererData.Height;
		Viewport.width = ( float )m_RendererData.Width;
		Viewport.height = -( float )m_RendererData.Height;
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;

		VkRect2D Scissor = { .offset = { 0, 0 }, .extent = Extent };

		vkCmdSetViewport( CommandBuffer, 0, 1, &Viewport );
		vkCmdSetScissor( CommandBuffer, 0, 1, &Scissor );
		
		// Actual scene composite pass.

		vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RendererData.SceneCompositePipeline.GetPipeline() );

		m_RendererData.SC_DescriptorSet->Bind( CommandBuffer, m_RendererData.SceneCompositePipeline.GetPipelineLayout() );

		m_RendererData.SC_VertexBuffer->Bind( CommandBuffer );
		m_RendererData.SC_IndexBuffer->Bind( CommandBuffer );

		m_RendererData.SC_IndexBuffer->Draw( CommandBuffer );
		
		// End scene composite pass.
		m_RendererData.SceneComposite.EndPass();
	}

	void SceneRenderer::RenderScene()
	{
		if( !m_pSence )
		{
			FlushDrawList();
			return;
		}

		/*
		// Cleanup descriptor sets from last frame.
		if ( m_RendererData.StaticMeshDescriptorSets.size() >= 1 )
		{
			for ( auto&& [uid, set] : m_RendererData.StaticMeshDescriptorSets )
			{
				m_RendererData.StaticMeshDescriptorSets[ uid ].Terminate();
			}

			m_RendererData.StaticMeshDescriptorSets.clear();
		}
		*/

		m_RendererData.CommandBuffer = Renderer::Get().ActiveCommandBuffer();

		// Passes

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Geometry" );
		
		GeometryPass();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );
		
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Scene Composite 1st pass" );

		SceneCompositePass();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		//
		
		FlushDrawList();
	}

	void SceneRenderer::SetEditorCamera( const EditorCamera& Camera )
	{
		m_RendererData.EditorCamera = Camera;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// RendererData
	//////////////////////////////////////////////////////////////////////////

	void RendererData::Terminate()
	{
		VkDevice LogicalDevice = VulkanContext::Get().GetDevice();
		
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		GeometryPass.Terminate();
		GeometryPassDepth.Terminate();
		GeometryPassColor.Terminate();

		vkDestroyFramebuffer( LogicalDevice, GeometryFramebuffer, nullptr );
		GeometryFramebuffer = nullptr;

		StaticMeshPipeline.Terminate();

		GridPipeline.Terminate();

		GridDescriptorSet->Terminate();

		GridVertexBuffer->Terminate();
		GridIndexBuffer->Terminate();

		SkyboxPipeline.Terminate();
		SkyboxDescriptorSet->Terminate();

		SkyboxVertexBuffer->Terminate();
		SkyboxIndexBuffer->Terminate();

		SceneComposite.Terminate();
		SceneCompositeDepth.Terminate();
		SceneCompositeColor.Terminate();

		vkDestroyFramebuffer( LogicalDevice, SceneCompositeFramebuffer, nullptr );

		SceneCompositePipeline.Terminate();
		
		SC_VertexBuffer->Terminate();
		SC_IndexBuffer->Terminate();

		vkDestroyCommandPool( LogicalDevice, CommandPool, nullptr );
		
		ImGui_ImplVulkan_RemoveTexture( SceneCompositeResult );

		GridShader = nullptr;
		SkyboxShader = nullptr;
		StaticMeshShader = nullptr;
	}

}