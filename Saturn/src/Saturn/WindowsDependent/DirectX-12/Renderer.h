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

#include "Saturn/Core/Base.h"
#include "Common.h"

namespace Saturn {

	class Renderer
	{
		SINGLETON( Renderer );

		Renderer() { Init(); }
		~Renderer() { Close(); }

	public:

		void Clear();
		void Resize( int width, int height );

		void PopulateCommandList();
		void WaitForPreviousFrame();
		void LoadDXPipeline();

		void Render();

	public:
	protected:
	private:


	private:
		static const UINT m_FrameCount = 2;

		CD3DX12_VIEWPORT m_Viewport;
		CD3DX12_RECT m_ScissorRect;

		ComPtr<IDXGISwapChain3>           m_SwapChain;
		ComPtr<ID3D12Device>              m_Device;
		ComPtr<ID3D12Resource>            m_RenderTargets[ m_FrameCount ];
		ComPtr<ID3D12CommandAllocator>    m_CommandAllocator;
		ComPtr<ID3D12CommandQueue>        m_CommandQueue;
		ComPtr<ID3D12RootSignature>       m_RootSignature;
		ComPtr<ID3D12DescriptorHeap>      m_RenderViewTargetHeap;
		ComPtr<ID3D12PipelineState>       m_PipelineState;
		ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ComPtr<ID3D12Fence>               m_Fence;
		// Temp
		ComPtr<ID3D12Resource>            m_VertexBuffer;

		UINT   m_RTVDescriptorSize;
		UINT   m_FrameIndex;
		UINT64 m_FenceValue;
		HANDLE m_FenceEvent;

		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

		float m_Gamma = 2.2F;
		float m_AspectRatio;
		bool m_UseWarpDevice;

		void Init();
		void Close();
	};
}