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

#include <string>
#include "Common.h"

namespace Saturn {

	enum class ShaderDomain
	{
		None = 0, Vertex = 0, Pixel = 1
	};

	class Shader
	{
	public:
		Shader() = default;
		Shader( const std::string& filename );

		const std::string& Name() const { return m_Name; }
		std::string& Name() { return m_Name; }

		ComPtr<ID3DBlob>& VertexShader() { return m_VertexShader; }
		ComPtr<ID3DBlob>& PixelShader() { return m_PixelShader; }

	private:

		void Load( const std::string& filepath );
		void Parse();

	private:

		ComPtr<ID3DBlob> m_VertexShader;
		ComPtr<ID3DBlob> m_PixelShader;

		bool m_Loaded = false;
		bool m_IsCompute = false;
		std::string m_Name, m_Filepath;
	};

}