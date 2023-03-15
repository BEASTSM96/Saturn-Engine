/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "Pipeline.h"

#include "Pass.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////
	
	static VkCullModeFlagBits CullModeToVulkan( CullMode mode ) 
	{
		switch( mode )
		{
			case Saturn::CullMode::None:
				return VK_CULL_MODE_NONE;
			case Saturn::CullMode::Front:
				return VK_CULL_MODE_FRONT_BIT;
			case Saturn::CullMode::Back:
				return VK_CULL_MODE_BACK_BIT;
			case Saturn::CullMode::FrontAndBack:
				return VK_CULL_MODE_FRONT_AND_BACK;
			default:
				return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
		}

		return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}

	//////////////////////////////////////////////////////////////////////////

	Pipeline::Pipeline( const PipelineSpecification& Spec )
		: m_Specification( Spec )
	{

		Create();
	}

	void Pipeline::Terminate()
	{
		if( m_PipelineLayout )
			vkDestroyPipelineLayout( VulkanContext::Get().GetDevice(), m_PipelineLayout, nullptr );

		if( m_Pipeline )
			vkDestroyPipeline( VulkanContext::Get().GetDevice(), m_Pipeline, nullptr );

		m_Pipeline = nullptr;
		m_PipelineLayout = nullptr;

		for ( auto& [ stage, Map ] : m_DescriptorSets )
		{
			for ( auto&& [ Index, set ] : Map )
			{
				set = nullptr;
			}
		}

		m_DescriptorSets.clear();
	}

	void Pipeline::Bind( VkCommandBuffer CommandBuffer )
	{
		vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline );
	}

	void Pipeline::Create()
	{
		// Create the layout.

		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		if( m_Specification.Shader->GetPushConstantRanges().size() > 0 )
		{
			PipelineLayoutCreateInfo.pushConstantRangeCount = m_Specification.Shader->GetPushConstantRanges().size();
			PipelineLayoutCreateInfo.pPushConstantRanges = m_Specification.Shader->GetPushConstantRanges().data();
		}

		PipelineLayoutCreateInfo.setLayoutCount = m_Specification.Shader->GetSetLayouts().size();
		PipelineLayoutCreateInfo.pSetLayouts = m_Specification.Shader->GetSetLayouts().data();
		
		VK_CHECK( vkCreatePipelineLayout( VulkanContext::Get().GetDevice(), &PipelineLayoutCreateInfo, nullptr, &m_PipelineLayout ) );

		// Create shader modules

		VkShaderModule VertexModule = VK_NULL_HANDLE;
		VkShaderModule FragmentModule = VK_NULL_HANDLE;
		
		std::string VertexName = m_Specification.Shader->GetName() + "/Vertex" + "/0";
		std::string FragmentName = m_Specification.Shader->GetName() + "/Fragment" + "/0";
		
		// Shader object spirv code.
		auto& SpvSrc = ShaderLibrary::Get().Find( m_Specification.Shader->GetName() )->GetSpvCode();

		std::vector<uint32_t> VertexCode = SpvSrc.at( { ShaderType::Vertex, 0 } );
		
		{
			VkShaderModuleCreateInfo CreateInfo ={ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			CreateInfo.codeSize = 4 * VertexCode.size();
			CreateInfo.pCode = ( uint32_t* ) VertexCode.data();

			VK_CHECK( vkCreateShaderModule( VulkanContext::Get().GetDevice(), &CreateInfo, nullptr, &VertexModule ) );
		}

		if( SpvSrc.size() > 1 )
		{
			std::vector<uint32_t> FragmentCode = SpvSrc.at( { ShaderType::Fragment, 0 } );
			{
				VkShaderModuleCreateInfo FCreateInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
				FCreateInfo.codeSize = 4 * FragmentCode.size();
				FCreateInfo.pCode = ( uint32_t* ) FragmentCode.data();

				VK_CHECK( vkCreateShaderModule( VulkanContext::Get().GetDevice(), &FCreateInfo, nullptr, &FragmentModule ) );
			}

			SetDebugUtilsObjectName( std::string( m_Specification.Name + "/" + FragmentName ), ( uint64_t )FragmentModule, VK_OBJECT_TYPE_SHADER_MODULE );
		}

		SetDebugUtilsObjectName( std::string( m_Specification.Name + "/" + VertexName ), ( uint64_t )VertexModule, VK_OBJECT_TYPE_SHADER_MODULE );
		

		std::vector< VkPipelineShaderStageCreateInfo > ShaderStages;
		
		ShaderStages.push_back(
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = VertexModule,
				.pName = "main"
			} );


		if( FragmentModule != nullptr )
		{
			ShaderStages.push_back(
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.module = FragmentModule,
					.pName = "main"
				} );
		}
		
		switch( m_Specification.SpecializationStage )
		{
			case ShaderType::Vertex:
			{
				if( m_Specification.UseSpecializationInfo ) 
				{
					ShaderStages[ 0 ].pSpecializationInfo = &m_Specification.SpecializationInfo;
				}
			} break;

			case ShaderType::Fragment:
			{
				if( m_Specification.UseSpecializationInfo )
				{
					ShaderStages[ 1 ].pSpecializationInfo = &m_Specification.SpecializationInfo;
				}
			} break;

			case ShaderType::All:
			{
				if( m_Specification.UseSpecializationInfo )
				{
					ShaderStages[ 0 ].pSpecializationInfo = &m_Specification.SpecializationInfo;
					ShaderStages[ 1 ].pSpecializationInfo = &m_Specification.SpecializationInfo;
				}
			} break;
		}

		///// Vertex input state.

		std::vector< VkVertexInputAttributeDescription > VertexInputAttributes( m_Specification.VertexLayout.Count() );
		
		uint32_t i = 0;
		
		for ( auto element : m_Specification.VertexLayout )
		{
			VertexInputAttributes[ i ].binding = 0;
			VertexInputAttributes[ i ].location = i;
			VertexInputAttributes[ i ].format = ShaderDataTypeToVulkan( element.Type );
			VertexInputAttributes[ i ].offset = element.Offset;
			
			i++;
		}
		
		VkVertexInputBindingDescription VertexInputBinding = {};
		VertexInputBinding.binding = 0;
		VertexInputBinding.stride = m_Specification.VertexLayout.GetStride();
		VertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkPipelineVertexInputStateCreateInfo VertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VertexInputState.vertexBindingDescriptionCount = 1;
		VertexInputState.pVertexBindingDescriptions = &VertexInputBinding;
		VertexInputState.vertexAttributeDescriptionCount = VertexInputAttributes.size();
		VertexInputState.pVertexAttributeDescriptions = VertexInputAttributes.data();
		
		///// Descriptor sets
		
		if( m_Specification.RequestDescriptorSets.SetIndex == -1 && m_Specification.RequestDescriptorSets.Stage == ShaderType::None )
		{
			// No descriptor set requested.
		}
		else
		{
			auto CurrentStage = m_Specification.RequestDescriptorSets.Stage;
			auto Index = m_Specification.RequestDescriptorSets.SetIndex;

			if( Index == -1 && CurrentStage == ShaderType::All )
			{
				SAT_CORE_ERROR( "You requested a descriptor set at index {0}, and at shader stage {1}, but index is -1!\n In order to use -1 as a binding point you must have your shader type is 'All'!", Index, (int) CurrentStage );

				SAT_CORE_ASSERT( false );
			}
		
			switch( CurrentStage )
			{
				case ShaderType::All:
				{
					DescriptorSetSpecification SetSpec = {};
					SetSpec.Layout = m_Specification.Shader->GetSetLayout();
					SetSpec.Pool = m_Specification.Shader->GetDescriptorPool();

					if( Index == -1 )
					{
						for( int i = 0; i < m_Specification.Shader->GetDescriptorSetCount(); i++ )
						{
							m_DescriptorSets[ CurrentStage ][ Index ] = Ref<DescriptorSet>::Create( SetSpec );
						}
					} 
					else
					{
						m_DescriptorSets[ CurrentStage ][ Index ] = Ref<DescriptorSet>::Create( SetSpec );
					}
				} break;

				case ShaderType::Vertex:
				case ShaderType::Fragment:
				{
					DescriptorSetSpecification SetSpec = {};
					SetSpec.Layout = m_Specification.Shader->GetSetLayout();
					SetSpec.Pool = m_Specification.Shader->GetDescriptorPool();

					m_DescriptorSets[ CurrentStage ][ Index ] =  Ref<DescriptorSet>::Create( SetSpec );
				} break;
			}
		}

		/////

		// Create the color blend attachment state.
		VkPipelineColorBlendStateCreateInfo ColorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		
		std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachmentStates;
		
		for( size_t i = 0; i < m_Specification.RenderPass->GetColorAttachmetSize(); i++ )
		{
			VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = {};
			ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			ColorBlendAttachmentState.blendEnable = VK_TRUE;
			ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

			ColorBlendAttachmentStates.push_back( ColorBlendAttachmentState );
		}

		if( m_Specification.HasColorAttachment )
		{
			ColorBlendState.attachmentCount = ColorBlendAttachmentStates.size();
			ColorBlendState.pAttachments = ColorBlendAttachmentStates.data();
		}
		
		// Create the rasterization state.
		VkPipelineRasterizationStateCreateInfo RasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		RasterizationState.polygonMode = m_Specification.PolygonMode;
		RasterizationState.cullMode = CullModeToVulkan( m_Specification.CullMode );
		RasterizationState.frontFace = m_Specification.FrontFace;
		RasterizationState.lineWidth = 2.0f;
		RasterizationState.depthBiasEnable = VK_TRUE;
		RasterizationState.depthClampEnable = VK_FALSE;
		RasterizationState.rasterizerDiscardEnable = VK_FALSE;
		RasterizationState.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo PipelineMultisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		PipelineMultisampleState.sampleShadingEnable = VK_TRUE;
		PipelineMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo DepthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		DepthStencilState.depthTestEnable = m_Specification.UseDepthTest ? VK_TRUE : VK_FALSE;
		DepthStencilState.depthWriteEnable = VK_TRUE;
		DepthStencilState.depthCompareOp = m_Specification.DepthCompareOp;
		DepthStencilState.depthBoundsTestEnable = VK_FALSE;
		DepthStencilState.stencilTestEnable = m_Specification.UseStencilTest;
		DepthStencilState.front.compareOp = VK_COMPARE_OP_NEVER;
		DepthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineInputAssemblyStateCreateInfo AssemblyStateCreateInfo = {  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		AssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		AssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
		
		VkRect2D Scissor = {};
		
		VkViewport Viewport = {};
		Viewport.x = 0.0f;
		Viewport.y = 0.0f;
		Viewport.width = ( float )m_Specification.Width;
		Viewport.height = ( float )m_Specification.Height;
		Viewport.minDepth = 0.0f;
		Viewport.maxDepth = 1.0f;
		
		std::vector< VkDynamicState > DynamicStates;
		DynamicStates.push_back( VK_DYNAMIC_STATE_VIEWPORT );
		DynamicStates.push_back( VK_DYNAMIC_STATE_SCISSOR );
		
		VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		PipelineDynamicStateCreateInfo.pDynamicStates = DynamicStates.data();
		PipelineDynamicStateCreateInfo.dynamicStateCount = DynamicStates.size();
		
		VkPipelineViewportStateCreateInfo PipelineViewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		PipelineViewportState.pViewports = &Viewport;
		PipelineViewportState.viewportCount = 1;
		PipelineViewportState.pScissors = &Scissor;
		PipelineViewportState.scissorCount = 1;
		
		// Create the pipeline.
		VkGraphicsPipelineCreateInfo PipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		PipelineCreateInfo.layout              = m_PipelineLayout;
		PipelineCreateInfo.renderPass          = m_Specification.RenderPass->GetVulkanPass();
		PipelineCreateInfo.pVertexInputState   = &VertexInputState;
		PipelineCreateInfo.pInputAssemblyState = &AssemblyStateCreateInfo;
		PipelineCreateInfo.pRasterizationState = &RasterizationState;
		PipelineCreateInfo.pColorBlendState    = m_Specification.HasColorAttachment ? &ColorBlendState : nullptr;
		PipelineCreateInfo.pMultisampleState   = &PipelineMultisampleState;
		PipelineCreateInfo.pViewportState      = &PipelineViewportState;
		PipelineCreateInfo.pDepthStencilState  = &DepthStencilState;
		PipelineCreateInfo.pDynamicState       = &PipelineDynamicStateCreateInfo;
		PipelineCreateInfo.pStages             = ShaderStages.data();
		PipelineCreateInfo.stageCount          = ShaderStages.size();
		
		VK_CHECK( vkCreateGraphicsPipelines( VulkanContext::Get().GetDevice(), 0, 1, &PipelineCreateInfo, nullptr, &m_Pipeline ) );

		SetDebugUtilsObjectName( m_Specification.Name, ( uint64_t )m_Pipeline, VK_OBJECT_TYPE_PIPELINE );

		if( m_Specification.Name != "" )
			SAT_CORE_WARN( "Created pipeline: {0}!", m_Specification.Name );
		else				
			SAT_CORE_WARN( "Created pipeline: {0}!", ( uint64_t )m_Pipeline );

		vkDestroyShaderModule( VulkanContext::Get().GetDevice(), VertexModule, nullptr );
		vkDestroyShaderModule( VulkanContext::Get().GetDevice(), FragmentModule, nullptr );
	}
}
