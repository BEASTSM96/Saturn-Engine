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

namespace Saturn {

	void PipelineLayout::Create()
	{
		VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
		PipelineLayoutCreateInfo.pushConstantRangeCount = PushConstants.PushConstantRanges.size();
		PipelineLayoutCreateInfo.pPushConstantRanges = PushConstants.PushConstantRanges.data();
		PipelineLayoutCreateInfo.setLayoutCount = SetLayouts.SetLayouts.size();
		PipelineLayoutCreateInfo.pSetLayouts = SetLayouts.SetLayouts.data();

		VK_CHECK( vkCreatePipelineLayout( VulkanContext::Get().GetDevice(), &PipelineLayoutCreateInfo, nullptr, &Layout ) );
	}

	//////////////////////////////////////////////////////////////////////////

	Pipeline::Pipeline( PipelineSpecification Spec )
	{
		m_Specification = Spec;

		Create();
	}

	void Pipeline::Terminate()
	{

	}

	void Pipeline::Create()
	{
		// Create the layout.

		m_Specification.Layout.Create();

		// Create shader modules

		VkShaderModule VertexModule = VK_NULL_HANDLE;
		VkShaderModule FragmentModule = VK_NULL_HANDLE;

		VkShaderModuleCreateInfo VertexCreateInfo ={ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		//VertexCreateInfo.codeSize = ShaderWorker::Get().GetShaderCode( std::string( m_Specification.Shader->GetName() ) );
	}
}
