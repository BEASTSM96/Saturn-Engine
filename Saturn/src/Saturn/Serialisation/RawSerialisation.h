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

#pragma once

#include <fstream>
#include <unordered_map>

namespace Saturn {

	// Helpers for reading/writing in binary.
	class RawSerialisation
	{
	public:
		template<typename Ty>
		static void WriteMap( const Ty& rMap, std::ofstream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& [key, value] : rMap )
			{
				WriteObject( key, rStream );
				WriteObject( value, rStream );
			}
		}

		template<typename Ty>
		static void WriteVector( const Ty& rMap, std::ofstream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& value : rMap )
			{
				WriteObject( value, rStream );
			}
		}

		template<typename Ty>
		static void WriteObject( const Ty& rObject, std::ofstream& rStream )
		{
			rStream.write( reinterpret_cast< const char* >( &rObject ), sizeof( Ty ) );
		}

		static void WriteString( const std::string& rString, std::ofstream& rStream )
		{
			size_t size = rString.size();

			rStream.write( reinterpret_cast< const char* >( &size ), sizeof( size ) );

			rStream.write( rString.data(), size );
		}

		template<typename Ty>
		static void ReadVector( uint8_t** ppData, Ty& rMap ) 
		{
			if( rMap.size() )
				rMap.clear();

			size_t size = *( size_t* ) *ppData;
			*ppData += sizeof( size_t );

			rMap.resize( size );
			
			std::memcpy( rMap.data(), *ppData, size * sizeof( typename Ty::value_type ) );
			*ppData += size * sizeof( typename Ty::value_type );
		}

		template<typename MapType, typename K, typename V>
		static void ReadMap( uint8_t** ppData, MapType& rMap )
		{
			if( rMap.size() )
				rMap.clear();

			size_t size = *( size_t* ) *ppData;
			*ppData += sizeof( size_t );

			for( size_t i = 0; i < size; i++ )
			{
				K key;
				std::memcpy( &key, *ppData, sizeof( K ) );
				*ppData += sizeof( K );

				V value;
				std::memcpy( &value, *ppData, sizeof( V ) );
				*ppData += sizeof( V );

				rMap[ key ] = value;
			}
		}

		static std::string ReadString( uint8_t** ppData )
		{
			size_t size = *( size_t* ) ppData;
			*ppData += sizeof( size_t );

			std::string result( reinterpret_cast< const char* >( *ppData ), size );

			*ppData += size;
			
			return result;
		}
	};
}