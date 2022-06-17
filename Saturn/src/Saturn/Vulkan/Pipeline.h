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
#include "VertexBuffer.h"

#include <vulkan.h>

namespace Saturn {
	
	class Pass;

	enum class CullMode 
	{
		None,
		Front,
		Back,
		FrontAndBack
	};

	struct PipelineSpecification
	{
		// TODO: Remove this.
		PipelineSpecification() {}
		~PipelineSpecification() {}
		
		PipelineSpecification(const PipelineSpecification& other)
		{
			*this = other;
		}

		PipelineSpecification& operator=( const PipelineSpecification& other )
		{
			if( this == &other )
				return *this;

			Shader = other.Shader;
			RenderPass = other.RenderPass;
			Name = other.Name;
			VertexLayout = other.VertexLayout;
			CullMode = other.CullMode;
			UseDepthTest = other.UseDepthTest;
			UseStencilTest = other.UseStencilTest;
			FrontFace = other.FrontFace;
			PolygonMode = other.PolygonMode;
			SetLayouts = other.SetLayouts;
			Width = other.Width;
			Height = other.Height;

			return *this;
		}

		//////

		Ref<Shader> Shader;
		Ref<Pass> RenderPass;
		std::string Name = "Pipeline";
		VertexBufferLayout VertexLayout;
		std::vector< VkDescriptorSetLayout > SetLayouts;
		uint32_t Width = 0, Height = 0;
		bool UseDepthTest = false;
		bool UseStencilTest = false;
		CullMode CullMode = CullMode::Back;
		VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
	};

	class Pipeline
	{
	public:
		Pipeline() { }
		Pipeline( PipelineSpecification Spec );
		~Pipeline() { Terminate(); }
		
		void Bind( VkCommandBuffer CommandBuffer );

		VkPipeline GetPipeline() { return m_Pipeline; }
		VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }
		
		operator VkPipeline() const { return m_Pipeline; }

		void Terminate();
	private:

		void Create();

		PipelineSpecification m_Specification = {};
		
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
	};
}