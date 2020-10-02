#pragma once

#include <xhash>

namespace Saturn {

	// "UUID" (universally unique identifier) or GUID is (usually) a 128-bit integer.
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID& other);

		operator uint64_t () { return m_UUID; }
		operator const uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};

}

_STD_BEGIN
	template <>
	struct hash<Saturn::UUID>
	{
		std::size_t operator()(const Saturn::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
_STD_END