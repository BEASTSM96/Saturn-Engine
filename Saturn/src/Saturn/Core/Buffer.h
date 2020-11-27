#pragma once

#include "Saturn/Log.h"
#include "Saturn/Core/Base.h"

namespace Saturn {

	struct Buffer
	{
		u8* Data;
		u32 Size;

		Buffer()
			: Data(nullptr), Size(0)
		{
		}

		Buffer(u8* data, u32 size)
			: Data(data), Size(size)
		{
		}

		static Buffer Copy(void* data, u32 size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(u32 size)
		{
			delete[] Data;
			Data = nullptr;

			if (size == 0) {
				SAT_CORE_ASSERT(!size == 0, "The size that was given was 0!");
				return;
			}

			Data = new u8[size];
			Size = size;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		template<typename T>
		T& Read(u32 offset = 0)
		{
			return *(T*)(Data + offset);
		}

		void Write(void* data, u32 size, u32 offset = 0)
		{
			SAT_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
			memcpy(Data + offset, data, size);
		}

		operator bool() const
		{
			return Data;
		}

		u8& operator[](int index)
		{
			return Data[index];
		}

		u8 operator[](int index) const
		{
			return Data[index];
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		inline u32 GetSize() const { return Size; }


	};
}