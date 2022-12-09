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

		InitAO();

		InitAOComposite();

		InitSceneComposite();

		m_RendererData.SceneEnvironment = Ref<EnvironmentMap>::Create();

		m_RendererData.BRDFLUT_Texture = Ref<Texture2D>::Create( "assets/textures/BRDF_LUT.tga", AddressingMode::Repeat, false );

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
			PassSpec.Attachments = { ImageFormat::RGBA32F, ImageFormat::RGBA16F, ImageFormat::RGBA16F, ImageFormat::Depth };
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
			PassSpec.Attachments = { ImageFormat::Depth };

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
			FBSpec.Attachments = { ImageFormat::Depth };

			m_RendererData.PreDepthFramebuffer = Ref<Framebuffer>::Create( FBSpec );
		}

		if( !m_RendererData.PreDepthShader ) 
		{
			ShaderLibrary::Get().Load( "assets/shaders/PreDepth.glsl" );
			ShaderLibrary::Get().Load( "assets/shaders/LightCulling.glsl" );
			m_RendererData.PreDepthShader = ShaderLibrary::Get().Find( "PreDepth" );
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
		PipelineSpec.CullMode = CullMode::None;
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
		Ref<Shader> lightCullingShader;
		lightCullingShader = ShaderLibrary::Get().Find( "LightCulling" );

		m_RendererData.LightCullingPipeline = Ref<ComputePipeline>::Create( lightCullingShader );

		if( !m_RendererData.LightCullingDescriptorSet )
			m_RendererData.LightCullingDescriptorSet = lightCullingShader->CreateDescriptorSet( 0 );

		lightCullingShader->WriteDescriptor( "u_PreDepth", m_RendererData.PreDepthFramebuffer->GetDepthAttachmentsResource()->GetDescriptorInfo(), m_RendererData.LightCullingDescriptorSet->GetVulkanSet() );

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

	void SceneRenderer::InitAO()
	{
		PassSpecification PassSpec = {};
		PassSpec.Name = "SSAO";
		PassSpec.Attachments = { ImageFormat::RED8 };

		if( m_RendererData.SSAORenderPass )
			m_RendererData.SSAORenderPass->Recreate();
		else
			m_RendererData.SSAORenderPass = Ref<Pass>::Create( PassSpec );

		PassSpec.Name = "SSAO-Blur";

		if( m_RendererData.SSAOBlurRenderPass )
			m_RendererData.SSAOBlurRenderPass->Recreate();
		else
			m_RendererData.SSAOBlurRenderPass = Ref<Pass>::Create( PassSpec );

		FramebufferSpecification FBSpec = {};
		FBSpec.RenderPass = m_RendererData.SSAORenderPass;
		FBSpec.Width = m_RendererData.Width;
		FBSpec.Height = m_RendererData.Height;
		FBSpec.CreateDepth = false;

		FBSpec.Attachments = { ImageFormat::RED8 };

		m_RendererData.SSAOFramebuffer = Ref<Framebuffer>::Create( FBSpec );

		FBSpec.RenderPass = m_RendererData.SSAOBlurRenderPass;

		m_RendererData.SSAOBlurFramebuffer = Ref<Framebuffer>::Create( FBSpec );

		if( !m_RendererData.SSAOShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/SSAO.glsl" );
			m_RendererData.SSAOShader = ShaderLibrary::Get().Find( "SSAO" );
		}

		if( !m_RendererData.SSAOBlurShader )
		{
			ShaderLibrary::Get().Load( "assets/shaders/SSAO-Blur.glsl" );
			m_RendererData.SSAOBlurShader = ShaderLibrary::Get().Find( "SSAO-Blur" );
		}

		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = m_RendererData.Width;
		PipelineSpec.Height = m_RendererData.Height;
		PipelineSpec.Name = "SSAO";
		PipelineSpec.Shader = m_RendererData.SSAOShader.Pointer();
		PipelineSpec.RenderPass = m_RendererData.SSAORenderPass;
		PipelineSpec.UseDepthTest = false;
		PipelineSpec.CullMode = CullMode::None;
		PipelineSpec.FrontFace = VK_FRONT_FACE_CLOCKWISE;
		PipelineSpec.VertexLayout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" },
		};

		struct SpecializationInfo
		{
			uint32_t krnlSize = 32;
			float    radius = 0.3F;
		} _SpecializationInfo;

		VkSpecializationMapEntry a = { 0, offsetof( SpecializationInfo, krnlSize ), sizeof( SpecializationInfo::krnlSize ) };
		VkSpecializationMapEntry b = { 1, offsetof( SpecializationInfo, radius ), sizeof( SpecializationInfo::radius ) };

		std::array<VkSpecializationMapEntry, 2> SpecializationEntries = 
		{
			a,
			b
		};

		VkSpecializationInfo info = {};
		info.mapEntryCount = 2;
		info.pMapEntries = SpecializationEntries.data();
		info.dataSize = sizeof( _SpecializationInfo );
		info.pData = &_SpecializationInfo;

		//PipelineSpec.SpecializationInfo = info;
		//PipelineSpec.UseSpecializationInfo = true;
		//PipelineSpec.SpecializationStage = ShaderType::Fragment;

		if( m_RendererData.SSAOPipeline )
			m_RendererData.SSAOPipeline = nullptr;

		if( m_RendererData.SSAOBlurPipeline )
			m_RendererData.SSAOPipeline = nullptr;

		m_RendererData.SSAOPipeline = Ref<Pipeline>::Create( PipelineSpec );

		if( !m_RendererData.SSAO_DescriptorSet )
			m_RendererData.SSAO_DescriptorSet = m_RendererData.SSAOShader->CreateDescriptorSet( 0 );

		m_RendererData.SSAOShader->WriteDescriptor( "u_ViewNormalTexture", m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 0 ]->GetDescriptorInfo(), m_RendererData.SSAO_DescriptorSet->GetVulkanSet() );

		m_RendererData.SSAOShader->WriteDescriptor( "u_DepthTexture", m_RendererData.ShadowCascades[0].Framebuffer->GetDepthAttachmentsResource()->GetDescriptorInfo(), m_RendererData.SSAO_DescriptorSet->GetVulkanSet() );

		m_RendererData.SSAOShader->WriteAllUBs( m_RendererData.SSAO_DescriptorSet );

		PipelineSpec.Name = "SSAO-Blur";
		PipelineSpec.Shader = m_RendererData.SSAOBlurShader;
		PipelineSpec.RenderPass = m_RendererData.SSAORenderPass;
		PipelineSpec.SpecializationInfo = {};
		PipelineSpec.UseSpecializationInfo = false;
		PipelineSpec.SpecializationStage = ShaderType::None;

		m_RendererData.SSAOBlurPipeline = Ref<Pipeline>::Create( PipelineSpec );

		if( !m_RendererData.SSAO_VertexBuffer && !m_RendererData.SSAO_IndexBuffer )
			Renderer::Get().CreateFullscreenQuad( &m_RendererData.SSAO_VertexBuffer, &m_RendererData.SSAO_IndexBuffer );

		// Create noise texture for SSAO
		std::default_random_engine rndEngine( time( nullptr ) );
		std::uniform_real_distribution<float> rndDist( 0.0f, 1.0f );

		std::vector<glm::vec4> ssaoNoise( m_RendererData.SSAO_NOISE_DIM* m_RendererData.SSAO_NOISE_DIM );
		for( uint32_t i = 0; i < static_cast< uint32_t >( ssaoNoise.size() ); i++ )
		{
			ssaoNoise[ i ] = glm::vec4( rndDist( rndEngine ) * 2.0f - 1.0f, rndDist( rndEngine ) * 2.0f - 1.0f, 0.0f, 0.0f );
		}

		m_RendererData.SSAO_NoiseImage = Ref<Image2D>::Create( ImageFormat::RGBA32F, 4, 4, 1, ssaoNoise.data(), ssaoNoise.size() * sizeof( glm::vec4 ) );

		auto lerp = []( float a, float b, float f )
		{
			return a + f * ( b - a );
		};

		m_RendererData.SSAOKernel.resize( m_RendererData.SSAO_KERNEL_SIZE );

		for( uint32_t i = 0; i < m_RendererData.SSAO_KERNEL_SIZE; i++ )
		{
			glm::vec3 sample( rndDist( rndEngine ) * 2.0 - 1.0, rndDist( rndEngine ) * 2.0 - 1.0, rndDist( rndEngine ) );
			sample = glm::normalize( sample );
			sample *= rndDist( rndEngine );
			float scale = float( i ) / float( m_RendererData.SSAO_KERNEL_SIZE );
			scale = lerp( 0.1f, 1.0f, scale * scale );
			m_RendererData.SSAOKernel[ i ] = glm::vec4( sample * scale, 0.0f );
		}

		m_RendererData.SSAOShader->WriteDescriptor( "u_NoiseTexture", m_RendererData.SSAO_NoiseImage->GetDescriptorInfo(), m_RendererData.SSAO_DescriptorSet->GetVulkanSet() );

		m_RendererData.SSAOShader->WriteAllUBs( m_RendererData.SSAO_DescriptorSet );
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

		m_RendererData.AOCompositeShader->WriteDescriptor( "u_AOTexture", m_RendererData.SSAOFramebuffer->GetColorAttachmentsResources()[ 0 ]->GetDescriptorInfo(), m_RendererData.AO_DescriptorSet->GetVulkanSet() );

		m_RendererData.AOCompositeShader->WriteDescriptor( "u_AlbedoTexture", m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 2 ]->GetDescriptorInfo(), m_RendererData.AO_DescriptorSet->GetVulkanSet() );

		m_RendererData.AOCompositeShader->WriteDescriptor( "u_TestTexture", m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 0 ]->GetDescriptorInfo(), m_RendererData.AO_DescriptorSet->GetVulkanSet() );

		m_RendererData.AOCompositeShader->WriteAllUBs( m_RendererData.AO_DescriptorSet );
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

			ImGui::Text( "SceneRenderer::ShadowMapPass: %.2f ms", shadowPassTime );

			ImGui::Text( "SceneRenderer::GeometryPass: %.2f ms", m_RendererData.GeometryPassTimer.ElapsedMilliseconds() );

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

		if( TreeNode( "Debug", false ) )
		{
			if( TreeNode( "Static mesh shader", false ) )
			{
				ImGui::Checkbox( "Enable Debug settings", &m_RendererData.st_EnableDebugSettings );

				if( m_RendererData.st_EnableDebugSettings )
				{
					ImGui::Checkbox( "Only Normal", &m_RendererData.st_OnlyNormal );
					ImGui::Checkbox( "Only Albedo", &m_RendererData.st_OnlyAlbedo );
					ImGui::Checkbox( "Enable PBR", &m_RendererData.st_EnablePBR );
					ImGui::Checkbox( "Enable IBL", &m_RendererData.st_EnableIBL );
				}

				EndTreeNode();
			}

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

			if( TreeNode( "SSAO", true ) )
			{
				auto framebuffer = m_RendererData.SSAOFramebuffer->GetColorAttachmentsResources()[ 0 ];

				float size = ImGui::GetContentRegionAvail().x;

				Image( framebuffer, { size, size }, { 0, 1 }, { 1, 0 } );

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

		InitAO();
		InitAOComposite();

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

			struct Light
			{
				glm::vec3 Direction;
				float Padding = 0.0f;
				glm::vec3 Radiance;
				float Multiplier;
			};
			
			// We don't need a Point light struct, Scene already has one. However we need a struct to match the one in the shader.
			struct
			{
				uint32_t nbLights;
				PointLight Lights[ 1024 ];
			} u_Lights;

			u_Lights.nbLights = m_pScene->m_Lights.PointLights.size();

			// We need a better way...
			for( uint32_t i = 0; i < u_Lights.nbLights; i++ ) 
			{
				auto& rLight = m_pScene->m_Lights.PointLights[ i ];

				u_Lights.Lights[ i ] = { rLight.Position, rLight.Radiance, rLight.Multiplier, rLight.LightSize, rLight.Radius, rLight.MinRadius, rLight.Falloff };
			}

			struct SceneData
			{
				Light Lights;
				glm::vec3 CameraPosition;
			} u_SceneData = {};
			
			struct ShadowData
			{
				glm::vec4 CascadeSplits;
			} u_ShadowData = {};

			struct DebugData
			{
				float EnableDebugSettings;
				float OnlyNormal;
				float OnlyAlbedo;
				float EnableIBL;
				float EnablePBR;

				int TilesCountX;
			} u_DebugData = {};

			u_DebugData.EnableDebugSettings = ( float ) m_RendererData.st_EnableDebugSettings;
			u_DebugData.OnlyAlbedo = ( float ) m_RendererData.st_OnlyAlbedo;
			u_DebugData.OnlyNormal = ( float ) m_RendererData.st_OnlyNormal;
			u_DebugData.EnableIBL = ( float ) m_RendererData.st_EnableIBL;
			u_DebugData.EnablePBR = ( float ) m_RendererData.st_EnablePBR;
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

			StaticMeshShader->UploadUB( ShaderType::Fragment, 0, 13, &u_Lights, sizeof( u_Lights ) );

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

	void SceneRenderer::AOPass()
	{
		VkExtent2D Extent = { m_RendererData.Width, m_RendererData.Height };
		VkCommandBuffer CommandBuffer = m_RendererData.CommandBuffer;

		m_RendererData.SSAORenderPass->BeginPass( CommandBuffer, m_RendererData.SSAOFramebuffer->GetVulkanFramebuffer(), Extent );

		m_RendererData.SSAOPassTimer.Reset();

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

		struct Matrices
		{
			glm::mat4 ViewProjection;
			glm::mat4 VP;
		} u_Matrices;

		u_Matrices.ViewProjection = glm::inverse( m_RendererData.EditorCamera.ProjectionMatrix() * m_RendererData.EditorCamera.ViewMatrix() );

		u_Matrices.VP = m_RendererData.EditorCamera.ProjectionMatrix() * m_RendererData.EditorCamera.ViewMatrix();

		struct SSAOData
		{
			glm::vec4 Samples[ 32 ];
			float SSAORadius;
		} u_Data;

		m_RendererData.SSAOShader->WriteDescriptor( "u_ViewNormalTexture", m_RendererData.GeometryFramebuffer->GetColorAttachmentsResources()[ 1 ]->GetDescriptorInfo(), m_RendererData.SSAO_DescriptorSet->GetVulkanSet() );

		m_RendererData.SSAOShader->WriteDescriptor( "u_DepthTexture", m_RendererData.GeometryFramebuffer->GetDepthAttachmentsResource()->GetDescriptorInfo(), m_RendererData.SSAO_DescriptorSet->GetVulkanSet() );

		u_Data.SSAORadius = (float)m_RendererData.SSAO_RADIUS;

		for( int i = 0; i < m_RendererData.SSAO_KERNEL_SIZE; i++ )
			u_Data.Samples[ i ] = m_RendererData.SSAOKernel[ i ];

		m_RendererData.SSAOShader->UploadUB( ShaderType::Fragment, 0, 0, &u_Matrices, sizeof( u_Matrices ) );
		m_RendererData.SSAOShader->UploadUB( ShaderType::Fragment, 0, 1, &u_Data, sizeof( u_Data ) );

		m_RendererData.SSAOShader->WriteAllUBs( m_RendererData.SSAO_DescriptorSet );

		// Draw
		Renderer::Get().SubmitFullscreenQuad(
			m_RendererData.CommandBuffer, m_RendererData.SSAOPipeline, m_RendererData.SSAO_DescriptorSet, m_RendererData.SSAO_IndexBuffer, m_RendererData.SSAO_VertexBuffer );

		m_RendererData.SSAORenderPass->EndPass();

		m_RendererData.SSAOPassTimer.Stop();
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
		Renderer::Get().SubmitFullscreenQuad(
			m_RendererData.CommandBuffer, m_RendererData.AOCompositePipeline, m_RendererData.AO_DescriptorSet, m_RendererData.SSAO_IndexBuffer, m_RendererData.SSAO_VertexBuffer );

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
		constexpr uint32_t TILE_SIZE = 16;
		glm::uvec2 Viewport = { m_RendererData.Width, m_RendererData.Height };
		glm::uvec2 Size = Viewport;
		Size += TILE_SIZE - Viewport % TILE_SIZE;
		
		m_RendererData.LightCullingWorkGroups = { Size / TILE_SIZE, 1 };

		Ref<Shader> lightCullingShader = ShaderLibrary::Get().Find( "LightCulling" );

		m_RendererData.StorageBufferSet->Resize( 0, 14, m_RendererData.LightCullingWorkGroups.x * m_RendererData.LightCullingWorkGroups.y * 4 * 1024 );

		// UBs
		UBLights u_Lights;

		struct 
		{
			glm::vec2 FullResolution;
		} u_ScreenData;

		struct 
		{
			glm::mat4 ViewProjection;
			glm::mat4 View;
		} u_Matrices;

		struct
		{
			glm::vec2 DepthUnpack;
		} u_Camera;

		u_Matrices.ViewProjection   = m_RendererData.EditorCamera.ViewProjection();
		u_Matrices.View             = m_RendererData.EditorCamera.ViewMatrix();
		u_ScreenData.FullResolution = { m_RendererData.Width, m_RendererData.Height };

		auto projection = m_RendererData.EditorCamera.ProjectionMatrix();

		float depthLinearizeMul = ( -projection[ 3 ][ 2 ] );
		float depthLinearizeAdd = ( projection[ 2 ][ 2 ] );

		if( depthLinearizeMul * depthLinearizeAdd < 0 )
			depthLinearizeAdd = -depthLinearizeAdd;

		u_Camera.DepthUnpack = { depthLinearizeAdd, depthLinearizeMul };

		u_Lights.nbLights = m_pScene->m_Lights.PointLights.size();

		for( uint32_t i = 0; i < u_Lights.nbLights; i++ )
			u_Lights.Lights[ i ] = m_pScene->m_Lights.PointLights[ i ];

		lightCullingShader->UploadUB( ShaderType::Compute, 0, 0, &u_Lights, sizeof( u_Lights ) );
		lightCullingShader->UploadUB( ShaderType::Compute, 0, 3, &u_ScreenData, sizeof( u_ScreenData ) );
		lightCullingShader->UploadUB( ShaderType::Compute, 0, 4, &u_Matrices, sizeof( u_Matrices ) );
		lightCullingShader->UploadUB( ShaderType::Compute, 0, 5, &u_Camera, sizeof( u_Camera ) );

		lightCullingShader->WriteAllUBs( m_RendererData.LightCullingDescriptorSet );

		// Write sb
		auto& rSB = m_RendererData.StorageBufferSet->Get( 0, 14 );

		VkDescriptorBufferInfo Info = { .buffer = rSB.Buffer, .offset = 0, .range = rSB.Size };

		lightCullingShader->WriteSB( 0, 14, Info, m_RendererData.LightCullingDescriptorSet );

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
		
		//for( auto&& func : m_ScheduledFunctions )
		//	func();

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
		
		/*
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "AO" );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Screen-Space AO" );

		AOPass();

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Screen-Space AO Blur" );

		CmdEndDebugLabel( m_RendererData.CommandBuffer );

		CmdEndDebugLabel( m_RendererData.CommandBuffer );
		*/

		//AOCompositePass();

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Scene Composite - Texture pass" );

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

		// Pipelines
		SceneCompositePipeline = nullptr;

		for( int i = 0; i < SHADOW_CASCADE_COUNT; i++ )
			DirShadowMapPipelines[ i ] = nullptr;

		StaticMeshPipeline = nullptr;
		GridPipeline = nullptr;
		SkyboxPipeline = nullptr;

		// Shaders
		GridShader = nullptr;
		SkyboxShader = nullptr;
		StaticMeshShader = nullptr;
		SceneCompositeShader = nullptr;
		DirShadowMapShader = nullptr;
		PreethamShader = nullptr;

		// Textures
		BRDFLUT_Texture = nullptr;
		
		SceneEnvironment = nullptr;

		ShaderLibrary::Get().Shutdown();
	}

}