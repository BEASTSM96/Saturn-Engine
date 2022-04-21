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

#include <glm/gtx/matrix_decompose.hpp>

namespace Saturn {

	SceneRenderer::~SceneRenderer()
	{
		m_DrawList.clear();
	}

	void SceneRenderer::AddDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		m_DrawList.push_back( { entity, mesh, transform } );
	}

	void SceneRenderer::RenderDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform )
	{
		auto& uuid = entity.GetComponent<IdComponent>().ID;

		// Render a draw command.

		// Bind vertex and index buffers.
		mesh->GetVertexBuffer()->Bind( m_RendererData.CommandBuffer );
		mesh->GetIndexBuffer()->Bind( m_RendererData.CommandBuffer );

		mesh->GetMaterial()->Bind( nullptr );

		// Bind the descriptor sets.
		vkCmdBindDescriptorSets( m_RendererData.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanContext::Get().GetPipeline().GetPipelineLayout(), 0, 1, &VulkanContext::Get().GetDescriptorSets()[ uuid ], 0, nullptr );

		// No push constants for now.

		// Update uniform buffers.
		VulkanContext::Get().UpdateUniformBuffers( entity.GetComponent<IdComponent>().ID, Application::Get().Time(), entity.GetComponent<TransformComponent>().GetTransform() );

		// Draw.
		mesh->GetIndexBuffer()->Draw( m_RendererData.CommandBuffer );
	}

	void SceneRenderer::FlushDrawList()
	{
		m_DrawList.clear();
	}

	void SceneRenderer::RenderScene()
	{
		if( m_DrawList.size() )
		{
			VulkanContext::Get().CreateDescriptorPool();
			//VulkanContext::Get().CreateDescriptorSets();
		}
		
		for ( auto& Cmd : m_DrawList )
		{
			RenderDrawCommand( Cmd.entity, Cmd.Mesh, Cmd.Transform );	
		}

		FlushDrawList();
	}

}