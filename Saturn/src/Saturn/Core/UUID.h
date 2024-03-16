/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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

#include "Base.h"

#if defined ( SAT_LINUX )
#include <functional>
#else
#include <xhash>
#endif

namespace Saturn {

	// "UUID" (universally unique identifier) or GUID is (usually) a 128-bit integer.
	class UUID
	{
	public:
		UUID();
		UUID( uint64_t uuid );
		UUID( const UUID& other );


		operator uint64_t() { return m_UUID; }
		operator const uint64_t() const { return m_UUID; }

	public:
		static void Serialise( const UUID& rObject, std::ofstream& rStream );
		static void Deserialise( UUID& rObject, std::istream& rStream );

	private:

		uint64_t m_UUID;
	};

}

namespace std {

	template <>
	struct hash<Saturn::UUID>
	{
		std::size_t operator()( const Saturn::UUID& uuid ) const
		{
			return hash<uint64_t>()( ( uint64_t )uuid );
		}
	};
}
