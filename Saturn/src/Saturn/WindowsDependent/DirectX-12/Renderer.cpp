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
#include "Renderer.h"

#include "Saturn/Core/Window.h"
#include "Shader.h"

namespace Saturn {

	void Renderer::Init() 
	{
		m_FrameIndex = 0;
		m_RTVDescriptorSize = 0;

		m_Viewport = CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast< float >( Window::Get().Width() ), static_cast< float >( Window::Get().Height() ) );
		m_ScissorRect = CD3DX12_RECT( 0, 0, static_cast< LONG >( Window::Get().Width() ), static_cast< LONG >( Window::Get().Height() ) );

		LoadDXPipeline();
	}

	void Renderer::Close()
	{
		WaitForPreviousFrame();

		CloseHandle( m_FenceEvent );
	}

	void Renderer::Clear()
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_RenderViewTargetHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize );

		const float clearColor[] ={ 0.0f, 0.0f, 0.0f, 1.0f };
		m_CommandList->ClearRenderTargetView( rtvHandle, clearColor, 0, nullptr );
	}

	void Renderer::Resize( int width, int height )
	{

	}
	
	void Renderer::PopulateCommandList()
	{
		ThrowIfFailed( m_CommandAllocator->Reset() );

		ThrowIfFailed( m_CommandList->Reset( m_CommandAllocator.Get(), m_PipelineState.Get() ) );

		m_CommandList->SetGraphicsRootSignature( m_RootSignature.Get() );

		ID3D12DescriptorHeap* ppHeaps[] ={ m_SRVHeap.Get() };
		m_CommandList->SetDescriptorHeaps( _countof( ppHeaps ), ppHeaps );

		m_CommandList->SetGraphicsRootDescriptorTable( 0, m_SRVHeap->GetGPUDescriptorHandleForHeapStart() );
		m_CommandList->RSSetViewports( 1, &m_Viewport );
		m_CommandList->RSSetScissorRects( 1, &m_ScissorRect );

		m_CommandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_RenderTargets[ m_FrameIndex ].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET ) );

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_RenderViewTargetHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize );
		m_CommandList->OMSetRenderTargets( 1, &rtvHandle, FALSE, nullptr );

		Clear();

		m_CommandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		m_CommandList->IASetVertexBuffers( 0, 1, &m_VertexBufferView );
		m_CommandList->DrawInstanced( 3, 1, 0, 0 );

		m_CommandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( m_RenderTargets[ m_FrameIndex ].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

		ThrowIfFailed( m_CommandList->Close() );
	}

	void Renderer::WaitForPreviousFrame()
	{
		const UINT64 fence = m_FenceValue;
		ThrowIfFailed( m_CommandQueue->Signal( m_Fence.Get(), fence ) );
		m_FenceValue++;

		if( m_Fence->GetCompletedValue() < fence )
		{
			ThrowIfFailed( m_Fence->SetEventOnCompletion( fence, m_FenceEvent ) );
			WaitForSingleObject( m_FenceEvent, INFINITE );
		}

		m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void Renderer::LoadDXPipeline()
	{
		UINT dxFactoryFlags = 0;

	#if defined(_DEBUG)
		{
			ComPtr<ID3D12Debug> debugController;
			if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugController ) ) ) )
			{
				debugController->EnableDebugLayer();

				// Enable additional debug layers.
				dxFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
	#endif

		// Create Factory
		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed( CreateDXGIFactory2( dxFactoryFlags, IID_PPV_ARGS( &factory ) ) );

		if( m_UseWarpDevice )
		{
			ComPtr<IDXGIAdapter> warpAdapter;
			ThrowIfFailed( factory->EnumWarpAdapter( IID_PPV_ARGS( &warpAdapter ) ) );

			ThrowIfFailed( D3D12CreateDevice(
				warpAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS( &m_Device )
			) );
		}
		else
		{
			ComPtr<IDXGIAdapter1> hardwareAdapter;
			GetHardwareAdapter( factory.Get(), &hardwareAdapter );

			ThrowIfFailed( D3D12CreateDevice(
				hardwareAdapter.Get(),
				D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS( &m_Device )
			) );
		}

		// Create the command queue
		D3D12_COMMAND_QUEUE_DESC queueDesc ={};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		ThrowIfFailed( m_Device->CreateCommandQueue( &queueDesc, IID_PPV_ARGS( &m_CommandQueue ) ) );

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc ={};
		swapChainDesc.BufferCount = m_FrameCount;
		swapChainDesc.Width = Window::Get().Width();
		swapChainDesc.Height = Window::Get().Height();
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		ComPtr<IDXGISwapChain1> swapChain;

		ThrowIfFailed( factory->CreateSwapChainForHwnd( m_CommandQueue.Get(), Window::Get().PlatformWindow(), &swapChainDesc, nullptr, nullptr, &swapChain ) );

		ThrowIfFailed( factory->MakeWindowAssociation( Window::Get().PlatformWindow(), DXGI_MWA_NO_ALT_ENTER ) );

		ThrowIfFailed( swapChain.As( &m_SwapChain ) );
		m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

		// Create descriptor heaps
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc ={};
			rtvHeapDesc.NumDescriptors = m_FrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed( m_Device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &m_RenderViewTargetHeap ) ) );

			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc ={};
			srvHeapDesc.NumDescriptors = 1;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			ThrowIfFailed( m_Device->CreateDescriptorHeap( &srvHeapDesc, IID_PPV_ARGS( &m_SRVHeap ) ) );

			m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
		}

		// Create frame resources
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( m_RenderViewTargetHeap->GetCPUDescriptorHandleForHeapStart() );

			// Create a RTV for each frame
			for( UINT n = 0; n < m_FrameCount; n++ )
			{
				ThrowIfFailed( m_SwapChain->GetBuffer( n, IID_PPV_ARGS( &m_RenderTargets[ n ] ) ) );
				m_Device->CreateRenderTargetView( m_RenderTargets[ n ].Get(), nullptr, rtvHandle );
				rtvHandle.Offset( 1, m_RTVDescriptorSize );
			}
		}

		ThrowIfFailed( m_Device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &m_CommandAllocator ) ) );


		// TEMP LOAD HELLO TRI
		{
			// Create an empty root signature.
			{
				CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
				rootSignatureDesc.Init( 0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );

				ComPtr<ID3DBlob> signature;
				ComPtr<ID3DBlob> error;
				ThrowIfFailed( D3D12SerializeRootSignature( &rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error ) );
				ThrowIfFailed( m_Device->CreateRootSignature( 0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS( &m_RootSignature ) ) );
			}

			// Create the pipeline state, which includes compiling and loading shaders.
			{
				Shader triangleShader( "assets\\shaders\\DX\\shader.hlsl" );

				// Define the vertex input layout.
				D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				// Describe and create the graphics pipeline state object (PSO).
				D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc ={};
				psoDesc.InputLayout ={ inputElementDescs, _countof( inputElementDescs ) };
				psoDesc.pRootSignature = m_RootSignature.Get();
				psoDesc.VS = CD3DX12_SHADER_BYTECODE( triangleShader.VertexShader().Get() );
				psoDesc.PS = CD3DX12_SHADER_BYTECODE( triangleShader.PixelShader().Get() );
				psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT );
				psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
				psoDesc.DepthStencilState.DepthEnable = FALSE;
				psoDesc.DepthStencilState.StencilEnable = FALSE;
				psoDesc.SampleMask = UINT_MAX;
				psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
				psoDesc.NumRenderTargets = 1;
				psoDesc.RTVFormats[ 0 ] = DXGI_FORMAT_R8G8B8A8_UNORM;
				psoDesc.SampleDesc.Count = 1;
				ThrowIfFailed( m_Device->CreateGraphicsPipelineState( &psoDesc, IID_PPV_ARGS( &m_PipelineState ) ) );
			}

			// Create the command list.
			ThrowIfFailed( m_Device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), m_PipelineState.Get(), IID_PPV_ARGS( &m_CommandList ) ) );

			// Command lists are created in the recording state, but there is nothing
			// to record yet. The main loop expects it to be closed, so close it now.
			ThrowIfFailed( m_CommandList->Close() );

			m_AspectRatio = Window::Get().Width() / Window::Get().Height();

			// Create the vertex buffer.
			{
				// Define the geometry for a triangle.
				DXVertex triangleVertices[] =
				{
					{ { 0.0f, 0.25f * m_AspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
					{ { 0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
					{ { -0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
				};

				const UINT vertexBufferSize = sizeof( triangleVertices );

				// Note: using upload heaps to transfer static data like vert buffers is not 
				// recommended. Every time the GPU needs it, the upload heap will be marshalled 
				// over. Please read up on Default Heap usage. An upload heap is used here for 
				// code simplicity and because there are very few verts to actually transfer.
				ThrowIfFailed( m_Device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer( vertexBufferSize ),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS( &m_VertexBuffer ) ) );

				// Copy the triangle data to the vertex buffer.
				UINT8* pVertexDataBegin;
				CD3DX12_RANGE readRange( 0, 0 );        // We do not intend to read from this resource on the CPU.
				ThrowIfFailed( m_VertexBuffer->Map( 0, &readRange, reinterpret_cast< void** >( &pVertexDataBegin ) ) );
				memcpy( pVertexDataBegin, triangleVertices, sizeof( triangleVertices ) );
				m_VertexBuffer->Unmap( 0, nullptr );

				// Initialize the vertex buffer view.
				m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
				m_VertexBufferView.StrideInBytes = sizeof( DXVertex );
				m_VertexBufferView.SizeInBytes = vertexBufferSize;
			}

			// Create synchronization objects and wait until assets have been uploaded to the GPU.
			{
				ThrowIfFailed( m_Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_Fence ) ) );
				m_FenceValue = 1;

				// Create an event handle to use for frame synchronization.
				m_FenceEvent = CreateEvent( nullptr, FALSE, FALSE, nullptr );
				if( m_FenceEvent == nullptr )
				{
					ThrowIfFailed( HRESULT_FROM_WIN32( GetLastError() ) );
				}

				// Wait for the command list to execute; we are reusing the same command 
				// list in our main loop but for now, we just want to wait for setup to 
				// complete before continuing.
				WaitForPreviousFrame();
			}
		}

	}

	void Renderer::Render()
	{
		PopulateCommandList();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] ={ m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

		// Present the frame.
		ThrowIfFailed( m_SwapChain->Present( 1, 0 ) );

		WaitForPreviousFrame();
	}

}