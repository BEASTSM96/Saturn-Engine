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

#include "Saturn/Core/Log.h"
#include "Saturn/Core/Base.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <stdexcept>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// From DXSamples on GitHub https://github.com/microsoft/DirectX-Graphics-Samples

#define SAT_DX_SAFE_RELEASE( x ) if ( ( x ) ) ( x )->Release()

namespace Saturn {

	struct DXVertex
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	inline std::string HResultToString( HRESULT result )
	{
		char s_str[ 64 ] ={};
		sprintf_s( s_str, "HRESULT of 0x%08X", static_cast< UINT >( result ) );

		return std::string( s_str );
	}

	class HResultException : public std::runtime_error
	{
	public:

		HResultException( HRESULT result ) : std::runtime_error( HResultToString( result ) ), m_HResult( result ) 
		{

		}

		HRESULT Error() const { return m_HResult; }

	private:

		const HRESULT m_HResult;
	};

	inline void AssertIfFailed( HRESULT result )
	{
		if( FAILED( result ) )
		{
			SAT_ASSERT( false, "HResult failed." );
		}
	}

	inline void ThrowIfFailed( HRESULT result )
	{
	#if defined( SAT_DEBUG )
		if( FAILED( result ) )
		{
			SAT_CORE_ERROR( HResultToString( result ) );
			throw HResultException( result );
		}
	#else
		if( FAILED( result ) )
		{
			SAT_CORE_ERROR( HResultToString( result ) );
			throw HResultException( result );
		}
	#endif
	}

	// Resets all elements in a ComPtr array.
	template< class Ty >
	inline void ResetComPtrArray( Ty* comPtrArray )
	{
		for( auto& i : *comPtrArray )
			i.Reset();
	}

	// Resets all elements in a unique_ptr array.
	template< class Ty >
	inline void ResetUniquePtrArray( Ty* uniquePtrArray )
	{
		for( auto& i : *uniquePtrArray )
			i.reset();
	}

	inline void GetHardwareAdapter( _In_ IDXGIFactory1* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false ) 
	{
		*ppAdapter = nullptr;

		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
		if( SUCCEEDED( pFactory->QueryInterface( IID_PPV_ARGS( &factory6 ) ) ) )
		{
			for(
				UINT adapterIndex = 0;
				SUCCEEDED( factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
				IID_PPV_ARGS( &adapter ) ) );
				++adapterIndex )
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1( &desc );

				if( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if( SUCCEEDED( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof( ID3D12Device ), nullptr ) ) )
				{
					break;
				}
			}
		}

		if( adapter.Get() == nullptr )
		{
			for( UINT adapterIndex = 0; SUCCEEDED( pFactory->EnumAdapters1( adapterIndex, &adapter ) ); ++adapterIndex )
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1( &desc );

				if( desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE )
				{
					// Don't select the Basic Render Driver adapter.
					// If you want a software adapter, pass in "/warp" on the command line.
					continue;
				}

				// Check to see whether the adapter supports Direct3D 12, but don't create the
				// actual device yet.
				if( SUCCEEDED( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof( ID3D12Device ), nullptr ) ) )
				{
					break;
				}
			}
		}

		*ppAdapter = adapter.Detach();
	}
}