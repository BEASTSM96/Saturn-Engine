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

		Ref<Shader> shader = Ref<Shader>::Create( "assets/shaders/PBR_Static.glsl" );

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

		PassSpecification PassSpec = {};
		PassSpec.Name = "Geometry Pass";
		PassSpec.Attachments = { ImageFormat::BGRA8, ImageFormat::Depth };

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
			
			FBSpec.Attachments = { ImageFormat::BGRA8, ImageFormat::Depth };

			m_RendererData.GeometryFramebuffer = Ref< Framebuffer >::Create( FBSpec );
		}

		//////////////////////////////////////////////////////////////////////////
		// STATIC MESHES
		//////////////////////////////////////////////////////////////////////////

		// Create the static meshes pipeline.
		// Load the shader
		if( !m_RendererData.StaticMeshShader ) 
		{
			m_RendererData.StaticMeshShader = Ref< Shader >::Create( "assets/shaders/shader_new.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.StaticMeshShader );
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
		
		PassSpec.Attachments = { ImageFormat::Depth };

		m_RendererData.DirShadowMapPass = Ref<Pass>::Create( PassSpec );
		
		for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			FramebufferSpecification FBSpec = {};
			FBSpec.RenderPass = m_RendererData.DirShadowMapPass;
			FBSpec.Width = SHADOW_MAP_SIZE;
			FBSpec.Height = SHADOW_MAP_SIZE;
			FBSpec.ArrayLevels = SHADOW_CASCADE_COUNT;

			FBSpec.Attachments = { ImageFormat::Depth };

			m_RendererData.ShadowCascades[ i ].Framebuffer = Ref<Framebuffer>::Create( FBSpec );
		}

		if( !m_RendererData.DirShadowMapShader )
		{
			m_RendererData.DirShadowMapShader = Ref< Shader >::Create( "assets/shaders/ShadowMap.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.DirShadowMapShader );
		}
	
		PipelineSpecification PipelineSpec = {};
		PipelineSpec.Width = SHADOW_MAP_SIZE;
		PipelineSpec.Height = SHADOW_MAP_SIZE;
		PipelineSpec.Name = "DirShadowMap";
		PipelineSpec.Shader = m_RendererData.DirShadowMapShader.Pointer();
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
		PipelineSpec.RequestDescriptorSets = { ShaderType::Vertex, 0 };

		m_RendererData.DirShadowMapPipeline = Ref< Pipeline >::Create( PipelineSpec );
	}

	void SceneRenderer::InitSceneComposite()
	{
		if( m_RendererData.SceneComposite )
			m_RendererData.SceneComposite = nullptr;

		// Create the scene composite render pass.
		PassSpecification PassSpec = {};
		PassSpec.Name = "Texture pass";
		
		PassSpec.Attachments = { ImageFormat::BGRA8, ImageFormat::Depth };
		
		m_RendererData.SceneComposite = Ref< Pass >::Create( PassSpec );
		
		if( m_RendererData.SceneCompositeFramebuffer )
			m_RendererData.SceneCompositeFramebuffer->Recreate();
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
		Renderer::Get().CreateFullscreenQuad( &m_RendererData.SC_VertexBuffer, &m_RendererData.SC_IndexBuffer );
		
		if( !m_RendererData.SceneCompositeShader )
		{
			m_RendererData.SceneCompositeShader = Ref< Shader >::Create( "assets/shaders/SceneComposite.glsl" );
			
			ShaderLibrary::Get().Add( m_RendererData.SceneCompositeShader );
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

			SkyboxMatricesObject.View = m_RendererData.EditorCamera.ViewMatrix();
			SkyboxMatricesObject.Projection = m_RendererData.EditorCamera.ProjectionMatrix();
			SkyboxMatricesObject.Turbidity = Skylight.Turbidity;
			SkyboxMatricesObject.Azimuth = Skylight.Azimuth;
			SkyboxMatricesObject.Inclination = Skylight.Inclination;

			auto Data = m_RendererData.SkyboxShader->MapUB( ShaderType::All, 0, 0 );

			memcpy( Data, &SkyboxMatricesObject, sizeof( SkyboxMatricesObject ) );

			m_RendererData.SkyboxShader->UnmapUB( ShaderType::All, 0, 0 );

			m_RendererData.SkyboxShader->WriteAllUBs( m_RendererData.SkyboxDescriptorSet );
			
			Renderer::Get().SubmitFullscreenQuad( 
				CommandBuffer, m_RendererData.SkyboxPipeline, m_RendererData.SkyboxDescriptorSet, m_RendererData.SkyboxIndexBuffer, m_RendererData.SkyboxVertexBuffer );
		}
	}

	struct FrustumBounds
	{
		float r, l, b, t, f, n;
	};

	void SceneRenderer::UpdateCascades( const glm::vec3& Direction )
	{
		FrustumBounds frustumBounds[ 3 ];

		auto viewProjection = m_RendererData.EditorCamera.ProjectionMatrix() * m_RendererData.EditorCamera.ViewMatrix();

		float cascadeSplits[ SHADOW_CASCADE_COUNT ];

		float CascadeFarPlaneOffset = 15.0f;
		float CascadeNearPlaneOffset = -15.0f;

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
		for( uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
		{
			float p = ( i + 1 ) / static_cast< float >( SHADOW_CASCADE_COUNT );
			float log = minZ * std::pow( ratio, p );
			float uniform = minZ + range * p;
			float d = m_RendererData.CascadeSplitLambda * ( log - uniform ) + uniform;
			cascadeSplits[ i ] = ( d - nearClip ) / clipRange;
		}

		// Manually set cascades here
		cascadeSplits[ 0 ] = 0.05f;
		cascadeSplits[ 1 ] = 0.15f;
		cascadeSplits[ 3 ] = 0.3f;
		cascadeSplits[ 2 ] = 0.3f;
		cascadeSplits[ 3 ] = 1.0f;

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for( uint32_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
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
			glm::mat4 lightOrthoMatrix = glm::ortho( minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + CascadeNearPlaneOffset, maxExtents.z - minExtents.z + CascadeFarPlaneOffset );

			// Offset to texel space to avoid shimmering (from https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
			glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			const float ShadowMapResolution = SHADOW_MAP_SIZE;
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
			m_RendererData.GridShader = Ref< Shader >::Create( "assets/shaders/Grid.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.GridShader );
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
		Renderer::Get().CreateFullscreenQuad( &m_RendererData.SkyboxVertexBuffer, &m_RendererData.SkyboxIndexBuffer );

		// Create skybox shader.
		
		if( !m_RendererData.SkyboxShader )
		{
			m_RendererData.SkyboxShader = Ref<Shader>::Create( "assets/shaders/Skybox.glsl" );
			ShaderLibrary::Get().Add( m_RendererData.SkyboxShader );
		}
		
		if( !m_RendererData.SkyboxDescriptorSet ) 
			m_RendererData.SkyboxDescriptorSet = m_RendererData.SkyboxShader->CreateDescriptorSet( 0 );

		m_RendererData.SkyboxShader->WriteAllUBs( m_RendererData.SkyboxDescriptorSet );
		
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
		PipelineSpec.CullMode = CullMode::None;
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

			Image( m_RendererData.ShadowCascades[0].Framebuffer->GetDepthAttachmentsResource(), ImVec2( 100, 100 ) );
			
			/*
			for( size_t i = 0; i < SHADOW_CASCADE_COUNT; i++ )
			{
			}
			*/
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

	void SceneRenderer::SetWidthAndHeight( uint32_t w, uint32_t h )
	{
		if( m_RendererData.Width != w && m_RendererData.Width != h )
		{
			m_RendererData.Width = w;
			m_RendererData.Height = h;
			m_RendererData.Resized = true;
		}
	}

	void SceneRenderer::FlushDrawList()
	{
		m_DrawList.clear();
	}

	void SceneRenderer::Recreate()
	{
		InitGeometryPass();
		InitSceneComposite();

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

		Renderer::Get().Begin( m_RendererData.ShadowCascades[ 0 ].Framebuffer->GetDepthAttachmentsResource() );

		// Render static meshes.
		Ref< Shader > StaticMeshShader = m_RendererData.StaticMeshShader;


		for( auto& Cmd : m_DrawList )
		{
			auto& uuid = Cmd.entity.GetComponent<IdComponent>().ID;

			// u_Matrices
			RendererData::StaticMeshMatrices u_Matrices = {};
			u_Matrices.ViewProjection = m_RendererData.EditorCamera.ViewProjection();

			struct
			{
				glm::mat4 LightMatrix;
			} u_LightData;

			struct Light
			{
				glm::vec3 Direction;
				float Padding = 0.0f;
				glm::vec3 Radiance;
				float Multiplier;
			};
			
			struct SceneData
			{
				Light Lights;
				glm::vec3 CameraPosition;
			} u_SceneData;

			auto dirLight = m_pScene->m_DirectionalLight[ 0 ];
			
			u_SceneData.CameraPosition = m_RendererData.EditorCamera.Position();
			u_SceneData.Lights = { .Direction = dirLight.Direction, .Radiance = dirLight.Radiance, .Multiplier = dirLight.Intensity };

			u_LightData.LightMatrix = m_RendererData.ShadowCascades[ 0 ].ViewProjection;

			auto pData = StaticMeshShader->MapUB( ShaderType::Vertex, 0, 0 );

			memcpy( pData, &u_Matrices, sizeof( u_Matrices ) );

			StaticMeshShader->UnmapUB( ShaderType::Vertex, 0, 0 );

			pData = StaticMeshShader->MapUB( ShaderType::Vertex, 0, 1 );

			memcpy( pData, &u_LightData, sizeof( u_LightData ) );

			StaticMeshShader->UnmapUB( ShaderType::Vertex, 0, 1 );

			pData = StaticMeshShader->MapUB( ShaderType::Fragment, 0, 2 );

			memcpy( pData, &u_SceneData, sizeof( u_SceneData ) );

			StaticMeshShader->UnmapUB( ShaderType::Vertex, 0, 2 );

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
		
		//////////////////////////////////////////////////////////////////////////
		
		Ref< Shader > ShadowShader = m_RendererData.DirShadowMapShader;

		UpdateCascades( m_pScene->m_DirectionalLight[ 0 ].Direction );
		
		// u_Matrices
		struct UB_Matrices
		{
			glm::mat4 ViewProjection;
		} u_Matrices;

		u_Matrices = {};

		u_Matrices.ViewProjection = m_RendererData.ShadowCascades[ 0 ].ViewProjection;

		auto pData = m_RendererData.DirShadowMapShader->MapUB( ShaderType::Vertex, 0, 0 );

		memcpy( pData, &u_Matrices, sizeof( u_Matrices ) );

		m_RendererData.DirShadowMapShader->UnmapUB( ShaderType::Vertex, 0, 0 );

		m_RendererData.DirShadowMapPipeline->GetShader()->WriteAllUBs( m_RendererData.DirShadowMapPipeline->GetDescriptorSet( ShaderType::Vertex, 0 ) );

		for( size_t i = 0; i < 1; i++ )
		{
			RenderPassBeginInfo.framebuffer = m_RendererData.ShadowCascades[ i ].Framebuffer->GetVulkanFramebuffer();

			// Begin directional shadow map pass.
			CmdBeginDebugLabel( CommandBuffer, "DirShadowMap" );
			vkCmdBeginRenderPass( CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

			vkCmdSetViewport( m_RendererData.CommandBuffer, 0, 1, &Viewport );
			vkCmdSetScissor( m_RendererData.CommandBuffer, 0, 1, &Scissor );

			for( auto& Cmd : m_DrawList )
			{
				Renderer::Get().RenderMeshWithoutMaterial( CommandBuffer, m_RendererData.DirShadowMapPipeline, Cmd.Mesh, Cmd.Transform );
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
		
		m_RendererData.SceneCompositeShader->WriteAllUBs( m_RendererData.SC_DescriptorSet );

		Renderer::Get().SubmitFullscreenQuad( 
			CommandBuffer, m_RendererData.SceneCompositePipeline, 
			m_RendererData.SC_DescriptorSet, 
			m_RendererData.SC_IndexBuffer, m_RendererData.SC_VertexBuffer );
		
		// End scene composite pass.
		m_RendererData.SceneComposite->EndPass();
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
		}

		m_RendererData.CommandBuffer = Renderer::Get().ActiveCommandBuffer();
		
		// Passes

		DirShadowMapPass();

		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Geometry" );

		GeometryPass();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );
		
		CmdBeginDebugLabel( m_RendererData.CommandBuffer, "Scene Composite - Texture pass" );

		SceneCompositePass();
		
		CmdEndDebugLabel( m_RendererData.CommandBuffer );

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