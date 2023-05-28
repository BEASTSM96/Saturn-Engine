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
#include "JoltMeshColliderStream.h"

#include "Saturn/Project/Project.h"

namespace Saturn {

	JoltMeshColliderStream::JoltMeshColliderStream( const Ref<JoltMeshCollider>& asset )
		: m_Asset( asset )
	{

	}

	JoltMeshColliderStream::~JoltMeshColliderStream()
	{

	}

	void JoltMeshColliderStream::Serialise()
	{
		std::ofstream fout( m_Asset->Path, std::ios::binary );

		const char* pHeader = "SMC\0";

		fout.write( pHeader, 5 );

		size_t ColliderSize = m_Asset->m_Shapes.size();
		fout.write( reinterpret_cast< const char* >( &ColliderSize ), sizeof( size_t ) );
		fout.write( reinterpret_cast< const char* >( m_Asset->m_Shapes.data() ), ColliderSize * sizeof( m_Asset->m_Shapes ) );

		fout.close();
	}

	void JoltMeshColliderStream::Deserialise()
	{
		std::ifstream input( m_Asset->Path, std::ios::binary );

		input.seekg( 0, std::ios::end );
		std::streampos size = input.tellg();
		input.seekg( 0, std::ios::beg );

		std::vector<char> content( size );
		input.read( content.data(), size );

		input.close();

		// Validate the header
		if( content[ 0 ] != 'S' || content[ 1 ] != 'M' || content[ 2 ] != 'C' || content[ 3 ] != '\0' )
		{
			SAT_CORE_ERROR( "Invaild mesh collider header!" );
		}


	}

}