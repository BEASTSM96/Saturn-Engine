#include "sppch.h"
#include "UUID.h"

#include <random>

namespace Saturn {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 eng(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	UUID::UUID()
		: m_UUID(s_UniformDistribution(eng))
	{
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
	}

	UUID::UUID(const UUID& other)
		: m_UUID(other.m_UUID)
	{
	}

	unsigned int rand_char()
	{
		using namespace std;

		random_device rd;
		mt19937 gen( rd() );
		uniform_int_distribution<> dis( 0, 255 );
		return dis( gen );
	}
}
