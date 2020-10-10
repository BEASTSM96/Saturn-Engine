#pragma once

#include <stdint.h>

namespace Saturn::Core {

	class RefCounter
	{
	public:
		virtual void IncRefCount() const = 0;
		virtual void DecRefCount() const = 0;
		virtual uint32_t GetRefCount() const = 0;
	};

	class RefCounted : RefCounter
	{
	public:
		void IncRefCount() const override
		{
			m_RefCount++;
		}
		void DecRefCount() const override
		{
			m_RefCount--;
		}

		uint32_t GetRefCount() const override { return m_RefCount; }
	private:
		mutable uint32_t m_RefCount = 0; // TODO: atomic
	};

}