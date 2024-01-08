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

#include <fstream>
#include <unordered_map>

namespace Saturn {

	// Helpers for reading/writing in binary.
	class RawSerialisation
	{
	public:
		template<typename K, typename V>
		static void WriteMap( const std::unordered_map<K, V>& rMap, std::ofstream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& [key, value] : rMap )
			{
				if constexpr( std::is_trivial<K>() )
				{
					WriteObject( key, rStream );
				}
				else
				{
					K::Serialise( key, rStream );
				}

				if constexpr( std::is_trivial<V>() )
				{
					WriteObject( value, rStream );
				}
				else
				{
					V::Serialise( value, rStream );
				}
			}
		}

		template<typename K, typename V>
		static void WriteMap( const std::unordered_map<K, std::vector<V>>& rMap, std::ofstream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& [key, value] : rMap )
			{
				if constexpr( std::is_trivial<K>() )
				{
					WriteObject( key, rStream );
				}
				else
				{
					K::Serialise( key, rStream );
				}

				WriteVector( value, rStream );
			}
		}

		template<typename Ty>
		static void WriteVector( const std::vector<Ty>& rMap, std::ofstream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& value : rMap )
			{
				if constexpr( std::is_trivial<Ty>() )
				{
					WriteObject( value, rStream );
				}
				else
				{
					Ty::Serialise( value, rStream );
				}
			}
		}

		template<typename Ty>
		static void WriteObject( const Ty& rObject, std::ofstream& rStream )
		{
			rStream.write( reinterpret_cast< const char* >( &rObject ), sizeof( Ty ) );
		}

		template<typename Ty>
		static void ReadObject( Ty& rObject, std::ifstream& rStream )
		{
			rStream.read( reinterpret_cast<char*>( &rObject ), sizeof( Ty ) );
		}

		static void WriteString( const std::string& rString, std::ofstream& rStream )
		{
			size_t size = rString.size();
			rStream.write( reinterpret_cast< const char* >( &size ), sizeof( size ) );

			rStream.write( rString.data(), size );
		}

		template<typename Ty>
		static void ReadVector( std::vector<Ty>& rMap, std::ifstream& rStream )
		{
			if( rMap.size() )
				rMap.clear();

			size_t size = 0;
			rStream.read( reinterpret_cast< char* >( &size ), sizeof( size_t ) );
			rMap.resize( size );

			for( size_t i = 0; i < size; i++ )
			{
				Ty value{};
				if constexpr( std::is_trivial<Ty>() )
				{
					ReadObject<Ty>( value, rStream );
				}
				else
				{
					Ty::Deserialise( value, rStream );
				}

				rMap[i] = value;
			}
		}

		template<typename K, typename V>
		static void ReadMap( std::unordered_map<K, V>& rMap, std::ifstream& rStream )
		{
			if( rMap.size() )
				rMap.clear();

			size_t size = 0;
			rStream.read( reinterpret_cast< char* >( &size ), sizeof( size_t ) );

			for( size_t i = 0; i < size; i++ )
			{
				K key{};
				if constexpr( std::is_trivial<K>() )
				{
					ReadObject<K>( key, rStream );
				}
				else
				{
					K::Deserialise( key, rStream );
				}

				V value{};
				if constexpr( std::is_trivial<V>() )
				{
					ReadObject<V>( value, rStream );
				}
				else
				{
					V::Deserialise( value, rStream );
				}

				rMap[ key ] = value;
			}
		}

		template<typename K, typename V>
		static void ReadMap( std::unordered_map<K, std::vector<V>>& rMap, std::ifstream& rStream )
		{
			if( rMap.size() )
				rMap.clear();

			size_t size = 0;
			rStream.read( reinterpret_cast< char* >( &size ), sizeof( size_t ) );

			for( size_t i = 0; i < size; i++ )
			{
				K key{};
				if constexpr( std::is_trivial<K>() )
				{
					ReadObject<K>( key, rStream );
				}
				else
				{
					K::Deserialise( key, rStream );
				}

				std::vector<V> values{};
				ReadVector( values, rStream );

				rMap[ key ] = std::move( values );
			}
		}

		static std::string ReadString( std::ifstream& rStream )
		{
			size_t length = 0;
			rStream.read( reinterpret_cast< char* >( &length ), sizeof( size_t ) );

			char* TemporaryBuffer = new char[ length + 1 ];
			rStream.read( TemporaryBuffer, length );

			TemporaryBuffer[ length ] = '\0';

			std::string result = TemporaryBuffer;

			delete[] TemporaryBuffer;

			return result;
		}
	};
}