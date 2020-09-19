#pragma once


#include <string>
#include "Saturn/Core.h"

namespace Saturn { 


	/*             Sparky Texture         */
	class SATURN_API Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
	};

	/*               Sparky Texture 2D        */
	class SATURN_API Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(const std::string& path);
	};

}

