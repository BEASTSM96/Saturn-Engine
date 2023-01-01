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
#include "ComputePipeline.h"
#include "Saturn/ImGui/UITools.h"
#include "Saturn/Core/Memory/Buffer.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <backends/imgui_impl_vulkan.h>

#include <random>

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
		
		if( m_RendererData.Width == 0 && m_RendererData.Height == 0 )
		{
			m_RendererData.Width = Window::Get().Width();
			m_RendererData.Height = Window::Get().Height();
		}
		
		//////////////////////////////////////////////////////////////////////////
		// Geometry 
		//////////////////////////////////////////////////////////////////////////

		if( !Application::Get().GetSpecification().CreateSceneRenderer )
			return;
			
		m_RendererData.StorageBufferSet = Ref<StorageBufferSet>::Create(0,0);

		InitPreDepth();

		InitGeometryPass();

		// Create grid.
		CreateGridComponents();

		// Create skybox.
		CreateSkyboxComponents();

		InitDirShadowMap();

		InitBloom();

		InitSceneComposite();

		m_RendererData.SceneEnvironment = Ref<EnvironmentMap>::Create();

		m_RendererData.BRDFLUT_Texture = Ref<Texture2D>::Create( "assets/textures/BRDF_LUT.tga", AddressingMode::Repeat, false );

		m_RendererData.SSAOPassTimer.Reset();
		m_RendererData.SSAOPassTimer.Stop();

		m_RendererData.AOCompositeTimer.Reset();
		m_RendererData.AOCompositeTimer.Stop();

		//////////////////////////////////////////////////////////////////////////
	}

	Ref<Image2D> SceneRenderer::CompositeImage()
	{
		return m_RendererData.ViewShadowCascades ? m_RendererData.ShadowCascades[0].Framebuffer->GetDepthAttachmentsResource() : m_RendererData.SceneCompositeFramebuffer->GetColorAttachmentsResources()[0];
	}

	void SceneRenderer::Terminate()
	{
		m_pScene = nullptr;
		
		m_DrawList.clear();
		m_ShadowMapDrawList.clear();
		
		m_RendererData.Terminate();
	}

	void SceneRenderer::InitGeometryPass()
	{
		// Create render pass.
		if( m_RendererData.GeometryPass )
			m_RendererData.GeometryPass->Recreate();
		else
		{
			PassSpecification PassSpec = {};
			PassSpec.Name = "Geometry Pass";
			PassSpec.Attachments = { ImageFormat::RGBA32F, ImageFormat::RGBA16F, ImageFormat::RGBA16F, ImageFormat::DEPTH24STENCIL8 };
			PassSpec.LoadDepth = true;

			m_RendererData.GeometryPass = Ref< Pass >::Create( PassSpec );
		}

		// Create geometry framebuffer.
		if( m_RendererData.GeometryFramebuffer )
			m_RendererData.GeometryFramebuffer = nullptr;

		FramebufferSpecification FBSpec = {};
		FBSpec.RenderPass = m_RendererData.GeometryPass;
		FBSpec.Width = m_RendererData.Width;
		FBSpec.Height = m_RendererData.Height;
		FBSpec.ExistingImage = m_RendererData.PreDepthFramebuffer->GetDepthAttachmentsResource();
		FBSpec.ExistingImageIndex = 3;
		// Depth will be the PreDepth image.
		FBSpec.Attachments = { ImageFormat::RGBA32F, ImageFormat::RGBA16F, ImageFormat::RGBA16F };

		m_RendererData.GeometryFramebuffer = Ref< Framebuffer >::Create( FBSpec );
		

		//////////////////////////////////////////////////////////////////////////
		// STATIC MESHES
		//////////////////////////////////////////////////////////////////////////

		// Create the static meshes pipeline.
		// Load the shader
		if( !m_RendererData.StaticMeshShader ) 
		{
			ShaderLibrary::Get().Load( "assets/shaders/shader_new.glsl" );
			m_RendererData.StaticMeshShader = ShaderLibrary::Get().Find( "shader_new" );
		}
		
		if( m_RendererData.StaticMeshPipeline )
			m_RendererData.StaticMeshPipeline = nullptr;

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Static Meshes";
		PipelineSpec.Shader = m_RendererData.StaticMeshShader.Pointer();
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tangent" },
			{ ShaderDataType::Float3, "a_Binormal" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		PipelineSpec.CullMode = CullMode::Back;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		
		m_RendererData.StaticMeshPipeline = Ref< Pipeline >::Create( PipelineSpec ); 
	}

	void SceneRenderer::InitDirShadowMap()
	{	
		m_RendererData.ShadowCascades.resize( SHADOW_CASCADE_COUNT );
		m_RendererData.DirShadowMapPasses.resize( SHADOW_CASCADE_COUNT );
		m_RendererData.DirShadowMapPipelines.resize( SHADOW_CASCADE_COUNT );

		if( !m_RendererData.DirShadowMapShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/ShadowMap.glsl" );
			m_RendererData.DirShadowMapShader = ShaderLibrary::Get().Find( "ShadowMap" );
		}

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = SHADOW_MAP_SIZE;
		PipelineSpec.Height = SHADOW_MAP_SIZE;
		PipelineSpec.Name = "DirShadowMap";
		PipelineSpec.Shader = m_RendererData.DirShadowMapShader.Pointer();
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tanget" },
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		PipelineSpec.CullMode = CullMode::Back;
		PipelineSpec.HasColorAttachment = false;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.RequestDescriptorSets = { ShaderType::Vertex, 0 };

		// Layered image
		Ref<Image2D> shadowImage = Ref<Image2D>::Create( ImageFormat::DEPTH32F, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 4 );
		shadowImage->SetDebugName( "Layered shadow image" );

		FramebufferSpecification FBSpec = {};
		FBSpec.Width = SHADOW_MAP_SIZE;
		FBSpec.Height = SHADOW_MAP_SIZE;
		FBSpec.ArrayLevels = SHADOW_CASCADE_COUNT;
		FBSpec.Attachments = { ImageFormat::Depth };
		FBSpec.ExistingImage = shadowImage;

		PassSpecification PassSpec = {};
		PassSpec.Name = "Dir Shadow Map";
		PassSpec.Attachments = { ImageFormat::Depth };

		for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{			
			m_RendererData.DirShadowMapPasses[ i ] = Ref<Pass>::Create( PassSpec );

			FBSpec.RenderPass = m_RendererData.DirShadowMapPasses[ i ];
			FBSpec.ExistingImageLayer = i;

			PipelineSpec.RenderPass = m_RendererData.DirShadowMapPasses[i];

			m_RendererData.ShadowCascades[ i ].Framebuffer = Ref<Framebuffer>::Create( FBSpec );

			m_RendererData.DirShadowMapPipelines[i] = Ref< Pipeline >::Create( PipelineSpec );
		}

	}

	void SceneRenderer::InitPreDepth()
	{
		if( m_RendererData.PreDepthPass )
			m_RendererData.PreDepthPass->Recreate();
		else
		{
			PassSpecification PassSpec = {};
			PassSpec.Name = "PreDepth";
			PassSpec.Attachments = { ImageFormat::DEPTH24STENCIL8 };

			m_RendererData.PreDepthPass = Ref<Pass>::Create( PassSpec );
		}

		if( m_RendererData.PreDepthFramebuffer )
			m_RendererData.PreDepthFramebuffer->Recreate( m_RendererData.Width, m_RendererData.Height );
		else
		{
			FramebufferSpecification FBSpec = {};
			FBSpec.Width = m_RendererData.Width;
			FBSpec.Height = m_RendererData.Height;
			FBSpec.RenderPass = m_RendererData.PreDepthPass;
			FBSpec.Attachments = { ImageFormat::DEPTH24STENCIL8 };

			m_RendererData.PreDepthFramebuffer = Ref<Framebuffer>::Create( FBSpec );
		}

		if( !m_RendererData.PreDepthShader ) 
		{
			ShaderLibrary::Get().Load( "assets/shaders/PreDepth.glsl" );
			ShaderLibrary::Get().Load( "assets/shaders/LightCulling.glsl" );

			m_RendererData.PreDepthShader = ShaderLibrary::Get().Find( "PreDepth" );
			m_RendererData.LightCullingShader = ShaderLibrary::Get().Find( "LightCulling" );
		}

		if( m_RendererData.PreDepthPipeline )
			m_RendererData.PreDepthPipeline = nullptr;

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "PreDepth";
		PipelineSpec.Shader = m_RendererData.PreDepthShader.Pointer();
		PipelineSpec.RenderPass = m_RendererData.PreDepthPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.CullMode = CullMode::Back;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.RequestDescriptorSets = { ShaderType::Vertex, 0 };
		PipelineSpec.DepthCompareOp = VK_COMPARE_OP_LESS;
		PipelineSpec.HasColorAttachment = false;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float3, "a_Tanget" },
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};

		m_RendererData.PreDepthPipeline = Ref<Pipeline>::Create( PipelineSpec );

		// Light culling
		m_RendererData.LightCullingPipeline = Ref<ComputePipeline>::Create( m_RendererData.LightCullingShader );

		if( !m_RendererData.LightCullingDescriptorSet )
			m_RendererData.LightCullingDescriptorSet = m_RendererData.LightCullingShader->CreateDescriptorSet( 0 );

		m_RendererData.LightCullingShader->WriteDescriptor( "u_PreDepth", m_RendererData.PreDepthFramebuffer->GetDepthAttachmentsResource()->GetDescriptorInfo(), m_RendererData.LightCullingDescriptorSet->GetVulkanSet() );

		m_RendererData.StorageBufferSet->Create( 0, 14 );
	}

	void SceneRenderer::InitSceneComposite()
	{
		if( m_RendererData.SceneComposite )
			m_RendererData.SceneComposite->Recreate();
		else
		{
			// Create the scene composite render pass.
			PassSpecification PassSpec = {};
			PassSpec.Name = "Texture pass";

			PassSpec.Attachments = { ImageFormat::BGRA8, ImageFormat::Depth };

			m_RendererData.SceneComposite = Ref< Pass >::Create( PassSpec );
		}

		if( m_RendererData.SceneCompositeFramebuffer )
			m_RendererData.SceneCompositeFramebuffer->Recreate( m_RendererData.Width, m_RendererData.Height );
		else
		{
			FramebufferSpecification FBSpec = {};
			FBSpec.RenderPass = m_RendererData.SceneComposite;
			FBSpec.Width = m_RendererData.Width;
			FBSpec.Height = m_RendererData.Height;

			FBSpec.Attachments = { ImageFormat::BGRA8, ImageFormat::Depth };

			m_RendererData.SceneCompositeFramebuffer = Ref< Framebuffer >::Create( FBSpec );
		}

		/////////

		// Create fullscreen quad.
		if( m_RendererData.SC_VertexBuffer == nullptr && m_RendererData.SC_IndexBuffer == nullptr )
			Renderer::Get().CreateFullscreenQuad( &m_RendererData.SC_VertexBuffer, &m_RendererData.SC_IndexBuffer );
		
		if( !m_RendererData.SceneCompositeShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/SceneComposite.glsl" );
			m_RendererData.SceneCompositeShader = ShaderLibrary::Get().Find( "SceneComposite" );
		}
		
		if( !m_RendererData.SC_DescriptorSet )
			m_RendererData.SC_DescriptorSet = m_RendererData.SceneCompositeShader->CreateDescriptorSet( 0 );

		m_RendererData.SceneCompositeShader->WriteDescriptor( "u_GeometryPassTexture", m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[0]->GetDescriptorInfo(), m_RendererData.SC_DescriptorSet->GetVulkanSet() );
		m_RendererData.SceneCompositeShader->WriteDescriptor( "u_BloomTexture", m_RendererData.BloomTextures[ 2 ]->GetDescriptorInfo(), m_RendererData.SC_DescriptorSet->GetVulkanSet() );

		m_RendererData.SceneCompositeShader->WriteAllUBs( m_RendererData.SC_DescriptorSet );

		if( m_RendererData.SceneCompositePipeline )
			m_RendererData.SceneCompositePipeline = nullptr;

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Scene Composite";
		PipelineSpec.Shader = m_RendererData.SceneCompositeShader.Pointer();
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

	void SceneRenderer::InitAOComposite()
	{
		PassSpecification PassSpec = {};
		PassSpec.Name = "AO-Composite";
		PassSpec.Attachments = { ImageFormat::RGBA32F };
		PassSpec.LoadColor = true;

		if( m_RendererData.AOComposite )
			m_RendererData.AOComposite->Recreate();
		else
			m_RendererData.AOComposite = Ref<Pass>::Create( PassSpec );

		FramebufferSpecification FBSpec = {};
		FBSpec.RenderPass = m_RendererData.AOComposite;
		FBSpec.Width = m_RendererData.Width;
		FBSpec.Height = m_RendererData.Height;
		FBSpec.CreateDepth = false;
		FBSpec.ExistingImage = m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 0 ];
		//FBSpec.Attachments = { ImageFormat::RGBA32F };

		//if( m_RendererData.AOCompositeFramebuffer )
		//	m_RendererData.AOCompositeFramebuffer->Recreate( m_RendererData.Width, m_RendererData.Height );
		//else
			m_RendererData.AOCompositeFramebuffer = Ref<Framebuffer>::Create( FBSpec );

		if( !m_RendererData.AOCompositeShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/AO-Composite.glsl" );
			m_RendererData.AOCompositeShader = ShaderLibrary::Get().Find( "AO-Composite" );
		}

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "AO-Composite";
		PipelineSpec.Shader = m_RendererData.AOCompositeShader;
		PipelineSpec.RenderPass = m_RendererData.AOComposite;
		PipelineSpec.UseDepthTest = false;
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		if( m_RendererData.AOCompositePipeline )
			m_RendererData.AOCompositePipeline = nullptr;

		m_RendererData.AOCompositePipeline = Ref<Pipeline>::Create( PipelineSpec );

		if( !m_RendererData.AO_DescriptorSet )
			m_RendererData.AO_DescriptorSet = m_RendererData.AOCompositeShader->CreateDescriptorSet( 0 );

		m_RendererData.AOCompositeShader->WriteDescriptor( "u_AlbedoTexture", m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 2 ]->GetDescriptorInfo(), m_RendererData.AO_DescriptorSet->GetVulkanSet() );

		m_RendererData.AOCompositeShader->WriteDescriptor( "u_TestTexture", m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 0 ]->GetDescriptorInfo(), m_RendererData.AO_DescriptorSet->GetVulkanSet() );

		m_RendererData.AOCompositeShader->WriteAllUBs( m_RendererData.AO_DescriptorSet );
	}

	void SceneRenderer::InitBloom()
	{
		if( !m_RendererData.BloomShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/Bloom.glsl" );
			m_RendererData.BloomShader = ShaderLibrary::Get().Find( "Bloom" );
		}

		m_RendererData.BloomComputePipeline = Ref<ComputePipeline>::Create( m_RendererData.BloomShader );

		for( size_t i = 0; i < 3; i++ ) 
		{
			m_RendererData.BloomTextures[ i ] = Ref<Texture2D>::Create( ImageFormat::RGBA32F, 1, 1, nullptr, true );
			m_RendererData.BloomTextures[ i ]->SetDebugName( "Bloom Texture: " + std::to_string( i ) );
		}

		m_RendererData.BloomDirtTexture = Renderer::Get().GetPinkTexture();

		m_RendererData.BloomDS = m_RendererData.BloomShader->CreateDescriptorSet( 0 );

		m_RendererData.BloomShader->WriteAllUBs( m_RendererData.BloomDS );

		m_RendererData.BloomDirtTexture = Ref<Texture2D>::Create( "assets/textures/BloomDirtTextureUE.png", AddressingMode::Repeat );

		VkDescriptorPoolSize PoolSizes[] = 
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		};

		VkDescriptorPoolCreateInfo PoolCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		PoolCreateInfo.maxSets = 10000;
		PoolCreateInfo.poolSizeCount = IM_ARRAYSIZE(PoolSizes);
		PoolCreateInfo.pPoolSizes = PoolSizes;
		PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VK_CHECK( vkCreateDescriptorPool( VulkanContext::Get().GetDevice(), &PoolCreateInfo, nullptr, &m_RendererData.BloomDescriptorPool ) );
	}

	void SceneRenderer::RenderGrid()
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// Set UB Data.

		glm::mat4 trans = glm::rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) ) * glm::scale( glm::mat4( 1.0f ), glm::vec3( 16.0f ) );

		RendererData::GridMatricesObject GridMatricesObject = {};
		GridMatricesObject.Transform = trans;
		GridMatricesObject.ViewProjection = m_RendererData.EditorCamera.ViewProjection();

		GridMatricesObject.Res = 0.025f;
		GridMatricesObject.Scale = 16.025f;

		auto Data = m_RendererData.GridShader->MapUB( ShaderType::All, 0, 0 );

		memcpy( Data, &GridMatricesObject, sizeof( GridMatricesObject ) );
		
		m_RendererData.GridShader->UnmapUB( ShaderType::All, 0, 0 );

		m_RendererData.GridShader->WriteAllUBs( m_RendererData.GridDescriptorSet );
		
		Renderer::Get().SubmitFullscreenQuad( 
			m_RendererData.CommandBuffer, m_RendererData.GridPipeline, m_RendererData.GridDescriptorSet, m_RendererData.GridIndexBuffer, m_RendererData.GridVertexBuffer );
	}

	void SceneRenderer::RenderSkybox()
	{
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		auto& UBs = m_RendererData.SkyboxDescriptorSet;

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
			SkyboxMatricesObject.InverseVP = glm::inverse( m_RendererData.EditorCamera.ViewProjection() );

			m_RendererData.SkyboxShader->UploadUB( ShaderType::Vertex, 0, 0, &SkyboxMatricesObject, sizeof( SkyboxMatricesObject ) );

			m_RendererData.SkyboxShader->WriteDescriptor( "u_CubeTexture", m_RendererData.SceneEnvironment->IrradianceMap->GetDescriptorInfo(), m_RendererData.SkyboxDescriptorSet->GetVulkanSet() );

			struct ub_Data
			{
				float SkyboxLod;
			} u_Data;

			u_Data = {};
			u_Data.SkyboxLod = m_RendererData.SkyboxLod;

			m_RendererData.SkyboxShader->UploadUB( ShaderType::Fragment, 0, 2, &u_Data, sizeof( u_Data ) );

			m_RendererData.SkyboxShader->WriteAllUBs( m_RendererData.SkyboxDescriptorSet );

			Renderer::Get().SubmitFullscreenQuad( 
				CommandBuffer, m_RendererData.SkyboxPipeline, m_RendererData.SkyboxDescriptorSet, m_RendererData.SkyboxIndexBuffer, m_RendererData.SkyboxVertexBuffer );
		}
	}

	void SceneRenderer::PrepareSkybox()
	{
		if( !m_pScene )
			return;

		Entity SkylightEntity;

		auto view = m_pScene->GetAllEntitiesWith< SkylightComponent >();

		for( const auto e : view )
		{
			SkylightEntity = { e, m_pScene };
		}

		if( SkylightEntity )
		{
			auto& Skylight = SkylightEntity.GetComponent< SkylightComponent >();

			if( !Skylight.DynamicSky )
				return;

			if( Skylight.DynamicSky && !m_RendererData.SceneEnvironment->IrradianceMap && !m_RendererData.SceneEnvironment->IrradianceMap )
			{
				m_RendererData.SceneEnvironment->Turbidity = Skylight.Turbidity;
				m_RendererData.SceneEnvironment->Azimuth = Skylight.Azimuth;
				m_RendererData.SceneEnvironment->Inclination = Skylight.Inclination;

				Ref<TextureCube> map = CreateDymanicSky();

				m_RendererData.SceneEnvironment->IrradianceMap = map;
				m_RendererData.SceneEnvironment->RadianceMap = map;
			}

			m_RendererData.SkyboxShader->WriteAllUBs( m_RendererData.SkyboxDescriptorSet );
		}
	}

	void SceneRenderer::UpdateCascades( const glm::vec3& Direction )
	{
		const auto& viewProjection = m_RendererData.EditorCamera.ProjectionMatrix() * m_RendererData.EditorCamera.ViewMatrix();

		const int SHADOW_MAP_CASCADE_COUNT = 4;
		float cascadeSplits[ SHADOW_MAP_CASCADE_COUNT ];

		float nearClip = 0.1F;
		float farClip = 1000.0F;
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
			glm::mat4 lightOrthoMatrix = glm::ortho( minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + -15.0f, maxExtents.z - minExtents.z + 15.0f );

			// Offset to texel space to avoid shimmering (from https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
			glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			float ShadowMapResolution = SHADOW_MAP_SIZE;
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

		// Create fullscreen quad.
		Renderer::Get().CreateFullscreenQuad( &m_RendererData.GridVertexBuffer, &m_RendererData.GridIndexBuffer );
		
		if( !m_RendererData.GridShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/Grid.glsl" );
			m_RendererData.GridShader = ShaderLibrary::Get().Find( "Grid" );
		}
	
		if( !m_RendererData.GridDescriptorSet ) 
			m_RendererData.GridDescriptorSet = m_RendererData.GridShader->CreateDescriptorSet( 0 );

		m_RendererData.GridShader->WriteAllUBs( m_RendererData.GridDescriptorSet );

		if( m_RendererData.GridPipeline )
			m_RendererData.GridPipeline = nullptr;

		// Grid pipeline spec.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Grid";
		PipelineSpec.Shader = m_RendererData.GridShader.Pointer();
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
	
	//////////////////////////////////////////////////////////////////////////
	// Skybox
	//////////////////////////////////////////////////////////////////////////

	void SceneRenderer::CreateSkyboxComponents()
	{
		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		// Create fullscreen quad.
		if( m_RendererData.SkyboxVertexBuffer == nullptr && m_RendererData.SkyboxIndexBuffer == nullptr )
			Renderer::Get().CreateFullscreenQuad( &m_RendererData.SkyboxVertexBuffer, &m_RendererData.SkyboxIndexBuffer );

		// Create skybox shader.
		
		if( !m_RendererData.SkyboxShader && !m_RendererData.PreethamShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/Skybox.glsl" );
			ShaderLibrary::Get().Load( "assets/shaders/Skybox_Compute.glsl" );

			m_RendererData.SkyboxShader = ShaderLibrary::Get().Find( "Skybox" );
			m_RendererData.PreethamShader = ShaderLibrary::Get().Find( "Skybox_Compute" );
		}
				
		if( !m_RendererData.SkyboxDescriptorSet ) 
			m_RendererData.SkyboxDescriptorSet = m_RendererData.SkyboxShader->CreateDescriptorSet( 0 );

		if( !m_RendererData.PreethamDescriptorSet )
			m_RendererData.PreethamDescriptorSet = m_RendererData.PreethamShader->CreateDescriptorSet( 0 );

		m_RendererData.SkyboxShader->WriteAllUBs( m_RendererData.SkyboxDescriptorSet );
		m_RendererData.PreethamShader->WriteAllUBs( m_RendererData.PreethamDescriptorSet );

		if( m_RendererData.SkyboxPipeline )
			m_RendererData.SkyboxPipeline = nullptr;

		// Create pipeline.
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "Skybox";
		PipelineSpec.Shader = m_RendererData.SkyboxShader.Pointer();
		PipelineSpec.RenderPass = m_RendererData.GeometryPass;
		PipelineSpec.UseDepthTest = true;
		PipelineSpec.CullMode = CullMode::Back;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};

		m_RendererData.SkyboxPipeline = Ref<Pipeline>::Create( PipelineSpec );
	}

	void SceneRenderer::ImGuiRender()
	{
		ImGui::Begin( "Scene Renderer" );
		
		ImGui::Text( "Viewport size, %i, %i", ( int )m_RendererData.Width, ( int ) m_RendererData.Height );

		ImGui::Text( "FPS: %.1f", ImGui::GetIO().Framerate );

		if( TreeNode( "Stats", true ) )
		{
			auto FrameTimings = Renderer::Get().GetFrameTimings();

			float shadowPassTime = 0;

			if( m_RendererData.EnableShadows )
			{
				for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
				{
					shadowPassTime += m_RendererData.ShadowMapTimers[ i ].ElapsedMilliseconds();
				}
			}

			ImGui::Text( "Renderer::BeginFrame: %.2f ms", FrameTimings.first );

			ImGui::Text( "SceneRenderer::PreDepthPass: %.2f ms", m_RendererData.PreDepthTimer.ElapsedMilliseconds() );

			ImGui::Text( "SceneRenderer::ShadowMapPass: %.2f ms", shadowPassTime );

			ImGui::Text( "SceneRenderer::LightCulling: %.4f ms", m_RendererData.LightCullingTimer.ElapsedMilliseconds() );

			ImGui::Text( "SceneRenderer::GeometryPass: %.2f ms", m_RendererData.GeometryPassTimer.ElapsedMilliseconds() );

			ImGui::Text( "SceneRenderer::BlomPass: %.3f ms", m_RendererData.BloomTimer.ElapsedMilliseconds() );

			ImGui::Text( "SceneRenderer::SSAOPass: %.2f ms", m_RendererData.SSAOPassTimer.ElapsedMilliseconds() );

			ImGui::Text( "SceneRenderer::AOComposite: %.2f ms", m_RendererData.AOCompositeTimer.ElapsedMilliseconds() );

			ImGui::Text( "Renderer::EndFrame - Queue Present: %.2f ms", Renderer::Get().GetQueuePresentTime() );

			ImGui::Text( "Renderer::EndFrame: %.2f ms", FrameTimings.second );

			ImGui::Text( "Total : %.2f ms", Application::Get().Time().Milliseconds() );
			
			EndTreeNode();
		}

		if( TreeNode( "Environment", false ) )
		{
			ImGui::DragFloat( "Skybox Lod", &m_RendererData.SkyboxLod, 0.1f, 0.0f, 1000.0f );

			EndTreeNode();
		}

		if( TreeNode( "Scene renderer data", true ) )
		{
			if( TreeNode( "Shadow settings", true ) )
			{
				ImGui::DragFloat( "Cascade Split Lambda", &m_RendererData.CascadeSplitLambda, 1.0f, 0.01f, 1.0f );
				ImGui::DragFloat( "Cascade Near plane", &m_RendererData.CascadeNearPlaneOffset, 1.0f, -1000.0f, 1000.0f );
				ImGui::DragFloat( "Cascade Far plane", &m_RendererData.CascadeFarPlaneOffset, 1.0f, -1000.0f, 1000.0f );	

				ImGui::Checkbox( "Enable shadows", &m_RendererData.EnableShadows );

				static int index = 0;
				auto framebuffer = m_RendererData.ShadowCascades[ index ].Framebuffer->GetDepthAttachmentsResource();

				float size = ImGui::GetContentRegionAvail().x;

				ImGui::SliderInt( "##cascade_dt", &index, 0, 3 );

				Image( framebuffer, (uint32_t)index, { size, size }, { 0, 1 }, { 1, 0 } );

				EndTreeNode();
			}

			if( TreeNode( "Bloom settings", true ) )
			{
				static int index = 0;
				static int MipIndex = 0;
				auto& img = m_RendererData.BloomTextures[ index ];

				ImGui::SliderInt( "##bloom_tex", &index, 0, 2 );
				ImGui::SliderInt( "##mip", &MipIndex, 0, img->GetMipMapLevels() );

				float size = ImGui::GetContentRegionAvail().x;

				Image( img, MipIndex, { size, size }, { 0, 1 }, { 1, 0 } );

				EndTreeNode();
			}

			/*
			if( TreeNode( "SSAO", true ) )
			{
				auto framebuffer = m_RendererData.SSAOFramebuffer->GetColorAttachmentsResources()[ 0 ];

				float size = ImGui::GetContentRegionAvail().x;

				Image( framebuffer, { size, size }, { 0, 1 }, { 1, 0 } );

				EndTreeNode();
			}
			*/

			if( TreeNode( "Forward+", true ) )
			{
				ImGui::Checkbox( "Show Light Complexity", &m_RendererData.ShowLightComplexity );
				ImGui::Checkbox( "Show Light Culling", &m_RendererData.ShowLightCulling );

				EndTreeNode();
			}
			
			EndTreeNode();
		}

		ImGui::End();
	}

	void SceneRenderer::SubmitSelectedMesh( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		auto& submeshes = mesh->Submeshes();
		for( size_t i = 0; i < submeshes.size(); i++ )
			m_DrawList.push_back( { .entity = entity, .Mesh = mesh, .Transform = transform, .SubmeshIndex = ( uint32_t ) i } );

		m_ShadowMapDrawList.push_back( { .entity = entity, .Mesh = mesh, .Transform = transform, .SubmeshIndex = ( uint32_t ) 0 } );
	}

	void SceneRenderer::SubmitMesh( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		auto& submeshes = mesh->Submeshes();
		for( size_t i = 0; i < submeshes.size(); i++ )
			m_DrawList.push_back( { .entity = entity, .Mesh = mesh, .Transform = transform, .SubmeshIndex = (uint32_t)i } );

		m_ShadowMapDrawList.push_back( { .entity = entity, .Mesh = mesh, .Transform = transform, .SubmeshIndex = ( uint32_t ) 0 } );
	}

	void SceneRenderer::SetViewportSize( uint32_t w, uint32_t h )
	{
		if( m_RendererData.Width != w && m_RendererData.Width != h )
		{
			m_RendererData.Width = w;
			m_RendererData.Height = h;
			m_RendererData.Resized = true;
		}
	}

	void SceneRenderer::Recreate()
	{
		InitPreDepth();

		InitGeometryPass();
		InitSceneComposite();

		//InitAO();
		//InitAOComposite();

		glm::uvec2 bs = ( glm::uvec2( m_RendererData.Width, m_RendererData.Height ) + 1u ) / 2u;
		bs += m_RendererData.m_BloomWorkSize - bs % m_RendererData.m_BloomWorkSize;

		for( uint32_t i = 0; i < 3; i++ )
		{
			m_RendererData.BloomTextures[ i ]->Terminate();

			m_RendererData.BloomTextures[ i ] = Ref<Texture2D>::Create( ImageFormat::RGBA32F, bs.x, bs.y, nullptr, true );
			m_RendererData.BloomTextures[ i ]->SetDebugName( "Bloom Texture: " + std::to_string( i ) );
		}

		m_RendererData.SceneCompositeShader->WriteDescriptor( "u_BloomTexture", m_RendererData.BloomTextures[ 2 ]->GetDescriptorInfo(), m_RendererData.SC_DescriptorSet->GetVulkanSet() );

		//InitBloom();

		CreateSkyboxComponents();
		CreateGridComponents();
	}

	void SceneRenderer::GeometryPass()
	{
		m_RendererData.GeometryPassTimer.Reset();

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();

		VkExtent2D Extent = { m_RendererData.Width, m_RendererData.Height };

		// Prepare skybox
		PrepareSkybox();

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
		
		vkCmdSetScissor( m_RendererData.CommandBuffer, 0, 1, &Scissor );
		vkCmdSetViewport( m_RendererData.CommandBuffer, 0, 1, &Viewport );

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

		// Set environment resource.
		Renderer::Get().SetSceneEnvironment( m_RendererData.ShadowCascades[ 0 ].Framebuffer->GetDepthAttachmentsResource(), m_RendererData.SceneEnvironment, m_RendererData.BRDFLUT_Texture );

		// Render static meshes.
		Ref< Shader > StaticMeshShader = m_RendererData.StaticMeshShader;

		for( auto& Cmd : m_DrawList )
		{
			// Entity may of been deleted.
			if( !Cmd.entity )
				continue;

			auto& uuid = Cmd.entity.GetComponent<IdComponent>().ID;

			// u_Matrices
			RendererData::StaticMeshMatrices u_Matrices = {};
			u_Matrices.View = m_RendererData.EditorCamera.ViewMatrix();
			u_Matrices.ViewProjection = m_RendererData.EditorCamera.ProjectionMatrix() * m_RendererData.EditorCamera.ViewMatrix();

			struct
			{
				glm::mat4 LightMatrix[4];
			} u_LightData = {};

			RendererData::PointLights u_Lights;

			u_Lights.nbLights = int( m_pScene->m_Lights.PointLights.size() );

			std::memcpy( u_Lights.Lights, m_pScene->m_Lights.PointLights.data(), m_pScene->m_Lights.GetPointLightSize() );

			struct SceneData
			{
				DirLight Lights;
				glm::vec3 CameraPosition;
			} u_SceneData = {};
			
			struct ShadowData
			{
				glm::vec4 CascadeSplits;
			} u_ShadowData = {};

			struct DebugData
			{
				float ShowLightComplexity;
				float ShowLightCulling;

				int TilesCountX;
			} u_DebugData = {};

			u_DebugData.ShowLightComplexity = ( float ) m_RendererData.ShowLightComplexity;
			u_DebugData.ShowLightCulling = ( float ) m_RendererData.ShowLightCulling;

			u_DebugData.TilesCountX = m_RendererData.LightCullingWorkGroups.x;

			auto dirLight = m_pScene->m_Lights.DirectionalLights[ 0 ];
			
			u_SceneData.CameraPosition = m_RendererData.EditorCamera.GetPosition();
			u_SceneData.Lights = { .Direction = dirLight.Direction, .Radiance = dirLight.Radiance, .Multiplier = dirLight.Intensity };

			if( m_RendererData.EnableShadows )
			{
				for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
				{
					u_ShadowData.CascadeSplits[ i ] = m_RendererData.ShadowCascades[ i ].SplitDepth;
					u_LightData.LightMatrix[ i ] = m_RendererData.ShadowCascades[ i ].ViewProjection;
				}
			}

			StaticMeshShader->UploadUB( ShaderType::Vertex, 0, 0, &u_Matrices, sizeof( u_Matrices ) );
			StaticMeshShader->UploadUB( ShaderType::Vertex, 0, 1, &u_LightData, sizeof( u_LightData ) );

			StaticMeshShader->UploadUB( ShaderType::Fragment, 0, 2, &u_SceneData, sizeof( u_SceneData ) );
			StaticMeshShader->UploadUB( ShaderType::Fragment, 0, 3, &u_ShadowData, sizeof( u_ShadowData ) );
			StaticMeshShader->UploadUB( ShaderType::Fragment, 0, 12, &u_DebugData, sizeof( u_DebugData ) );

			//StaticMeshShader->UploadUB( ShaderType::Fragment, 0, 13, &u_Lights, sizeof( u_Lights ) );
			StaticMeshShader->UploadUB( ShaderType::Fragment, 0, 13, &u_Lights, 16ull + sizeof( PointLight ) * u_Lights.nbLights );

			// Render
			Renderer::Get().SubmitMesh( m_RendererData.CommandBuffer,
				m_RendererData.StaticMeshPipeline, 
				Cmd.Mesh, m_RendererData.StorageBufferSet, Cmd.Transform, Cmd.SubmeshIndex );
		}
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		//////////////////////////////////////////////////////////////////////////
		
		// End geometry pass.
		m_RendererData.GeometryPass->EndPass();

		m_RendererData.GeometryPassTimer.Stop();
	}

	void SceneRenderer::DirShadowMapPass()
	{
		if( !m_RendererData.EnableShadows )
			return;

		auto pAllocator = VulkanContext::Get().GetVulkanAllocator();
		VkExtent2D Extent = { SHADOW_MAP_SIZE, SHADOW_MAP_SIZE };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		std::array<VkClearValue, 2> ClearColors{};
		ClearColors[ 0 ].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo RenderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
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
		
		//////////////////////////////////////////////////////////////////////////

		Ref< Shader > ShadowShader = m_RendererData.DirShadowMapShader;

		UpdateCascades( m_pScene->m_Lights.DirectionalLights[ 0 ].Direction );
		
		// u_Matrices
		struct UB_Matrices
		{
			glm::mat4 ViewProjection[4];
		} u_Matrices;

		u_Matrices = {};

		for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			u_Matrices.ViewProjection[ i ] = m_RendererData.ShadowCascades[ i ].ViewProjection;
		}

		auto pData = m_RendererData.DirShadowMapShader->MapUB( ShaderType::Vertex, 0, 0 );

		memcpy( pData, &u_Matrices, sizeof( u_Matrices ) );

		m_RendererData.DirShadowMapShader->UnmapUB( ShaderType::Vertex, 0, 0 );

		for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			m_RendererData.ShadowMapTimers[ i ].Reset();

			RenderPassBeginInfo.framebuffer = m_RendererData.ShadowCascades[ i ].Framebuffer->GetVulkanFramebuffer();
			RenderPassBeginInfo.renderPass = m_RendererData.DirShadowMapPasses[ i ]->GetVulkanPass();
			m_RendererData.DirShadowMapPipelines[ i ]->GetShader()->WriteAllUBs( m_RendererData.DirShadowMapPipelines[ i ]->GetDescriptorSet( ShaderType::Vertex, 0 ) );

			// Begin directional shadow map pass.
			CmdBeginDebugLabel( CommandBuffer, "DirShadowMap" );
			vkCmdBeginRenderPass( CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			vkCmdSetViewport( m_RendererData.CommandBuffer, 0, 1, &Viewport );
			vkCmdSetScissor( m_RendererData.CommandBuffer, 0, 1, &Scissor );

			for( auto& Cmd : m_ShadowMapDrawList )
			{
				// Entity may of been deleted.
				if( !Cmd.entity )
					continue;
				
				// Pass in the cascade index.
				Buffer AdditionalData( sizeof(uint32_t), &i );

				Renderer::Get().RenderMeshWithoutMaterial( CommandBuffer, m_RendererData.DirShadowMapPipelines[ i ], Cmd.Mesh, Cmd.Transform, AdditionalData );
			}

			vkCmdEndRenderPass( CommandBuffer );
			CmdEndDebugLabel( CommandBuffer );

			m_RendererData.ShadowMapTimers[ i ].Stop();
		}
	}

	void SceneRenderer::PreDepthPass()
	{
		m_RendererData.PreDepthTimer.Reset();

		VkExtent2D Extent = { m_RendererData.Width,m_RendererData.Height };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		m_RendererData.PreDepthPass->BeginPass( CommandBuffer, m_RendererData.PreDepthFramebuffer->GetVulkanFramebuffer(), Extent );

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

		// u_Matrices
		struct UB_Matrices
		{
			glm::mat4 ViewProjection;
		} u_Matrices;

		u_Matrices.ViewProjection = m_RendererData.EditorCamera.ViewProjection();

		m_RendererData.PreDepthShader->UploadUB( ShaderType::Vertex, 0, 0, &u_Matrices, sizeof( u_Matrices ) );

		m_RendererData.PreDepthShader->WriteAllUBs( m_RendererData.PreDepthPipeline->GetDescriptorSet( ShaderType::Vertex, 0 ) );

		// We can use the shadow map draw list for pre depth.
		for( auto& Cmd : m_ShadowMapDrawList )
		{
			// Entity may of been deleted.
			if( !Cmd.entity )
				continue;

			Renderer::Get().RenderMeshWithoutMaterial( CommandBuffer, m_RendererData.PreDepthPipeline, Cmd.Mesh, Cmd.Transform );
		}

		m_RendererData.PreDepthPass->EndPass();

		m_RendererData.PreDepthTimer.Stop();
	}

	void SceneRenderer::SceneCompositePass()
	{
		VkExtent2D Extent = { m_RendererData.Width,m_RendererData.Height };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		// Begin scene composite pass.
		m_RendererData.SceneComposite->BeginPass( CommandBuffer, m_RendererData.SceneCompositeFramebuffer->GetVulkanFramebuffer(), Extent );

		VkViewport Viewport = {};
		Viewport.x = 0;
		Viewport.y = 0;
		Viewport.width = ( float )m_RendererData.Width;
		Viewport.height = ( float )m_RendererData.Height;
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;

		VkRect2D Scissor = { .offset = { 0, 0 }, .extent = Extent };

		vkCmdSetViewport( CommandBuffer, 0, 1, &Viewport );
		vkCmdSetScissor( CommandBuffer, 0, 1, &Scissor );
		
		// Actual scene composite pass.
		
		m_RendererData.SceneCompositeShader->WriteAllUBs( m_RendererData.SC_DescriptorSet );

		Renderer::Get().SubmitFullscreenQuad( 
			CommandBuffer, m_RendererData.SceneCompositePipeline, 
			m_RendererData.SC_DescriptorSet, 
			m_RendererData.SC_IndexBuffer, m_RendererData.SC_VertexBuffer );
		
		// End scene composite pass.
		m_RendererData.SceneComposite->EndPass();
	}

	void SceneRenderer::AOCompositePass()
	{
		VkExtent2D Extent = { m_RendererData.Width, m_RendererData.Height };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		m_RendererData.AOComposite->BeginPass( CommandBuffer, m_RendererData.AOCompositeFramebuffer->GetVulkanFramebuffer(), Extent );

		m_RendererData.AOCompositeTimer.Reset();

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

		// Draw
		// We can use the SSAO vertex and index buffers.
		//Renderer::Get().SubmitFullscreenQuad(
		//	m_RendererData.CommandBuffer, m_RendererData.AOCompositePipeline, m_RendererData.AO_DescriptorSet, m_RendererData.SSAO_IndexBuffer, m_RendererData.SSAO_VertexBuffer );

		m_RendererData.AOComposite->EndPass();

		m_RendererData.AOCompositeTimer.Stop();
	}

	struct UBLights
	{
		uint32_t nbLights;
		PointLight Lights[ 1024 ];
	};

	void SceneRenderer::LightCullingPass()
	{
		m_RendererData.LightCullingTimer.Reset();

		constexpr uint32_t TILE_SIZE = 16;
		glm::uvec2 Viewport = { m_RendererData.Width, m_RendererData.Height };
		glm::uvec2 Size = Viewport;
		Size += TILE_SIZE - Viewport % TILE_SIZE;
		
		m_RendererData.LightCullingWorkGroups = { Size / TILE_SIZE, 1 };

		m_RendererData.StorageBufferSet->Resize( 0, 14, m_RendererData.LightCullingWorkGroups.x * m_RendererData.LightCullingWorkGroups.y * 4 * 1024 );

		// UBs
		UBLights u_Lights;

		struct 
		{
			glm::vec2 FullResolution;
		} u_ScreenData{};

		struct 
		{
			glm::mat4 ViewProjection;
			glm::mat4 Projection;
			glm::mat4 View;
			glm::mat4 InvP;
		} u_Matrices{};

		u_Matrices.ViewProjection   = m_RendererData.EditorCamera.ViewProjection();
		u_Matrices.Projection       = m_RendererData.EditorCamera.ProjectionMatrix();
		u_Matrices.View             = glm::inverse( m_RendererData.EditorCamera.ViewMatrix() );
		u_Matrices.InvP				= glm::inverse( u_Matrices.Projection );

		u_ScreenData.FullResolution = { m_RendererData.Width, m_RendererData.Height };

		u_Lights.nbLights = m_pScene->m_Lights.PointLights.size();

		std::memcpy( u_Lights.Lights, m_pScene->m_Lights.PointLights.data(), m_pScene->m_Lights.GetPointLightSize() );

		m_RendererData.LightCullingShader->UploadUB( ShaderType::Compute, 0, 0, &u_Lights, 16ull + sizeof PointLight * u_Lights.nbLights );
		m_RendererData.LightCullingShader->UploadUB( ShaderType::Compute, 0, 3, &u_ScreenData, sizeof( u_ScreenData ) );
		m_RendererData.LightCullingShader->UploadUB( ShaderType::Compute, 0, 4, &u_Matrices, sizeof( u_Matrices ) );

		m_RendererData.LightCullingShader->WriteAllUBs( m_RendererData.LightCullingDescriptorSet );

		// Write sb
		auto& rSB = m_RendererData.StorageBufferSet->Get( 0, 14 );

		VkDescriptorBufferInfo Info = { .buffer = rSB.Buffer, .offset = 0, .range = rSB.Size };

		m_RendererData.LightCullingShader->WriteSB( 0, 14, Info, m_RendererData.LightCullingDescriptorSet );

		// Light culling here
		auto& CullingPipeline = m_RendererData.LightCullingPipeline;

		CullingPipeline->BindWithCommandBuffer( m_RendererData.CommandBuffer );

		CullingPipeline->Execute(
			m_RendererData.LightCullingDescriptorSet->GetVulkanSet(),
			m_RendererData.LightCullingWorkGroups.x,
			m_RendererData.LightCullingWorkGroups.y,
			m_RendererData.LightCullingWorkGroups.z );

		VkMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier( CullingPipeline->GetCommandBuffer(),
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				1, &barrier,
				0, nullptr,
				0, nullptr );

		CullingPipeline->Unbind();

		m_RendererData.LightCullingTimer.Stop();
	}

	void SceneRenderer::BloomPass()
	{
		// Reset the descriptor pool.
		VK_CHECK( vkResetDescriptorPool( VulkanContext::Get().GetDevice(), m_RendererData.BloomDescriptorPool, 0 ) );

		m_RendererData.BloomTimer.Reset();

		struct u_Settings
		{
			float Threshold;
			float Knee;
			float TK; // Threshold - Knee
			float DK; // Knee * 2.0f
			float QK; // Knee / 0.25f
			float Stage; // -1 = Prefilter, 0 = Downsample, 1 = Upsample
			float LOD;
		} pc_Settings{};

		pc_Settings.LOD = 0.0f;
		pc_Settings.Threshold = 1.5f;
		pc_Settings.Knee = 0.1f;

		pc_Settings.TK = pc_Settings.Knee - pc_Settings.Threshold;
		pc_Settings.DK = pc_Settings.Knee * 2.0f;
		pc_Settings.QK = pc_Settings.Knee / 0.25f;

		auto& shader = m_RendererData.BloomShader;
		auto& pipeline = m_RendererData.BloomComputePipeline;

		DescriptorSetSpecification spec;
		spec.Layout = shader->GetSetLayout();
		spec.SetIndex = 0;
		spec.Pool = shader->GetDescriptorPool();

		VkDescriptorSet descriptorSet;
		VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		info.descriptorPool = m_RendererData.BloomDescriptorPool;
		info.descriptorSetCount = 1;
		info.pSetLayouts = shader->GetSetLayouts().data();

		auto& InputImg = m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 0 ];

		glm::vec2 workgrps{};

		uint32_t mips = m_RendererData.BloomTextures[ 0 ]->GetMipMapLevels();

		if( mips == 1 )
			return;

		mips = mips - 2;

		// Step 0: Bind compute pipeline in graphics queue.
		// Make sure to do it on the graphics queue.
		pipeline->BindWithCommandBuffer( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Prefilter" );

		// Step 0.5: Prefilter.
		// Here we are just getting anything that is brighter than the bloom threshold.
		pc_Settings.Stage = -1;

		{
			VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &info, &descriptorSet ) );

			shader->WriteDescriptor( "u_InputTexture", InputImg->GetDescriptorInfo(), descriptorSet );
			shader->WriteDescriptor( "u_BloomTexture", InputImg->GetDescriptorInfo(), descriptorSet );

			auto descriptorInfo = m_RendererData.BloomTextures[ 0 ]->GetDescriptorInfo();
			descriptorInfo.imageView = m_RendererData.BloomTextures[ 0 ]->GetOrCreateMipImageView( 0 );

			shader->WriteDescriptor( "o_Image", descriptorInfo, descriptorSet );

			workgrps.x = m_RendererData.BloomTextures[ 0 ]->Width() / m_RendererData.m_BloomWorkSize;
			workgrps.y = m_RendererData.BloomTextures[ 0 ]->Height() / m_RendererData.m_BloomWorkSize;

			pipeline->AddPushConstant( &pc_Settings, 0, sizeof( pc_Settings ) );
			pipeline->Execute( descriptorSet, workgrps.x, workgrps.y, 1 );
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Downsample" );

		// Step 1: Downsample.
		pc_Settings.Stage = 0;
		for( uint32_t i = 1; i < mips; i++ )
		{
			auto [w, h] = m_RendererData.BloomTextures[ 0 ]->GetMipSize( i );

			workgrps.x = ( uint32_t ) glm::ceil( ( float ) w / m_RendererData.m_BloomWorkSize );
			workgrps.y = ( uint32_t ) glm::ceil( ( float ) h / m_RendererData.m_BloomWorkSize );

			{
				auto descriptorInfo = m_RendererData.BloomTextures[ 1 ]->GetDescriptorInfo();
				descriptorInfo.imageView = m_RendererData.BloomTextures[ 1 ]->GetOrCreateMipImageView( i );

				VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &info, &descriptorSet ) );

				shader->WriteDescriptor( "u_InputTexture", m_RendererData.BloomTextures[ 0 ]->GetDescriptorInfo(), descriptorSet );
				shader->WriteDescriptor( "u_BloomTexture", InputImg->GetDescriptorInfo(), descriptorSet );

				shader->WriteDescriptor( "o_Image", descriptorInfo, descriptorSet );

				pc_Settings.LOD = i - 1.0f;

				// Step 1.5: Set push constants & Execute the compute shader.
				pipeline->AddPushConstant( &pc_Settings, 0, sizeof( pc_Settings ) );
				pipeline->Execute( descriptorSet, workgrps.x, workgrps.y, 1 );
			}

			// Now we do the same again.
			// However this time we swap the textures.
			{
				VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &info, &descriptorSet ) );

				shader->WriteDescriptor( "u_InputTexture", m_RendererData.BloomTextures[ 1 ]->GetDescriptorInfo(), descriptorSet );
				shader->WriteDescriptor( "u_BloomTexture", InputImg->GetDescriptorInfo(), descriptorSet );

				auto descriptorInfo = m_RendererData.BloomTextures[ 0 ]->GetDescriptorInfo();
				descriptorInfo.imageView = m_RendererData.BloomTextures[ 0 ]->GetOrCreateMipImageView( i );

				shader->WriteDescriptor( "o_Image", descriptorInfo, descriptorSet );

				pc_Settings.LOD = i;

				pipeline->AddPushConstant( &pc_Settings, 0, sizeof( pc_Settings ) );
				pipeline->Execute( descriptorSet, workgrps.x, workgrps.y, 1 );
			}
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "First Upsample" );

		// Step 2: First upsample.
		pc_Settings.Stage = -2;
		workgrps.x *= 2;
		workgrps.y *= 2;

		{
			VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &info, &descriptorSet ) );

			shader->WriteDescriptor( "u_InputTexture", m_RendererData.BloomTextures[ 0 ]->GetDescriptorInfo(), descriptorSet );
			shader->WriteDescriptor( "u_BloomTexture", InputImg->GetDescriptorInfo(), descriptorSet );

			auto info = m_RendererData.BloomTextures[ 0 ]->GetDescriptorInfo();
			info.imageView = m_RendererData.BloomTextures[ 2 ]->GetOrCreateMipImageView( mips - 2 );

			shader->WriteDescriptor( "o_Image", info, descriptorSet );

			pc_Settings.LOD--;

			auto [w, h] = m_RendererData.BloomTextures[ 2 ]->GetMipSize( mips - 2 );

			workgrps.x = ( uint32_t ) glm::ceil( ( float ) w / m_RendererData.m_BloomWorkSize );
			workgrps.y = ( uint32_t ) glm::ceil( ( float ) h / m_RendererData.m_BloomWorkSize );

			pipeline->AddPushConstant( &pc_Settings, 0, sizeof( pc_Settings ) );
			pipeline->Execute( descriptorSet, workgrps.x, workgrps.y, 1 );
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Upsample" );

		// Step 3: Upsample.
		pc_Settings.Stage = 1;
		for( int32_t mip = mips - 3; mip >= 0; mip-- )
		{
			auto [w, h] = m_RendererData.BloomTextures[ 2 ]->GetMipSize( mip );

			workgrps.x = ( uint32_t ) glm::ceil( ( float ) w / m_RendererData.m_BloomWorkSize );
			workgrps.y = ( uint32_t ) glm::ceil( ( float ) h / m_RendererData.m_BloomWorkSize );

			VK_CHECK( vkAllocateDescriptorSets( VulkanContext::Get().GetDevice(), &info, &descriptorSet ) );

			{
				shader->WriteDescriptor( "u_InputTexture", m_RendererData.BloomTextures[ 0 ]->GetDescriptorInfo(), descriptorSet );
				shader->WriteDescriptor( "u_BloomTexture", m_RendererData.BloomTextures[ 2 ]->GetDescriptorInfo(), descriptorSet );

				auto info = m_RendererData.BloomTextures[ 0 ]->GetDescriptorInfo();
				info.imageView = m_RendererData.BloomTextures[ 2 ]->GetOrCreateMipImageView( mip );

				shader->WriteDescriptor( "o_Image", info, descriptorSet );

				pc_Settings.LOD = mip;

				// Step 2.5: Set push constants & Execute the compute shader.
				pipeline->AddPushConstant( &pc_Settings, 0, sizeof( pc_Settings ) );
				pipeline->Execute( descriptorSet, workgrps.x, workgrps.y, 1 );
			}
		}

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		pipeline->Unbind();

		m_RendererData.BloomTimer.Stop();
	}

	void SceneRenderer::AddScheduledFunction( ScheduledFunc&& rrFunc )
	{
		m_ScheduledFunctions.push_back( rrFunc );
	}

	Ref<TextureCube> SceneRenderer::CreateDymanicSky()
	{
		const uint32_t cubemapSize = 512;
		const uint32_t irradianceMap = 32;

		Ref<TextureCube> Environment = Ref<TextureCube>::Create( ImageFormat::RGBA32F, cubemapSize, cubemapSize );

		Ref<Shader> skyShader = ShaderLibrary::Get().Find( "Skybox_Compute" );
		Ref<ComputePipeline> pipeline = Ref<ComputePipeline>::Create( skyShader );

		glm::vec3 params = { m_RendererData.SceneEnvironment->Turbidity, m_RendererData.SceneEnvironment->Azimuth, m_RendererData.SceneEnvironment->Inclination };

		skyShader->WriteDescriptor( "o_CubeMap", Environment->GetDescriptorInfo(), m_RendererData.PreethamDescriptorSet->GetVulkanSet() );

		auto CommandBuffer = m_RendererData.CommandBuffer;

		pipeline->Bind();
		pipeline->AddPushConstant( &params, 0, sizeof( glm::vec3 ) );
		pipeline->Execute( m_RendererData.PreethamDescriptorSet->GetVulkanSet(), cubemapSize / 32, cubemapSize / 32, 6 );
		pipeline->Unbind();

		Environment->CreateMips();

		pipeline = nullptr;

		return Environment;
	}

	void SceneRenderer::SetDynamicSky( float Turbidity, float Azimuth, float Inclination )
	{
		float turbidity = Turbidity;
		float azimuth = Azimuth;
		float inclination = Inclination;

		AddScheduledFunction( [&, turbidity, azimuth, inclination]()
		{
			m_RendererData.SceneEnvironment->Turbidity = turbidity;
			m_RendererData.SceneEnvironment->Azimuth = azimuth;
			m_RendererData.SceneEnvironment->Inclination = inclination;

			m_RendererData.SceneEnvironment->IrradianceMap = nullptr;
			m_RendererData.SceneEnvironment->RadianceMap = nullptr;

			Ref<TextureCube> map = CreateDymanicSky();

			m_RendererData.SceneEnvironment->IrradianceMap = map;
			m_RendererData.SceneEnvironment->RadianceMap = map;

		} );
	}

	void SceneRenderer::RenderScene()
	{
		if( !m_pScene )
		{
			FlushDrawList();
			return;
		}

		if( m_RendererData.Resized )
		{
			Recreate();

			m_RendererData.Resized = false;
		}

		m_RendererData.CommandBuffer = Renderer::Get().ActiveCommandBuffer();
		
		for( auto&& func : m_ScheduledFunctions )
			func();

		// Passes

		DirShadowMapPass();

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "PreDepth" );

		PreDepthPass();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "LightCulling" );

		LightCullingPass();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Geometry" );

		GeometryPass();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );
		
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Bloom" );

		BloomPass();

		//m_BloomComputeFunction();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Scene Composite - Post Processing" );

		SceneCompositePass();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		FlushDrawList();
	}

	void SceneRenderer::FlushDrawList()
	{
		m_DrawList.clear();
		m_ShadowMapDrawList.clear();
		m_ScheduledFunctions.clear();
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
		
		// Command Pools
		vkDestroyCommandPool( LogicalDevice, CommandPool, nullptr );

		if( !Application::Get().GetSpecification().CreateSceneRenderer )
			return;
	
		// DescriptorSets
		GridDescriptorSet = nullptr;
		SkyboxDescriptorSet = nullptr;
		SC_DescriptorSet = nullptr;
		PreethamDescriptorSet = nullptr;
		LightCullingDescriptorSet = nullptr;

		// Vertex and Index buffers
		GridVertexBuffer->Terminate();
		GridIndexBuffer->Terminate();
		SkyboxVertexBuffer->Terminate();
		SkyboxIndexBuffer->Terminate();
		SC_VertexBuffer->Terminate();
		SC_IndexBuffer->Terminate();

		// Framebuffers
		GeometryFramebuffer = nullptr;
		SceneCompositeFramebuffer = nullptr;
		PreDepthFramebuffer = nullptr;

		for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
			ShadowCascades[ i ].Framebuffer = nullptr;
		
		ShadowCascades.clear();

		// Render Passes
		for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
			DirShadowMapPasses[ i ]->Terminate();

		GeometryPass->Terminate();
		SceneComposite->Terminate();

		GeometryPass = nullptr;
		SceneComposite = nullptr;

		PreDepthPass->Terminate();

		// Pipelines
		SceneCompositePipeline = nullptr;

		for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
			DirShadowMapPipelines[ i ] = nullptr;

		StaticMeshPipeline = nullptr;
		GridPipeline = nullptr;
		SkyboxPipeline = nullptr;
		PreDepthPipeline = nullptr;
		LightCullingPipeline = nullptr;

		// Shaders
		GridShader = nullptr;
		SkyboxShader = nullptr;
		StaticMeshShader = nullptr;
		SceneCompositeShader = nullptr;
		DirShadowMapShader = nullptr;
		PreethamShader = nullptr;
		AOCompositeShader = nullptr;
		PreDepthShader = nullptr;
		LightCullingShader = nullptr;

		// Textures
		BRDFLUT_Texture = nullptr;
		
		SceneEnvironment = nullptr;

		ShaderLibrary::Get().Shutdown();
	}

}