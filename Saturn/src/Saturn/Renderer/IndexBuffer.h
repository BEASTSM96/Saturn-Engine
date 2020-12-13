#pragma once

#include "Saturn/Core/Ref.h"
#include "Saturn/Core/Buffer.h"

#include "RendererAPI.h"


namespace Saturn {
	
	using RendererID = uint32_t;

	class IndexBuffer : public RefCounted
	{
	public:
		virtual ~IndexBuffer() { }

		virtual void SetData( void* buffer, uint32_t size, uint32_t offset = 0 ) = 0;
		virtual void Bind( void ) const = 0;

		virtual uint32_t GetCount( void ) const = 0;

		virtual uint32_t GetSize( void ) const = 0;
		virtual RendererID GetRendererID( void ) const = 0;

		static Ref<IndexBuffer> Create( uint32_t size );
		static Ref<IndexBuffer> Create( void* data, uint32_t size = 0 );
	};
}