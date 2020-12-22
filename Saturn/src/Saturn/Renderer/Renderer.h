/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 BEAST                                                                  *
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

#include "RenderCommandQueue.h"
#include "RenderPass.h"
#include "Saturn/Core/AABB/AABB.h"
#include "Mesh.h"

namespace Saturn {

	class ShaderLibrary;

	class SATURN_API Renderer
	{
	public:
		typedef void( *RenderCommandFn )( void* );

		/*Commands*/

		static void Clear( void );
		static void Clear( float r, float g, float b, float a = 1.0f );
		static void SetClearColor( float r, float g, float b, float a );
		static void DrawIndexed( uint32_t count, PrimitiveType type, bool depthTest = true );

		/*OpenGL*/
		static void SetLineThickness( float thickness );
		static void ClearMagenta( void );
		static void Init( void );

		static Ref<ShaderLibrary> GetShaderLibrary( void );

		template<typename FuncT>
		static void Submit( FuncT&& func )
		{
			auto renderCmd = []( void* ptr )
			{
				auto pFunc = ( FuncT* )ptr;
				( *pFunc )( );
				pFunc->~FuncT();
			};
			auto storageBuffer = GetRenderCommandQueue().Allocate( renderCmd, sizeof( func ) );
			new ( storageBuffer ) FuncT( std::forward<FuncT>( func ) );
		}

		static void WaitAndRender( void );

		/*Render stuff*/
		static void BeginRenderPass( Ref<RenderPass> renderPass, bool clear = true );
		static void EndRenderPass( void );

		static void SubmitQuad( Ref<MaterialInstance> material, const glm::mat4& transform = glm::mat4( 1.0f ) );
		static void SubmitFullscreenQuad( Ref<MaterialInstance> material );
		static void SubmitMesh( Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial );

		static void DrawAABB( const AABB& aabb, const glm::mat4& transform, const glm::vec4& color = glm::vec4( 1.0f ) );
		static void DrawAABB( Ref<Mesh> mesh, const glm::mat4& transform, const glm::vec4& color = glm::vec4( 1.0f ) );

		static RendererAPIType GetAPI() { return RendererAPI::Current(); }
		static void OnWindowResize( uint32_t width, uint32_t height );
	private:
		static RenderCommandQueue& GetRenderCommandQueue( void );
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}