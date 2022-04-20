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

#include <stdint.h>

namespace Saturn {

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	enum class ShaderUniformTypes
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool, Sampler2D, SamplerCube
	};

	// Vulkan shader type sizes
	static uint32_t ShaderDataTypeSize( ShaderDataType type )
	{
		switch( type )
		{
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Bool:		return 1;
		}

		return 0;
	}

	// Vulkan shader type sizes
	static uint32_t ShaderUniformTypesSize( ShaderUniformTypes type )
	{
		switch( type )
		{
			case ShaderUniformTypes::Float:		    return 4;
			case ShaderUniformTypes::Float2:	    return 4 * 2;
			case ShaderUniformTypes::Float3:	    return 4 * 3;
			case ShaderUniformTypes::Float4:	    return 4 * 4;
			case ShaderUniformTypes::Mat3:		    return 4 * 3 * 3;
			case ShaderUniformTypes::Mat4:		    return 4 * 4 * 4;
			case ShaderUniformTypes::Int:		    return 4;
			case ShaderUniformTypes::Int2:		    return 4 * 2;
			case ShaderUniformTypes::Int3:		    return 4 * 3;
			case ShaderUniformTypes::Int4:		    return 4 * 4;
			case ShaderUniformTypes::Bool:		    return 1;
			case ShaderUniformTypes::Sampler2D:	    return 1;
			case ShaderUniformTypes::SamplerCube:	return 1;
		}

		return 0;
	}
}