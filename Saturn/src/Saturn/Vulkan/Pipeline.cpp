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
#include "Pipeline.h"

#include "VulkanContext.h"
#include "VulkanDebug.h"

namespace Saturn {

	void PipelineLayout::Create()
	{
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		PipelineLayoutCreateInfo.pushConstantRangeCount = PushConstants.PushConstantRanges.size();
		PipelineLayoutCreateInfo.pPushConstantRanges = PushConstants.PushConstantRanges.data();
		PipelineLayoutCreateInfo.setLayoutCount = SetLayouts.SetLayouts.size();
		PipelineLayoutCreateInfo.pSetLayouts = SetLayouts.SetLayouts.data();

		VK_CHECK( vkCreatePipelineLayout( VulkanContext::Get().GetDevice(), &PipelineLayoutCreateInfo, nullptr, &Layout ) );
	}

	void PipelineLayout::Terminate()
	{
		if( Layout )
			vkDestroyPipelineLayout( VulkanContext::Get().GetDevice(), Layout, nullptr );

		Layout = nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	
	void PipelineSpecification::Terminate()
	{
		Layout.Terminate();
	}

	//////////////////////////////////////////////////////////////////////////

	Pipeline::Pipeline( PipelineSpecification Spec )
	{
		m_Specification = Spec;

		Create();
	}

	void Pipeline::Bind( VkCommandBuffer CommandBuffer )
	{
		vkCmdBindPipeline( CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline );
	}

	void Pipeline::Create()
	{
		// Create the layout.

		m_Specification.Layout.Create();

		// Create shader modules

		VkShaderModule VertexModule = VK_NULL_HANDLE;
		VkShaderModule FragmentModule = VK_NULL_HANDLE;
		
		std::string VertexName = m_Specification.pShader->GetName() + "/Vertex" + "/0";
		std::string FragmentName = m_Specification.pShader->GetName() + "/Fragment" + "/0";
		
		// Shader object spirv code.
		auto& SpvSrc = ShaderLibrary::Get().Find( m_Specification.pShader->GetName() )->GetSpvCode();

		std::vector<uint32_t> VertexCode = SpvSrc.at( { ShaderType::Vertex, 0 } );
		std::vector<uint32_t> FragmentCode = SpvSrc.at( { ShaderType::Fragment, 0 } );
		
		//{
			VkShaderModuleCreateInfo CreateInfo ={ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			CreateInfo.codeSize = 4 * VertexCode.size();
			CreateInfo.pCode = ( uint32_t* ) VertexCode.data();

			VK_CHECK( vkCreateShaderModule( VulkanContext::Get().GetDevice(), &CreateInfo, nullptr, &VertexModule ) );
		//}

		//{
			VkShaderModuleCreateInfo FCreateInfo ={ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			FCreateInfo.codeSize = 4 * FragmentCode.size();
			FCreateInfo.pCode = ( uint32_t* ) FragmentCode.data();

			VK_CHECK( vkCreateShaderModule( VulkanContext::Get().GetDevice(), &FCreateInfo, nullptr, &FragmentModule ) );
		//}
		
		SetDebugUtilsObjectName( std::string( m_Specification.Name + "/" + VertexName ), ( uint64_t )VertexModule, VK_OBJECT_TYPE_SHADER_MODULE );
		
		SetDebugUtilsObjectName( std::string( m_Specification.Name + "/" + FragmentName ), ( uint64_t )FragmentModule, VK_OBJECT_TYPE_SHADER_MODULE );

		std::vector< VkPipelineShaderStageCreateInfo > ShaderStages;
		
		ShaderStages.push_back( 
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = VertexModule,
				.pName = "main"
			} );

		ShaderStages.push_back(
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = FragmentModule,
				.pName = "main"
			} );
		
		VkPipelineVertexInputStateCreateInfo VertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		VertexInputState.vertexBindingDescriptionCount = m_Specification.BindingDescriptions.size();
		VertexInputState.pVertexBindingDescriptions = m_Specification.BindingDescriptions.data();
		VertexInputState.vertexAttributeDescriptionCount = m_Specification.AttributeDescriptions.size();
		VertexInputState.pVertexAttributeDescriptions = m_Specification.AttributeDescriptions.data();
		
		// Create the color blend attachment state.
		VkPipelineColorBlendAttachmentState ColorBlendAttachmentState = {};
		ColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachmentState.blendEnable = VK_TRUE;
		ColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		ColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		ColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		ColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo ColorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		ColorBlendState.attachmentCount = 1;
		ColorBlendState.pAttachments = &ColorBlendAttachmentState;
		
		// Create the rasterization state.
		VkPipelineRasterizationStateCreateInfo RasterizationState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizationState.cullMode = m_Specification.CullMode;
		RasterizationState.frontFace = m_Specification.FrontFace;
		RasterizationState.lineWidth = 2.0f;

		VkPipelineMultisampleStateCreateInfo PipelineMultisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		PipelineMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		PipelineMultisampleState.sampleShadingEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo DepthStencilState = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		DepthStencilState.depthTestEnable = m_Specification.UseDepthTest ? VK_TRUE : VK_FALSE;
		DepthStencilState.depthWriteEnable = VK_TRUE;
		DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		DepthStencilState.depthBoundsTestEnable = VK_FALSE;
		DepthStencilState.stencilTestEnable = VK_FALSE;
		
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
		PipelineCreateInfo.layout              = m_Specification.Layout;
		PipelineCreateInfo.renderPass          = m_Specification.RenderPass;
		PipelineCreateInfo.pVertexInputState   = &VertexInputState;
		PipelineCreateInfo.pInputAssemblyState = &AssemblyStateCreateInfo;
		PipelineCreateInfo.pRasterizationState = &RasterizationState;
		PipelineCreateInfo.pColorBlendState    = &ColorBlendState;
		PipelineCreateInfo.pMultisampleState   = &PipelineMultisampleState;
		PipelineCreateInfo.pViewportState      = &PipelineViewportState;
		PipelineCreateInfo.pDepthStencilState  = &DepthStencilState;
		PipelineCreateInfo.pDynamicState       = &PipelineDynamicStateCreateInfo;
		PipelineCreateInfo.pStages             = ShaderStages.data();
		PipelineCreateInfo.stageCount          = ShaderStages.size();
		
		VK_CHECK( vkCreateGraphicsPipelines( VulkanContext::Get().GetDevice(), 0, 1, &PipelineCreateInfo, nullptr, &m_Pipeline ) );

		SetDebugUtilsObjectName( m_Specification.Name, ( uint64_t )m_Pipeline, VK_OBJECT_TYPE_PIPELINE );

		vkDestroyShaderModule( VulkanContext::Get().GetDevice(), VertexModule, nullptr );
		vkDestroyShaderModule( VulkanContext::Get().GetDevice(), FragmentModule, nullptr );
	}

	void Pipeline::Terminate()
	{
		m_Specification.Terminate();

		if( m_Pipeline )
			vkDestroyPipeline( VulkanContext::Get().GetDevice(), m_Pipeline, nullptr );

		m_Pipeline = nullptr;
	}
}
