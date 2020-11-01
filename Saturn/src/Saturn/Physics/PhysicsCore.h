#pragma once

#include <cstdint>

namespace Saturn::Physics {

	namespace /*Core*/ {

		enum class CollisionDetectionMode : std::uint8_t
		{
			Discrete,
			Continuous
		};

		enum class ColliderType : std::uint8_t
		{
			Box,
			Sphere,
			Capsule,
			Mesh
		};

		enum class FilterGroup : std::uint32_t
		{
			None							= 0,
			GroupI							= 1,
			GroupII =		GroupI			<< 1,
			GroupIII =		GroupII			<< 1,
			GroupIV =		GroupIII		<< 1,
			GroupV =		GroupIV			<< 1,
			GroupVI =		GroupV			<< 1,
			GroupVII =		GroupVI			<< 1,
			GroupVIII =		GroupVII		<< 1,
			GroupIX =		GroupVIII		<< 1,
			GroupX =		GroupIX			<< 1,
			GroupXI =		GroupX			<< 1,
			GroupXII =		GroupXI			<< 1,
			GroupXIII =		GroupXII		<< 1,
			GroupXIV =		GroupXIII		<< 1,
			GroupXV =		GroupXIV		<< 1,
			GroupXVI =		GroupXV			<< 1,
			All =			~None,
			Default =		 GroupI
		};

		FilterGroup operator&(FilterGroup lhs, FilterGroup rhs);

		FilterGroup operator|(FilterGroup lhs, FilterGroup rhs);

		FilterGroup operator^(FilterGroup lhs, FilterGroup rhs);

		FilterGroup& operator&=(FilterGroup& lhs, FilterGroup rhs);

		FilterGroup& operator|=(FilterGroup& lhs, FilterGroup rhs);

		FilterGroup operator^=(FilterGroup& lhs, FilterGroup rhs);

		FilterGroup operator~(FilterGroup lhs);

		enum class ForceMode : std::uint8_t
		{
			Force,
			Acceleration,
			Impulse,
			VelocityChange
		};

		enum class TriggerType : std::uint8_t
		{
			New,
			Persistent,
			Lost
		};

		template <typename T>
		T* GetDataAs() 
		{
			return static_cast<T*>(getData()->data);
		}

		template <typename T>
		const T* GetDataAs() 
		{
			return static_cast<T*>(getData()->data);
		}

	}

}