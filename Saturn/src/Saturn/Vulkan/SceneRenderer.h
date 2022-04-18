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

#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Scene.h"
#include "Saturn/Scene/Entity.h"
#include "Mesh.h"

namespace Saturn {

	struct DrawCommand
	{
		DrawCommand( Entity e, Ref< Mesh > mesh, glm::mat4 trans ) : entity( e ), Mesh( mesh ), Transform( trans )
		{
		}

		Entity entity;
		Ref< Mesh > Mesh = nullptr;
		glm::mat4 Transform;
	};

	struct RendererData
	{
		VkCommandBuffer CommandBuffer;
		uint32_t FrameCount;
	};

	class SceneRenderer
	{
		SINGLETON( SceneRenderer );
	public:
		 SceneRenderer() {}
		~SceneRenderer();
		
		void SetRendererData( RendererData Data ) { m_RendererData = Data; }

		void SetCurrentScene( Scene* pScene ) { m_pSence = pScene; }

		void AddDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );
		
		void RenderDrawCommand( Entity entity, Ref< Mesh > mesh, const glm::mat4 transform );
		
		void FlushDrawList();

		void RenderScene();
		
		std::vector< DrawCommand >& GetDrawCmds() { return m_DrawList; }

	private:
		RendererData m_RendererData;
		std::vector< DrawCommand > m_DrawList;
		Scene* m_pSence;
	};
}