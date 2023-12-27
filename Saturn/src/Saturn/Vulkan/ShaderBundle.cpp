/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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
#include "ShaderBundle.h"

#include "Saturn/Project/Project.h"
#include "Shader.h"

#include "Saturn/Serialisation/RawSerialisation.h"

namespace Saturn {

	struct ShaderBundleHeader
	{
		const char Magic[5] = ".SB\0";
		size_t Shaders;
	};

	ShaderBundle::ShaderBundle()
	{
	}

	ShaderBundle::~ShaderBundle()
	{
	}

	bool ShaderBundle::BundleShaders()
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();

		if( !std::filesystem::exists( cachePath ) )
			std::filesystem::create_directories( cachePath );

		cachePath /= "ShaderBundle.ssb";

		std::ofstream fout( cachePath, std::ios::binary | std::ios::trunc );

		ShaderBundleHeader header{};
		header.Shaders = ShaderLibrary::Get().GetShaders().size();

		fout.write( reinterpret_cast< char* >( &header ), sizeof( ShaderBundleHeader ) );

		int i = 0;
		for( auto&& [name, shader] : ShaderLibrary::Get().GetShaders() )
		{
			SAT_CORE_INFO( "Packaging shader: {0}", name );

			fout.write( name.c_str(), name.length() );
			fout.write( reinterpret_cast<char*>( &i ), sizeof( int ) );

			shader->SerialiseShaderData( fout );

			i++;
		}

		fout.close();
	
		return true;
	}

	void ShaderBundle::ReadBundle()
	{
		std::filesystem::path cachePath = Project::GetActiveProject()->GetFullCachePath();
		cachePath /= "ShaderBundle.ssb";

		if( !std::filesystem::exists( cachePath ) )
			return;

		Buffer FileBuffer;
		std::ifstream stream( cachePath, std::ios::binary | std::ios::ate );

		auto end = stream.tellg();
		stream.seekg( 0, std::ios::beg );
		auto size = end - stream.tellg();

		FileBuffer.Allocate( size );
		stream.read( reinterpret_cast<char*>( FileBuffer.Data ), FileBuffer.Size );
		stream.close();

		ShaderBundleHeader header = *(ShaderBundleHeader*) FileBuffer.Data;

		if( strcmp( header.Magic, ".SB\0" ) )
		{
			SAT_CORE_ERROR( "Invalid shader bundle file header!" );
			return;
		}

		uint8_t* shaderData = FileBuffer.As<uint8_t>() + sizeof( ShaderBundleHeader );

		for( uint32_t i = 0; i < header.Shaders; i++ )
		{
			Ref<Shader> shader = Ref<Shader>::Create();

			std::string name = RawSerialisation::ReadString( &shaderData );
			shader->m_Name = name;

			shader->DeserialiseShaderData( &shaderData );

			ShaderLibrary::Get().Add( shader );
		}

		// We are done with the file buffer.
		FileBuffer.Free();
	}
}