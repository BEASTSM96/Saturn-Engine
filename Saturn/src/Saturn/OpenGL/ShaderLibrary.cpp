/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
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

#include "sppch.h"
#include "ShaderLibrary.h"

namespace Saturn {

	ShaderLibaray::ShaderLibaray()
	{
	}

	ShaderLibaray::~ShaderLibaray()
	{
		m_Shaders.clear();
	}

	const Ref<Shader>& ShaderLibaray::Get( const std::string& name )
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) != m_Shaders.end(), "Shader was not found in the libaray" );

		return m_Shaders.at( name );
	}

	void ShaderLibaray::Add( const Ref<Shader>& shader )
	{
		auto& name = shader->Name();

		SAT_CORE_ASSERT( m_Shaders.find( name ) == m_Shaders.end(), "Shader was already found in the libaray" );

		m_Shaders[ name ] = shader;
	}

	void ShaderLibaray::Load( const std::string& name, const std::string& filepath )
	{
		SAT_CORE_ASSERT( m_Shaders.find( name ) != m_Shaders.end(), "Shader was not found in the libaray" );

		m_Shaders[ name ] = Ref<Shader>::Create( path );
	}

}
