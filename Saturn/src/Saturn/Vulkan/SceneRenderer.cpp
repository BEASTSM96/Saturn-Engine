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
#include "MaterialInstance.h"
#include "Saturn/ImGui/UITools.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <backends/imgui_impl_vulkan.h>

#define M_PI 3.14159265358979323846
#define SHADOW_MAP_SIZE 4096.0f 

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::Init()
	{
		// Create command pool.

		VkCommandPoolCreateInfo CommandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		CommandPoolInfo.queueFamilyIndex = VulkanContext::Get().GetQueueFamilyIndices().GraphicsFamily.value();
		CommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		VK_CHECK( vkCreateCommandPool( VulkanContext::Get().GetDevice(), &CommandPoolInfo, nullptr, &m_RendererData.CommandPool ) );
		
		///
		if( m_RendererData.Width == 0 && m_RendererData.Height == 0 )
		{
			m_RendererData.Width = Window::Get().Width();
			m_RendererData.Height = Window::Get().Height();
		}
		
		//////////////////////////////////////////////////////////////////////////
		// Geometry 
		//////////////////////////////////////////////////////////////////////////

		InitGeometryPass();

		// Create grid.
		CreateGridComponents();

		// Create skybox.
		CreateSkyboxComponents();

		InitSceneComposite();

		InitDirShadowMap();

		InitSelectedGeometryPass();

		//////////////////////////////////////////////////////////////////////////
	}

	void SceneRenderer::CreateAllFBSets()
	{
		for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			m_RendererData.ShadowCascades[i].Framebuffer->CreateDescriptorSets();
		}

		m_RendererData.GeometryFramebuffer->CreateDescriptorSets();
		m_RendererData.SceneCompositeFramebuffer->CreateDescriptorSets();
	}

	void SceneRenderer::Terminate()
	{
		// Destroy grid.
		DestroyGridComponents();

		DestroySkyboxComponents();

		m_pScene = nullptr;
		
		m_DrawList.clear();
		
		m_RendererData.Terminate();
	}

	void SceneRenderer::InitGeometryPass()
	{
		// Create render pass.
		if( m_RendererData.GeometryPass )
			m_RendererData.GeometryPass = nullptr;

		/*
		* PassSpecification PassSpec = {};
		* PassSpec.Attachments = { FramebufferTextureFormat::BGRA8, FramebufferTextureFormat::Depth };
		* PassSpec.Dependencies = { FramebufferTextureFormat::BGRA8, FramebufferTextureFormat::Depth };
		*/

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

		m_RendererData.GeometryPass = Ref< Pass >::Create( PassSpec );

		// Create geometry framebuffer.
		if( m_RendererData.GeometryFramebuffer )
			m_RendererData.GeometryFramebuffer->Recreate();
		else
		{
			FramebufferSpecification FBSpec = {};
			FBSpec.RenderPass = m_RendererData.GeometryPass;
			FBSpec.Width = m_RendererData.Width;
			FBSpec.Height = m_RendererData.Height;
			
			FBSpec.Attachments = { FramebufferTextureFormat::BGRA8, FramebufferTextureFormat::Depth };

			m_RendererData.GeometryFramebuffer = Ref< Framebuffer >::Create( FBSpec );
		}

		//////////////////////////////////////////////////////////////////////////
		// STATIC MESHES
		//////////////////////////////////////////////////////////////////////////

		// Create the static meshes pipeline.
		// Load the shader
		if( !m_RendererData.StaticMeshShader ) 
		{
			m_RendererData.StaticMeshShader = Ref< Shader >::Create( "shader_new", "assets/shaders/shader_new.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.StaticMeshShader );
		}			
		
		if( m_RendererData.StaticMeshPipeline )
			m_RendererData.StaticMeshPipeline = nullptr;

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Static Meshes";
		PipelineSpec.Shader = m_RendererData.StaticMeshShader.Pointer();
		PipelineSpec.SetLayouts = { { m_RendererData.StaticMeshShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Bitangent" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		PipelineSpec.CullMode = CullMode::Back;
		PipelineSpec.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		
		m_RendererData.StaticMeshPipeline = Ref< Pipeline >::Create( PipelineSpec );
	}

	void SceneRenderer::InitDirShadowMap()
	{	
		m_RendererData.ShadowCascades.resize( SHADOW_CASCADE_COUNT );

		if( m_RendererData.DirShadowMapPass )
			m_RendererData.DirShadowMapPass = nullptr;
		
		PassSpecification PassSpec = {};
		PassSpec.Name = "Dir Shadow Map";
		
		PassSpec.Attachments = {
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
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
			}
		};

		PassSpec.DepthAttachmentRef = { .attachment = 0, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		PassSpec.Dependencies = {
			{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			},
			{
				.srcSubpass = 0,
				.dstSubpass = VK_SUBPASS_EXTERNAL,
				.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			}
		};
		
		m_RendererData.DirShadowMapPass = Ref<Pass>::Create( PassSpec );
		
		for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			FramebufferSpecification FBSpec = {};
			FBSpec.RenderPass = m_RendererData.DirShadowMapPass;
			FBSpec.Width = SHADOW_MAP_SIZE;
			FBSpec.Height = SHADOW_MAP_SIZE;
			FBSpec.ArrayLevels = SHADOW_CASCADE_COUNT;

			FBSpec.Attachments = { FramebufferTextureFormat::Depth };

			m_RendererData.ShadowCascades[ i ].Framebuffer = Ref<Framebuffer>::Create( FBSpec );
		}

		if( !m_RendererData.DirShadowMapShader )
		{
			m_RendererData.DirShadowMapShader = Ref< Shader >::Create( "DirShadowMap", "assets/shaders/ShadowMap.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.DirShadowMapShader );
		}
	
		struct PC_ShadowMap
		{
			alignas( 4 ) uint32_t CascadeIndex;
		};

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = SHADOW_MAP_SIZE;
		PipelineSpec.Height = SHADOW_MAP_SIZE;
		PipelineSpec.Name = "DirShadowMap";
		PipelineSpec.Shader = m_RendererData.DirShadowMapShader.Pointer();
		PipelineSpec.SetLayouts = { { m_RendererData.DirShadowMapShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.DirShadowMapPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tanget" },
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		m_RendererData.DirShadowMapPipeline = Ref< Pipeline >::Create( PipelineSpec );
	}

	void SceneRenderer::InitSceneComposite()
	{
		if( m_RendererData.SceneComposite )
			m_RendererData.SceneComposite = nullptr;

		// Create the scene composite render pass.
		PassSpecification PassSpec = {};
		PassSpec.Name = "Texture pass";
		
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

		m_RendererData.SceneComposite = Ref< Pass >::Create( PassSpec );
		
		if( m_RendererData.SceneCompositeFramebuffer )
			m_RendererData.SceneCompositeFramebuffer->Recreate();
		else
		{
			FramebufferSpecification FBSpec = {};
			FBSpec.RenderPass = m_RendererData.SceneComposite;
			FBSpec.Width = m_RendererData.Width;
			FBSpec.Height = m_RendererData.Height;

			FBSpec.Attachments = { FramebufferTextureFormat::BGRA8, FramebufferTextureFormat::Depth };

			m_RendererData.SceneCompositeFramebuffer = Ref< Framebuffer >::Create( FBSpec );
		}

		/////////

		// Create fullscreen quad.
		Renderer::Get().CreateFullscreenQuad( &m_RendererData.SC_VertexBuffer, &m_RendererData.SC_IndexBuffer );
		
		if( !m_RendererData.SceneCompositeShader )
		{
			m_RendererData.SceneCompositeShader = Ref< Shader >::Create( "SceneComposite", "assets/shaders/SceneComposite.glsl" );
			
			ShaderLibrary::Get().Add( m_RendererData.SceneCompositeShader );
		}
		
		DescriptorSetSpecification Spec;
		Spec.Layout = m_RendererData.SceneCompositeShader->GetSetLayout();
		Spec.Pool = m_RendererData.SceneCompositeShader->GetDescriptorPool();

		if( !m_RendererData.SC_DescriptorSet )
			m_RendererData.SC_DescriptorSet = Ref< DescriptorSet >::Create( Spec );

		VkDescriptorImageInfo GeometryPassImageInfo = {};
		GeometryPassImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		GeometryPassImageInfo.imageView = m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 0 ].ImageView;
		GeometryPassImageInfo.sampler = m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 0 ].Sampler;

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
		
		if( m_RendererData.SceneCompositePipeline )
			m_RendererData.SceneCompositePipeline = nullptr;

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Scene Composite";
		PipelineSpec.Shader = m_RendererData.SceneCompositeShader.Pointer();
		PipelineSpec.SetLayouts = { { m_RendererData.SceneCompositeShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.SceneComposite;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		m_RendererData.SceneCompositePipeline = Ref<Pipeline>::Create( PipelineSpec );
	}

	void SceneRenderer::InitSelectedGeometryPass()
	{
		if( m_RendererData.SelectedGeometryPass )
			m_RendererData.SelectedGeometryPass = nullptr;

		// Create the selected geometry render pass.
		PassSpecification PassSpec = {};
		PassSpec.Name = "Selected Geometry";
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

		m_RendererData.SelectedGeometryPass = Ref< Pass >::Create( PassSpec );

		if( m_RendererData.SelectedGeometryFramebuffer )
			m_RendererData.SelectedGeometryFramebuffer->Recreate();
		else
		{
			FramebufferSpecification FBSpec = {};
			FBSpec.RenderPass = m_RendererData.SelectedGeometryPass;
			FBSpec.Width = m_RendererData.Width;
			FBSpec.Height = m_RendererData.Height;

			FBSpec.Attachments = { FramebufferTextureFormat::BGRA8, FramebufferTextureFormat::Depth };

			m_RendererData.SelectedGeometryFramebuffer = Ref< Framebuffer >::Create( FBSpec );
		}
		
		//////
		if( !m_RendererData.SelectedGeometryShader )
		{
			m_RendererData.SelectedGeometryShader = Ref< Shader >::Create( "Selected Geometry Shader", "assets/shaders/Outline.glsl" );

			ShaderLibrary::Get().Add( m_RendererData.SelectedGeometryShader );

			DescriptorSetSpecification DescriptorSetSpec = {};
			DescriptorSetSpec.Layout = m_RendererData.SelectedGeometryShader->GetSetLayout();
			DescriptorSetSpec.Pool = m_RendererData.SelectedGeometryShader->GetDescriptorPool();

			m_RendererData.SelectedGeometrySet = Ref<DescriptorSet>::Create( DescriptorSetSpec );
		}

		//////
		if( m_RendererData.SelectedGeometryPipeline )
			m_RendererData.SelectedGeometryPipeline = nullptr;

		std::vector< VkPushConstantRange > PushConstants;
		PushConstants.push_back( { .stageFlags = VK_SHADER_STAGE_ALL, .offset = 0, .size = sizeof( RendererData::StaticMeshMaterial ) } );

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Selected Geometry";
		PipelineSpec.Shader = m_RendererData.SelectedGeometryShader.Pointer();
		PipelineSpec.SetLayouts = { { m_RendererData.SelectedGeometryShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.SelectedGeometryPass;
		PipelineSpec.UseDepthTest = false;
		PipelineSpec.UseStencilTest = true;
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		m_RendererData.SelectedGeometryPipeline = Ref<Pipeline>::Create( PipelineSpec );
	}

	void SceneRenderer::RenderGrid()
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		auto& UBs = m_RendererData.GridShader->GetUniformBuffers();

		// Set UB Data.

		glm::mat4 trans = glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) * glm::scale( glm::mat4( 1.0f ), glm::vec3( 16.0f ) );

		RendererData::GridMatricesObject GridMatricesObject = {};
		GridMatricesObject.Transform = trans;
		GridMatricesObject.ViewProjection = m_RendererData.EditorCamera.ViewProjection();

		GridMatricesObject.Res = 0.025f;
		GridMatricesObject.Scale = 16.025f;

		auto bufferAloc = pAllocator->GetAllocationFromBuffer( UBs[ ShaderType::All ][ 0 ].Buffer );

		void* dstData = pAllocator->MapMemory< void >( bufferAloc );

		memcpy( dstData, &GridMatricesObject, sizeof( GridMatricesObject ) );

		pAllocator->UnmapMemory( bufferAloc );
		
		Renderer::Get().SubmitFullscreenQuad( 
			m_RendererData.CommandBuffer, m_RendererData.GridPipeline, m_RendererData.GridDescriptorSet, m_RendererData.GridIndexBuffer, m_RendererData.GridVertexBuffer );
	}

	void SceneRenderer::RenderSkybox()
	{
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		auto& UBs = m_RendererData.SkyboxShader->GetUniformBuffers();

		Entity SkylightEntity;

		if( !m_pScene )
			return;

		auto view = m_pScene->GetAllEntitiesWith< SkylightComponent >();

		for ( const auto e : view )
		{
			SkylightEntity = { e, m_pScene };
		}

		if( SkylightEntity )
		{
			auto& Skylight = SkylightEntity.GetComponent< SkylightComponent >();
			
			if( !Skylight.DynamicSky )
				return;

			RendererData::SkyboxMatricesObject SkyboxMatricesObject = {};

			SkyboxMatricesObject.View = m_RendererData.EditorCamera.ViewMatrix();
			SkyboxMatricesObject.Projection = m_RendererData.EditorCamera.ProjectionMatrix();
			SkyboxMatricesObject.Turbidity = Skylight.Turbidity;
			SkyboxMatricesObject.Azimuth = Skylight.Azimuth;
			SkyboxMatricesObject.Inclination = Skylight.Inclination;

			auto bufferAloc = pAllocator->GetAllocationFromBuffer( UBs[ ShaderType::All ][ 0 ].Buffer );

			void* Data = pAllocator->MapMemory< void >( bufferAloc );

			memcpy( Data, &SkyboxMatricesObject, sizeof( SkyboxMatricesObject ) );

			pAllocator->UnmapMemory( bufferAloc );

			Renderer::Get().SubmitFullscreenQuad( 
				CommandBuffer, m_RendererData.SkyboxPipeline, m_RendererData.SkyboxDescriptorSet, m_RendererData.SkyboxIndexBuffer, m_RendererData.SkyboxVertexBuffer );
		}
	}

	struct FrustumBounds
	{
		float r, l, b, t, f, n;
	};

	void SceneRenderer::UpdateCascades( glm::vec3 Direction )
	{
		FrustumBounds frustumBounds[ 3 ];

		auto viewProjection = m_RendererData.EditorCamera.ProjectionMatrix() * m_RendererData.EditorCamera.ViewMatrix();

		const int SHADOW_MAP_CASCADE_COUNT = 4;
		const float FAR_PLANE = 15.0f;
		const float NEAR_PLANE = -15.0f;
		
		float cascadeSplits[ SHADOW_MAP_CASCADE_COUNT ];

		// TODO: less hard-coding!
		float nearClip = 0.1f;
		float farClip = 1000.0f;
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for( uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++ )
		{
			float p = ( i + 1 ) / static_cast< float >( SHADOW_MAP_CASCADE_COUNT );
			float log = minZ * std::pow( ratio, p );
			float uniform = minZ + range * p;
			float d = m_RendererData.CascadeSplitLambda * ( log - uniform ) + uniform;
			cascadeSplits[ i ] = ( d - nearClip ) / clipRange;
		}

		cascadeSplits[ 3 ] = 0.3f;

		// Manually set cascades here
		// cascadeSplits[0] = 0.05f;
		// cascadeSplits[1] = 0.15f;
		// cascadeSplits[2] = 0.3f;
		// cascadeSplits[3] = 1.0f;

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for( uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++ )
		{
			float splitDist = cascadeSplits[ i ];

			glm::vec3 frustumCorners[ 8 ] =
			{
				glm::vec3( -1.0f,  1.0f, -1.0f ),
				glm::vec3( 1.0f,  1.0f, -1.0f ),
				glm::vec3( 1.0f, -1.0f, -1.0f ),
				glm::vec3( -1.0f, -1.0f, -1.0f ),
				glm::vec3( -1.0f,  1.0f,  1.0f ),
				glm::vec3( 1.0f,  1.0f,  1.0f ),
				glm::vec3( 1.0f, -1.0f,  1.0f ),
				glm::vec3( -1.0f, -1.0f,  1.0f ),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse( viewProjection );
			for( uint32_t i = 0; i < 8; i++ )
			{
				glm::vec4 invCorner = invCam * glm::vec4( frustumCorners[ i ], 1.0f );
				frustumCorners[ i ] = invCorner / invCorner.w;
			}

			for( uint32_t i = 0; i < 4; i++ )
			{
				glm::vec3 dist = frustumCorners[ i + 4 ] - frustumCorners[ i ];
				frustumCorners[ i + 4 ] = frustumCorners[ i ] + ( dist * splitDist );
				frustumCorners[ i ] = frustumCorners[ i ] + ( dist * lastSplitDist );
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3( 0.0f );
			for( uint32_t i = 0; i < 8; i++ )
				frustumCenter += frustumCorners[ i ];

			frustumCenter /= 8.0f;

			//frustumCenter *= 0.01f;

			float radius = 0.0f;
			for( uint32_t i = 0; i < 8; i++ )
			{
				float distance = glm::length( frustumCorners[ i ] - frustumCenter );
				radius = glm::max( radius, distance );
			}
			radius = std::ceil( radius * 16.0f ) / 16.0f;

			glm::vec3 maxExtents = glm::vec3( radius );
			glm::vec3 minExtents = -maxExtents;

			glm::vec3 lightDir = -Direction;
			glm::mat4 lightViewMatrix = glm::lookAt( frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3( 0.0f, 0.0f, 1.0f ) );
			glm::mat4 lightOrthoMatrix = glm::ortho( minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

			// Offset to texel space to avoid shimmering (from https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
			glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			const float ShadowMapResolution = 4096.0f;
			glm::vec4 shadowOrigin = ( shadowMatrix * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ) * ShadowMapResolution / 2.0f;
			glm::vec4 roundedOrigin = glm::round( shadowOrigin );
			glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
			roundOffset = roundOffset * 2.0f / ShadowMapResolution;
			roundOffset.z = 0.0f;
			roundOffset.w = 0.0f;

			lightOrthoMatrix[ 3 ] += roundOffset;

			// Store split distance and matrix in cascade
			m_RendererData.ShadowCascades[ i ].SplitDepth = ( nearClip + splitDist * clipRange ) * -1.0f;
			m_RendererData.ShadowCascades[ i ].ViewProjection = lightOrthoMatrix * lightViewMatrix;

			lastSplitDist = cascadeSplits[ i ];
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
		Renderer::Get().CreateFullscreenQuad( &m_RendererData.GridVertexBuffer, &m_RendererData.GridIndexBuffer );
		
		if( !m_RendererData.GridShader )
		{
			m_RendererData.GridShader = Ref< Shader >::Create( "Grid", "assets/shaders/Grid.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.GridShader );
		}
	
		DescriptorSetSpecification Spec = {};
		Spec.Layout = m_RendererData.GridShader->GetSetLayout();
		Spec.Pool = m_RendererData.GridShader->GetDescriptorPool();
		
		if( !m_RendererData.GridDescriptorSet )
			m_RendererData.GridDescriptorSet = Ref< DescriptorSet >::Create( Spec );

		auto& UBs = m_RendererData.GridShader->GetUniformBuffers();

		VkDescriptorBufferInfo DescriptorBufferInfo = {};
		DescriptorBufferInfo.buffer = UBs[ ShaderType::All ][ 0 ].Buffer;
		DescriptorBufferInfo.offset = 0;
		DescriptorBufferInfo.range = UBs[ ShaderType::All ][ 0 ].Size;

		m_RendererData.GridDescriptorSet->Write( DescriptorBufferInfo, {} );

		// Gird shader attribute descriptions.
		std::vector< VkVertexInputAttributeDescription > AttributeDescriptions;
		
		AttributeDescriptions.push_back( { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof( BaseVertex, Position ) } );
		AttributeDescriptions.push_back( { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( BaseVertex, Texcoord ) } );

		// Grid shader binding descriptions.
		std::vector< VkVertexInputBindingDescription > BindingDescriptions;
		BindingDescriptions.push_back( { 0, Layout.GetStride(), VK_VERTEX_INPUT_RATE_VERTEX } );
		
		if( m_RendererData.GridPipeline )
			m_RendererData.GridPipeline = nullptr;

		// Grid pipeline spec.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width =m_RendererData.Width;
		PipelineSpec.Height =m_RendererData.Height;
		PipelineSpec.Name = "Grid";
		PipelineSpec.Shader = m_RendererData.GridShader.Pointer();
		PipelineSpec.SetLayouts = { { m_RendererData.GridShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};
		
		m_RendererData.GridPipeline = Ref< Pipeline >::Create( PipelineSpec );
	}

	void SceneRenderer::DestroyGridComponents()
	{
		// Destroy grid pipeline.
		m_RendererData.GridPipeline = nullptr;

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

		// Create fullscreen quad.
		Renderer::Get().CreateFullscreenQuad( &m_RendererData.SkyboxVertexBuffer, &m_RendererData.SkyboxIndexBuffer );

		// Create skybox shader.
		
		if( !m_RendererData.SkyboxShader )
		{
			m_RendererData.SkyboxShader = Ref<Shader>::Create( "Skybox", "assets/shaders/Skybox.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.SkyboxShader );
		}
		
		auto& UBs = m_RendererData.SkyboxShader->GetUniformBuffers();

		// Create uniform buffer.		
		DescriptorSetSpecification Spec = {};
		Spec.Layout = m_RendererData.SkyboxShader->GetSetLayout();
		Spec.Pool = m_RendererData.SkyboxShader->GetDescriptorPool();
		
		if( !m_RendererData.SkyboxDescriptorSet )
			m_RendererData.SkyboxDescriptorSet = Ref< DescriptorSet >::Create( Spec );
		
		VkDescriptorBufferInfo DescriptorBufferInfo = {};
		DescriptorBufferInfo.buffer = UBs[ ShaderType::All ][ 0 ].Buffer;
		DescriptorBufferInfo.offset = 0;
		DescriptorBufferInfo.range = UBs[ ShaderType::All ][ 0 ].Size;

		m_RendererData.SkyboxDescriptorSet->Write( DescriptorBufferInfo, {} );
		
		if( m_RendererData.SkyboxPipeline )
			m_RendererData.SkyboxPipeline = nullptr;

		// Create pipeline.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Skybox";
		PipelineSpec.Shader = m_RendererData.SkyboxShader.Pointer();
		PipelineSpec.SetLayouts = { { m_RendererData.SkyboxShader->GetSetLayout() } };
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};

		m_RendererData.SkyboxPipeline = Ref<Pipeline>::Create( PipelineSpec );
	}

	void SceneRenderer::DestroySkyboxComponents()
	{
		// Destroy Skybox pipeline.
		m_RendererData.SkyboxPipeline = nullptr;

		// Destroy Skybox index buffer.
		m_RendererData.SkyboxIndexBuffer->Terminate();

		// Destroy Skybox vertex buffer.
		m_RendererData.SkyboxVertexBuffer->Terminate();

		m_RendererData.SkyboxDescriptorSet->Terminate();
	}

	void SceneRenderer::ImGuiRender()
	{
		ImGui::Begin( "Scene Renderer" );

		if( ImGui::CollapsingHeader( "Stats" ) )
		{
			auto FrameTimings = Renderer::Get().GetFrameTimings();

			ImGui::Text( "Renderer::BeginFrame: %.2f ms", FrameTimings.first );

			ImGui::Text( "SceneRenderer::GeometryPass: %.2f ms", m_RendererData.GeometryPassTimer.ElapsedMilliseconds() );

			ImGui::Text( "Renderer::EndFrame - Queue Present: %.2f ms", Renderer::Get().GetQueuePresentTime() );

			ImGui::Text( "Renderer::EndFrame: %.2f ms", FrameTimings.second );

			ImGui::Text( "Total : %.2f ms", Application::Get().Time().Milliseconds() );
		}

		if( ImGui::CollapsingHeader( "Shadow Settings" ) )
		{
			DrawVec3Control( "Light Pos", m_RendererData.LightPos );

			ImGui::Separator();

			for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
			{
				ImGui::Image( m_RendererData.ShadowCascades[i].Framebuffer->GetDepthAttachmentsResource().DescriptorSet, ImVec2( 100, 100 ) );
			}
		}

		ImGui::End();
	}

	void SceneRenderer::SubmitSelectedMesh( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		m_DrawList.push_back( { entity, mesh, transform } );
	}

	void SceneRenderer::SubmitMesh( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		m_DrawList.push_back( { entity, mesh, transform } );
	}

	void SceneRenderer::FlushDrawList()
	{
		m_DrawList.clear();
	}

	void SceneRenderer::Recreate()
	{
		InitGeometryPass();
		InitSelectedGeometryPass();
		CreateSkyboxComponents();
		CreateGridComponents();
	}

	void SceneRenderer::GeometryPass()
	{
		m_RendererData.GeometryPassTimer.Reset();

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		VkExtent2D Extent = { m_RendererData.Width, m_RendererData.Height };

		// Begin geometry pass.
		m_RendererData.GeometryPass->BeginPass( m_RendererData.CommandBuffer, m_RendererData.GeometryFramebuffer->GetVulkanFramebuffer(), Extent );
		
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
			auto& UBs = StaticMeshShader->GetUniformBuffers();

			// u_Matrices
			RendererData::StaticMeshMatrices u_Matrices = {};
			u_Matrices.ViewProjection = m_RendererData.EditorCamera.ViewProjection();
			u_Matrices.LightPos = m_RendererData.LightPos;

			auto bufferAloc = pAllocator->GetAllocationFromBuffer( UBs[ ShaderType::Vertex ][ 0 ].Buffer );

			void* pData = pAllocator->MapMemory< void >( bufferAloc );

			memcpy( pData, &u_Matrices, sizeof( u_Matrices ) );

			pAllocator->UnmapMemory( bufferAloc );

			Renderer::Get().SubmitMesh( m_RendererData.CommandBuffer,
				m_RendererData.StaticMeshPipeline, 
				Cmd.Mesh, Cmd.Transform );
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		//////////////////////////////////////////////////////////////////////////
		
		// End geometry pass.
		m_RendererData.GeometryPass->EndPass();

		m_RendererData.GeometryPassTimer.Stop();
	}

	void SceneRenderer::DirShadowMapPass()
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		VkExtent2D Extent = { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		std::array<VkClearValue, 2> ClearColors{};
		ClearColors[ 0 ].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo RenderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		RenderPassBeginInfo.renderPass = m_RendererData.DirShadowMapPass->GetVulkanPass();
		RenderPassBeginInfo.renderArea.extent = Extent;
		RenderPassBeginInfo.pClearValues = ClearColors.data();
		RenderPassBeginInfo.clearValueCount = ClearColors.size();
		
		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = 0;
		Viewport.width = SHADOW_MAP_SIZE;
		Viewport.height = SHADOW_MAP_SIZE;
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;
		
		VkRect2D Scissor = { .offset = { 0, 0 }, .extent = Extent };
		
		vkCmdSetViewport( CommandBuffer, 0, 1, &Viewport );
		vkCmdSetScissor( CommandBuffer, 0, 1, &Scissor );
		
		vkCmdSetDepthBias( CommandBuffer,
			1.25f,
			0.0f,
			1.75f );

		m_RendererData.DirShadowMapPipeline->Bind( CommandBuffer );

		//////////////////////////////////////////////////////////////////////////
		
		Ref< Shader > ShadowShader = m_RendererData.DirShadowMapShader;

		DescriptorSetSpecification DescriptorSetSpec;
		DescriptorSetSpec.Pool = ShadowShader->GetDescriptorPool();
		DescriptorSetSpec.Layout = ShadowShader->GetSetLayout();

		for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			for( auto& Cmd : m_DrawList )
			{
				for( Submesh& rSubmesh : Cmd.Mesh->Submeshes() )
				{
					if( m_RendererData.ShadowCascades[ i ].DescriptorSets[ rSubmesh ] )
						m_RendererData.ShadowCascades[ i ].DescriptorSets[ rSubmesh ]->Terminate();

					m_RendererData.ShadowCascades[ i ].DescriptorSets[ rSubmesh ] = nullptr;
				}
			}

			m_RendererData.ShadowCascades[ i ].DescriptorSets.clear();
		}
		
		UpdateCascades( m_pScene->m_DirectionalLight[ 0 ].Direction );

		for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			RenderPassBeginInfo.framebuffer = m_RendererData.ShadowCascades[ i ].Framebuffer->GetVulkanFramebuffer();

			// Begin directional shadow map pass.
			CmdBeginDebugLabel( CommandBuffer, "DirShadowMap" );
			vkCmdBeginRenderPass( CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			// u_Matrices
			struct UB_Matrices
			{
				glm::mat4 ViewProjection;
			} u_Matrices;

			struct PC_Transform
			{
				glm::mat4 Transform;
			} u_Transform;


			u_Matrices.ViewProjection = m_RendererData.ShadowCascades[ i ].ViewProjection;

			auto& UBs = ShadowShader->GetUniformBuffers();

			auto bufferAloc = pAllocator->GetAllocationFromBuffer( UBs[ ShaderType::Vertex ][ 0 ].Buffer );

			void* pData = pAllocator->MapMemory< void >( bufferAloc );

			memcpy( pData, &u_Matrices, sizeof( u_Matrices ) );

			pAllocator->UnmapMemory( bufferAloc );

			for( auto& Cmd : m_DrawList )
			{
				for( Submesh& rSubmesh : Cmd.Mesh->Submeshes() )
				{
					////
					m_RendererData.ShadowCascades[ i ].DescriptorSets[ rSubmesh ] = Ref<DescriptorSet>::Create( DescriptorSetSpec );
					////

					Cmd.Mesh->GetVertexBuffer()->Bind( CommandBuffer );
					Cmd.Mesh->GetIndexBuffer()->Bind( CommandBuffer );
					
					glm::mat4 Transform = Cmd.Transform * rSubmesh.Transform;

					u_Transform.Transform = Transform;

					vkCmdPushConstants( CommandBuffer, m_RendererData.DirShadowMapPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( glm::mat4 ), &u_Transform );

					ShadowShader->WriteAllUBs( m_RendererData.ShadowCascades[ i ].DescriptorSets[ rSubmesh ] );

					m_RendererData.ShadowCascades[ i ].DescriptorSets[ rSubmesh ]->Bind( CommandBuffer, m_RendererData.DirShadowMapPipeline->GetPipelineLayout() );

					vkCmdDrawIndexed( CommandBuffer, rSubmesh.IndexCount, 1, rSubmesh.BaseIndex, rSubmesh.BaseVertex, 0 );
				}
			}

			vkCmdEndRenderPass( CommandBuffer );
			CmdEndDebugLabel( CommandBuffer );
		}
	}

	void SceneRenderer::SceneCompositePass()
	{
		VkExtent2D Extent = { m_RendererData.Width,m_RendererData.Height };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		// Begin scene composite pass.
		m_RendererData.SceneComposite->BeginPass( CommandBuffer, m_RendererData.SceneCompositeFramebuffer->GetVulkanFramebuffer(), Extent );

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
		
		Renderer::Get().SubmitFullscreenQuad( 
			CommandBuffer, m_RendererData.SceneCompositePipeline, 
			m_RendererData.SC_DescriptorSet, 
			m_RendererData.SC_IndexBuffer, m_RendererData.SC_VertexBuffer );
		
		// End scene composite pass.
		m_RendererData.SceneComposite->EndPass();
	}

	void SceneRenderer::SelectedGeometryPass()
	{
		VkExtent2D Extent = { m_RendererData.Width,m_RendererData.Height };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		if( m_SelectedMeshDrawList.size() > 0 )
		{
			m_RendererData.SelectedGeometryPass->BeginPass( CommandBuffer, m_RendererData.SelectedGeometryFramebuffer->GetVulkanFramebuffer(), Extent );

			VkViewport Viewport = {};
			Viewport.x = 0;
			Viewport.y = 0;
			Viewport.width = ( float ) m_RendererData.Width;
			Viewport.height = ( float ) m_RendererData.Height;
			Viewport.minDepth = 0.0f;
			Viewport.maxDepth = 1.0f;

			VkRect2D Scissor = { .offset = { 0, 0 }, .extent = Extent };

			vkCmdSetViewport( CommandBuffer, 0, 1, &Viewport );
			vkCmdSetScissor( CommandBuffer, 0, 1, &Scissor );

			m_RendererData.SelectedGeometryPipeline->Bind( m_RendererData.CommandBuffer );

			for( auto& Cmd : m_SelectedMeshDrawList )
			{
				auto& uuid = Cmd.entity.GetComponent<IdComponent>().ID;
				auto& rMaterial = Cmd.Mesh->GetBaseMaterial();
				auto& UBs = m_RendererData.SelectedGeometryShader->GetUniformBuffers();

				struct UB_Matrices
				{
					glm::mat4 ViewProjection;
					glm::mat4 Transform;
				} Matrices;

				Matrices.ViewProjection = m_RendererData.EditorCamera.ViewProjection();
				Matrices.Transform = Cmd.Transform;

				auto bufferAloc = pAllocator->GetAllocationFromBuffer( UBs[ ShaderType::Vertex ][ 0 ].Buffer );
				void* dstData = pAllocator->MapMemory< void >( bufferAloc );
				memcpy( dstData, &Matrices, sizeof( Matrices ) );
				pAllocator->UnmapMemory( bufferAloc );

				m_RendererData.SelectedGeometryShader->WriteAllUBs( m_RendererData.SelectedGeometrySet );

				m_RendererData.SelectedGeometrySet->Bind( m_RendererData.CommandBuffer, m_RendererData.SelectedGeometryPipeline->GetPipelineLayout() );

				Cmd.Mesh->GetVertexBuffer()->Bind( m_RendererData.CommandBuffer );
				Cmd.Mesh->GetIndexBuffer()->Bind( m_RendererData.CommandBuffer );

				glm::mat4 Transform = Cmd.Transform;

				//Cmd.Mesh->GetIndexBuffer()->Draw( CommandBuffer );

				//Renderer::Get().RenderSubmesh( m_RendererData.CommandBuffer,
				//	m_RendererData.SelectedGeometryPipeline,
				//	Cmd.Mesh, rSubmesh, Transform );
			}

			m_RendererData.SelectedGeometryPass->EndPass();
		}
	}

	void SceneRenderer::RenderScene()
	{
		if( !m_pScene )
		{
			FlushDrawList();
			return;
		}

		// Cleanup descriptor sets from last frame.
		/*
		for( auto& rDrawCommand : m_DrawList )
		{
			if( rDrawCommand.Mesh->GetMaterial()->HasAnyValueChanged() )
			{
				rDrawCommand.Mesh->RefreshDescriptorSets();
			}
		}
		*/

		m_RendererData.CommandBuffer = Renderer::Get().ActiveCommandBuffer();
		
		// Passes

		// DirShadowMap
		//CmdBeginDebugLabel( m_RendererData.CommandBuffer, "DirShadowMap" );
		
		//DirShadowMapPass();
		
		//CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Selected Geometry" );

		//SelectedGeometryPass();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Geometry" );
		
		GeometryPass();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );
		
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Scene Composite - Texture pass" );

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
		
		// DescriptorSets
		GridDescriptorSet->Terminate();
		SkyboxDescriptorSet->Terminate();
		SC_DescriptorSet = nullptr;

		/*
		for ( auto& set : DirShadowMapDescriptorSets )
		{
			if( set )
				set = nullptr;
		}

		DirShadowMapDescriptorSets.clear();
		*/

		// Vertex and Index buffers
		GridVertexBuffer->Terminate();
		GridIndexBuffer->Terminate();
		SkyboxVertexBuffer->Terminate();
		SkyboxIndexBuffer->Terminate();
		SC_VertexBuffer->Terminate();
		SC_IndexBuffer->Terminate();

		// Framebuffers
		GeometryFramebuffer = nullptr;
		//DirShadowMapFramebuffer = nullptr;
		SceneCompositeFramebuffer = nullptr;

		// Render Passes
		DirShadowMapPass->Terminate();
		GeometryPass->Terminate();
		SceneComposite->Terminate();

		DirShadowMapPass = nullptr;
		GeometryPass = nullptr;
		SceneComposite = nullptr;

		// Pipelines
		SceneCompositePipeline = nullptr;
		DirShadowMapPipeline = nullptr;
		StaticMeshPipeline = nullptr;
		GridPipeline = nullptr;
		SkyboxPipeline = nullptr;

		// Shaders
		GridShader = nullptr;
		SkyboxShader = nullptr;
		StaticMeshShader = nullptr;
		SceneCompositeShader = nullptr;
		DirShadowMapShader = nullptr;
		
		// Command Pools
		vkDestroyCommandPool( LogicalDevice, CommandPool, nullptr );
	}

}