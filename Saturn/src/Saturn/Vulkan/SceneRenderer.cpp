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

		std::vector< VkDescriptorPoolSize > PoolSizes;

		// TODO: I want a nice API where I can use the same layout for the pool and the descriptor set layout.

		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000 } );

		m_RendererData.GeometryDescriptorPool = Ref< DescriptorPool >::Create( PoolSizes, 100000 );

		// u_Matrices
		m_RendererData.SM_DescriptorSetLayout.Bindings.push_back( { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL } );

		m_RendererData.SM_MatricesUBO.CreateBuffer( sizeof( RendererData::StaticMeshMatrices ) );
		
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
		PipelineSpec.CullMode = VK_CULL_MODE_NONE;
		PipelineSpec.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		
		m_RendererData.StaticMeshPipeline = Pipeline( PipelineSpec );
	}

	void SceneRenderer::InitSceneComposite()
	{
		// Start by creating the color resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_B8G8R8A8_UNORM, { .width = ( uint32_t ) Window::Get().Width(), .height = ( uint32_t ) Window::Get().Height(), .depth = 1 }, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &m_RendererData.SceneCompositeColor.Image, &m_RendererData.SceneCompositeColor.Memory );

			Renderer::Get().CreateImageView( m_RendererData.SceneCompositeColor, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &m_RendererData.SceneCompositeColor.ImageView );

			Renderer::Get().CreateSampler( VK_FILTER_LINEAR, &m_RendererData.SceneCompositeColor.Sampler );
		}

		// Then, create the depth resource
		{
			Renderer::Get().CreateImage(
				VK_IMAGE_TYPE_2D, VK_FORMAT_D32_SFLOAT, { .width = ( uint32_t ) Window::Get().Width(), .height = ( uint32_t ) Window::Get().Height(), .depth = 1 }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &m_RendererData.SceneCompositeDepth.Image, &m_RendererData.SceneCompositeDepth.Memory );

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
			{ .width = ( uint32_t ) Window::Get().Width(), .height = ( uint32_t ) Window::Get().Height() },
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
		
		// Create the descriptor pool.
		std::vector< VkDescriptorPoolSize > PoolSizes;
		PoolSizes.push_back( { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 } );
		
		m_RendererData.SC_DescriptorPool = Ref< DescriptorPool >::Create( PoolSizes, 1 );

		// Textures
		m_RendererData.SC_DescriptorSetLayout.Bindings.push_back( { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT } );
		
		m_RendererData.SC_DescriptorSetLayout.Create();

		// Create the descriptor set.
		DescriptorSetSpecification Spec;
		Spec.Layout = m_RendererData.SC_DescriptorSetLayout;
		Spec.Pool = m_RendererData.SC_DescriptorPool;

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
		PipelineSpec.Width = Window::Get().Width();
		PipelineSpec.Height = Window::Get().Height();
		PipelineSpec.Name = "Scene Composite";
		PipelineSpec.pShader = m_RendererData.SceneCompositeShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { m_RendererData.SC_DescriptorSetLayout.VulkanLayout } };
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

				auto vp = m_RendererData.EditorCamera.ViewMatrix() * m_RendererData.EditorCamera.ProjectionMatrix();

				SkyboxMatricesObject.View = m_RendererData.EditorCamera.ViewMatrix();
				SkyboxMatricesObject.Projection = m_RendererData.EditorCamera.ProjectionMatrix();
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
		
		VkDescriptorPoolSize PoolSize = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = 1;
		
		std::vector< VkDescriptorPoolSize > PoolSizes;
		PoolSizes.push_back( PoolSize );

		m_RendererData.GridDescriptorPool = Ref< DescriptorPool >::Create( PoolSizes, 1 );
		
		DescriptorSetLayout LayoutDescriptorSet;
		
		VkDescriptorSetLayoutBinding BindingLayout = {};
		BindingLayout.binding = 0;
		BindingLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		BindingLayout.descriptorCount = 1;
		BindingLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		LayoutDescriptorSet.Bindings = { { BindingLayout } };
		LayoutDescriptorSet.Create();
		
		DescriptorSetSpecification Spec = {};
		Spec.Layout = LayoutDescriptorSet;
		Spec.Pool = m_RendererData.GridDescriptorPool;
		
		m_RendererData.GridDescriptorSet = Ref< DescriptorSet >::Create( Spec );

		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = m_RendererData.GridUniformBuffer;
		BufferInfo.offset = 0;
		BufferInfo.range = BufferSize;

		m_RendererData.GridDescriptorSet->Write( BufferInfo, {} );

		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( BaseVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( BaseVertex, Texcoord ) } );

		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );

		// Grid pipeline spec.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = Window::Get().Width();
		PipelineSpec.Height = Window::Get().Height();
		PipelineSpec.Name = "Grid";
		PipelineSpec.pShader = m_RendererData.GridShader.Pointer();
		PipelineSpec.Layout.SetLayouts = { { LayoutDescriptorSet.VulkanLayout } };
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
		// Destroy grid pipeline.
		m_RendererData.GridPipeline.Terminate();

		// Destroy uniform buffer.
		m_RendererData.GridUniformBuffer.Terminate();

		// Destroy grid index buffer.
		m_RendererData.GridIndexBuffer->Terminate();

		// Destroy grid vertex buffer.
		m_RendererData.GridVertexBuffer->Terminate();

		m_RendererData.SkyboxDescriptorSet->Terminate();

		m_RendererData.SkyboxDescriptorPool->Terminate();
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
		
		VkDescriptorPoolSize PoolSize = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 };
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = 1;
		
		std::vector< VkDescriptorPoolSize  > PoolSizes;
		PoolSizes.push_back( PoolSize );

		m_RendererData.SkyboxDescriptorPool = Ref< DescriptorPool >::Create( PoolSizes, 1 );
		
		DescriptorSetLayout LayoutDescriptorSet;

		VkDescriptorSetLayoutBinding BindingLayout = {};
		BindingLayout.binding = 0;
		BindingLayout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		BindingLayout.descriptorCount = 1;
		BindingLayout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		LayoutDescriptorSet.Bindings = { { BindingLayout } };
		LayoutDescriptorSet.Create();
		
		DescriptorSetSpecification Spec = {};
		Spec.Layout = LayoutDescriptorSet;
		Spec.Pool = m_RendererData.SkyboxDescriptorPool;

		m_RendererData.SkyboxDescriptorSet = Ref< DescriptorSet >::Create( Spec );
		
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = m_RendererData.SkyboxUniformBuffer;
		BufferInfo.offset = 0;
		BufferInfo.range = BufferSize;

		m_RendererData.SkyboxDescriptorSet->Write( BufferInfo, {} );
		
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
		PipelineSpec.Layout.SetLayouts = { { LayoutDescriptorSet.VulkanLayout } };
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
		// Destroy Skybox pipeline.
		m_RendererData.SkyboxPipeline.Terminate();

		// Destroy uniform buffer.
		m_RendererData.SkyboxUniformBuffer.Terminate();

		// Destroy Skybox index buffer.
		m_RendererData.SkyboxIndexBuffer->Terminate();

		// Destroy Skybox vertex buffer.
		m_RendererData.SkyboxVertexBuffer->Terminate();

		m_RendererData.SkyboxDescriptorSet->Terminate();

		m_RendererData.SkyboxDescriptorPool->Terminate();
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

	void SceneRenderer::GeometryPass()
	{
		m_RendererData.GeometryPassTimer.Reset();

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
			auto& rMaterial = Cmd.Mesh->GetMaterial();

			// Update uniform buffers.
			RendererData::StaticMeshMatrices u_Matrices = {};
			u_Matrices.ViewProjection = m_RendererData.EditorCamera.ViewProjection();
			u_Matrices.UseAlbedoTexture = rMaterial->Get<float>( "u_Matrices.UseAlbedoTexture" ) ? 1.0f : 0.5f;

			u_Matrices.UseNormalTexture = rMaterial->Get<float>( "u_Matrices.UseNormalTexture" ) ? 1.0f : 0.5f;
			u_Matrices.UseMetallicTexture = rMaterial->Get<float>( "u_Matrices.UseMetallicTexture" ) ? 1.0f : 0.5f;
			u_Matrices.UseRoughnessTexture = rMaterial->Get<float>( "u_Matrices.UseRoughnessTexture" ) ? 1.0f : 0.5f;

			u_Matrices.AlbedoColor = rMaterial->Get<glm::vec4>( "u_Matrices.AlbedoColor" );

			//for ( Submesh& rSubmesh : Cmd.Mesh->Submeshes() )
			//{
			//	u_Matrices.Transform = Cmd.Transform * rSubmesh.Transform;
			//}

			//m_RendererData.SM_MatricesUBO.UpdateData( &u_Matrices, sizeof( u_Matrices ) );

			//m_RendererData.StaticMeshDescriptorSets[ uuid ] = CreateSMDescriptorSet( Cmd.Mesh );

			//m_RendererData.StaticMeshDescriptorSets[ uuid ]->Bind( m_RendererData.CommandBuffer, m_RendererData.StaticMeshPipeline.GetPipelineLayout() );

			/*
			Renderer::Get().RenderStaticMesh(
				m_RendererData.CommandBuffer,
				m_RendererData.StaticMeshPipeline,
				uuid, Cmd.Mesh, Cmd.Transform,
				m_RendererData.SM_MatricesUBO );
			*/

			//

			for ( Submesh& rSubmesh : Cmd.Mesh->Submeshes() )
			{
				u_Matrices.Transform = Cmd.Transform * rSubmesh.Transform;

				m_RendererData.SM_MatricesUBO.UpdateData( &u_Matrices, sizeof( u_Matrices ) );
				
				m_RendererData.StaticMeshDescriptorSets[ uuid ] = CreateSMDescriptorSet( uuid, Cmd.Mesh );

				m_RendererData.StaticMeshDescriptorSets[ uuid ].Bind( rSubmesh, m_RendererData.CommandBuffer, m_RendererData.StaticMeshPipeline.GetPipelineLayout() );

				m_RendererData.StaticMeshPipeline.Bind( m_RendererData.CommandBuffer );

				Cmd.Mesh->GetVertexBuffer()->Bind( m_RendererData.CommandBuffer );
				Cmd.Mesh->GetIndexBuffer()->Bind( m_RendererData.CommandBuffer );

				Renderer::Get().RenderSubmesh( m_RendererData.CommandBuffer, 
					m_RendererData.StaticMeshPipeline, 
					Cmd.Mesh, rSubmesh, u_Matrices.Transform,
					m_RendererData.SM_MatricesUBO );
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
		VkExtent2D Extent = { Window::Get().Width(), Window::Get().Height() };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		// Begin scene composite pass.
		m_RendererData.SceneComposite.BeginPass( CommandBuffer, m_RendererData.SceneCompositeFramebuffer, Extent );

		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = ( float ) Window::Get().Height();
		Viewport.width = ( float ) Window::Get().Width();
		Viewport.height = -( float ) Window::Get().Height();
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

		// Cleanup descriptor sets from last frame.
		if ( m_RendererData.StaticMeshDescriptorSets.size() >= 1 )
		{
			for ( auto&& [uid, set] : m_RendererData.StaticMeshDescriptorSets )
			{
				DestroySMDescriptorSet( uid );
			}

			m_RendererData.StaticMeshDescriptorSets.clear();
		}

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

	void SceneRenderer::AddDescriptorSet( const DescriptorSet& rDescriptorSet )
	{
		// Check if the descriptor set already exists.
//		auto res = std::find( std::begin( m_RendererData.StaticMeshDescriptorSets ), std::end( m_RendererData.StaticMeshDescriptorSets ), rDescriptorSet );
	
		//if( res == std::end( m_RendererData.StaticMeshDescriptorSets ) )
		//{
			//m_RendererData.StaticMeshDescriptorSets.push_back( rDescriptorSet );
		//}
	}

	MeshDescriptorSet SceneRenderer::CreateSMDescriptorSet( UUID& rUUID, const Ref<Mesh>& rMesh )
	{
		Ref< Material > rMaterial = rMesh->GetMaterial();
		
		Ref< Texture2D > AlbedoTexture = rMaterial->GetResource( "u_AlbedoTexture" );

		DescriptorSetSpecification Spec;
		Spec.Layout = m_RendererData.SM_DescriptorSetLayout;
		Spec.Pool = m_RendererData.GeometryDescriptorPool;
		
		std::unordered_map< Submesh, Ref< DescriptorSet > > Sets;
		
		for ( auto& submesh : rMesh->Submeshes() )
		{
			Ref< DescriptorSet > Set = Ref< DescriptorSet >::Create( Spec );

			// As this is a Static mesh (SM), descriptor set, we can hard-code the values.

			VkDescriptorBufferInfo MatricesBufferInfo = {};
			MatricesBufferInfo.buffer = m_RendererData.SM_MatricesUBO;
			MatricesBufferInfo.offset = 0;
			MatricesBufferInfo.range = sizeof( RendererData::StaticMeshMatrices );

			VkDescriptorImageInfo ImageInfo = {};
			ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			ImageInfo.imageView = AlbedoTexture->GetImageView();
			ImageInfo.sampler = AlbedoTexture->GetSampler();

			std::vector< VkWriteDescriptorSet > DescriptorWrites;

			DescriptorWrites.push_back( {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = Set->GetVulkanSet(),
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.pImageInfo = nullptr,
				.pBufferInfo = &MatricesBufferInfo,
				.pTexelBufferView = nullptr } );

			DescriptorWrites.push_back( {
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = Set->GetVulkanSet(),
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = &ImageInfo,
				.pBufferInfo = nullptr,
				.pTexelBufferView = nullptr } );

			Set->Write( DescriptorWrites );

			Sets[ submesh ] = Set;
		}
		
		return { .Owner = rUUID, .Mesh = rMesh, .DescriptorSets = Sets };
	}

	void SceneRenderer::DestroySMDescriptorSet( UUID uuid )
	{
		m_RendererData.StaticMeshDescriptorSets[ uuid ].Terminate();
	}
	
	//////////////////////////////////////////////////////////////////////////
	// RendererData
	//////////////////////////////////////////////////////////////////////////

	void RendererData::Terminate()
	{
		VkDevice LogicalDevice = VulkanContext::Get().GetDevice();

		GeometryPass.Terminate();
		GeometryPassDepth.Terminate();
		GeometryPassColor.Terminate();

		vkDestroyFramebuffer( LogicalDevice, GeometryFramebuffer, nullptr );
		GeometryFramebuffer = nullptr;

		GeometryDescriptorPool->Terminate();

		SM_MatricesUBO.Terminate();

		StaticMeshPipeline.Terminate();

		GridPipeline.Terminate();

		GridDescriptorSet->Terminate();
		GridDescriptorPool->Terminate();

		GridUniformBuffer.Terminate();

		GridVertexBuffer->Terminate();
		GridIndexBuffer->Terminate();

		SkyboxPipeline.Terminate();
		SkyboxDescriptorSet->Terminate();
		SkyboxDescriptorPool->Terminate();

		SkyboxUniformBuffer.Terminate();

		SkyboxVertexBuffer->Terminate();
		SkyboxIndexBuffer->Terminate();

		SceneComposite.Terminate();
		SceneCompositeDepth.Terminate();
		SceneCompositeColor.Terminate();

		vkDestroyFramebuffer( LogicalDevice, SceneCompositeFramebuffer, nullptr );

		SceneCompositePipeline.Terminate();
		
		SC_DescriptorSet.Delete();
		SC_DescriptorPool.Delete();
		
		SC_VertexBuffer->Terminate();
		SC_IndexBuffer->Terminate();

		
		//GridShader.Delete();
		//SkyboxShader.Delete();
		//StaticMeshShader.Delete();
	}

}