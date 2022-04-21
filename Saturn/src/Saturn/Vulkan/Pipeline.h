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

#pragma once

#include "Shader.h"
#include <vulkan.h>

namespace Saturn {

	struct PipelineSetLayout
	{
		std::vector< VkDescriptorSetLayout > SetLayouts;
	};
	
	struct PushConstantPipelineData
	{
		std::vector< VkPushConstantRange > PushConstantRanges;
	};
	
	struct PipelineLayout
	{
		void Create();
		void Terminate();

		operator VkPipelineLayout() const { return Layout; }
		operator VkPipelineLayout&()      { return Layout; }

		VkPipelineLayout Layout = VK_NULL_HANDLE;
		PushConstantPipelineData PushConstants = {};
		PipelineSetLayout SetLayouts = {};
	};

	struct PipelineSpecification
	{
		void Terminate();

		Shader* pShader = nullptr;
		VkRenderPass RenderPass = VK_NULL_HANDLE;

		PipelineLayout Layout = {};
		
		uint32_t Width = 0, Height = 0;
		
		bool UseDepthTest = false;

		std::string Name = "Pipeline";
	};

	class Pipeline
	{
	public:
		Pipeline() { }
		Pipeline( PipelineSpecification Spec );
		~Pipeline() {}
		
		VkPipeline& GetPipeline() { return m_Pipeline; }
		VkPipelineLayout& GetPipelineLayout() { return m_Specification.Layout.Layout; }
		
		operator VkPipeline() const { return m_Pipeline; }
		operator VkPipeline&()      { return m_Pipeline; }

		void Terminate();

	private:

		void Create();

		PipelineSpecification m_Specification = {};
		
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
	};
}