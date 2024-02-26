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

#include "Saturn/Core/Memory/Buffer.h"
#include "Saturn/Core/Serialisable.h"

#include <fstream>
#include <unordered_map>
#include <map>
#include <filesystem>

namespace Saturn {

	// Helpers for reading/writing in binary.
	class RawSerialisation
	{
	public:
		template<typename K, typename V, typename OStream>
		static void WriteUnorderedMap( const std::unordered_map<K, V>& rMap, OStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, K>::value, "K is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					K::Serialise( key, rStream );
				}

				if constexpr( std::is_trivial<V>() )
				{
					WriteObject( value, rStream );
				}
				else
				{
					//static_assert( std::is_base_of<Serialisable, V>::value, "V is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					V::Serialise( value, rStream );
				}
			}
		}

		template<typename V, typename OStream>
		static void WriteUnorderedMap( const std::unordered_map<std::string, V>& rMap, OStream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& [key, value] : rMap )
			{
				WriteString( key, rStream );

				if constexpr( std::is_trivial<V>() )
				{
					WriteObject( value, rStream );
				}
				else
				{
					//static_assert( std::is_base_of<Serialisable, V>::value, "V is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					V::Serialise( value, rStream );
				}
			}
		}

		// TODO: Move this into the VFS.
		// This is only used by the VFS.
		template<typename OStream>
		static void WriteUnorderedMap( const std::unordered_map<std::string, std::filesystem::path>& rMap, OStream& rStream )
		{
			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& [key, value] : rMap )
			{
				WriteString( key, rStream );
				WriteString( value.string(), rStream );
			}
		}

		template<typename K, typename V, typename OStream>
		static void WriteUnorderedMap( const std::unordered_map<K, std::vector<V>>& rMap, OStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, K>::value, "K is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					K::Serialise( key, rStream );
				}

				WriteVector( value, rStream );
			}
		}

		template<typename K, typename V, typename OStream>
		static void WriteMap( const std::map<K, V>& rMap, OStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, K>::value, "K is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					K::Serialise( key, rStream );
				}

				if constexpr( std::is_trivial<V>() )
				{
					WriteObject( value, rStream );
				}
				else
				{
					//static_assert( std::is_base_of<Serialisable, V>::value, "V is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					V::Serialise( value, rStream );
				}
			}
		}

		template<typename V, typename OStream>
		static void WriteMap( const std::map<std::string, V>& rMap, OStream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& [key, value] : rMap )
			{
				WriteString( key, rStream );

				if constexpr( std::is_trivial<V>() )
				{
					WriteObject( value, rStream );
				}
				else
				{
					//static_assert( std::is_base_of<Serialisable, V>::value, "V is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					V::Serialise( value, rStream );
				}
			}
		}

		template<typename K, typename K2, typename V, typename OStream>
		static void WriteMap( const std::map<K, std::map<K2, V>>& rMap, OStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, K>::value, "K is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					K::Serialise( key, rStream );
				}

				WriteMap( value, rStream );
			}
		}

		// Used for maps with a string type as key and a filesystem path as K2
		template<typename V, typename OStream>
		static void WriteMap( const std::map<std::string, std::map<std::filesystem::path, V>>& rMap, OStream& rStream )
		{
			if( !rStream.is_open() )
				return;

			size_t mapSize = rMap.size();
			rStream.write( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( const auto& [key, value] : rMap )
			{
				WriteString( key, rStream );

				WriteMap( value, rStream );
			}
		}

		template<typename Ty, typename OStream>
		static void WriteVector( const std::vector<Ty>& rMap, OStream& rStream )
		{
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
					//static_assert( std::is_base_of<Serialisable, Ty>::value, "Ty is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					Ty::Serialise( value, rStream );
				}
			}
		}

		template<typename Ty, typename OStream>
		static size_t WriteObject( const Ty& rObject, OStream& rStream )
		{
			rStream.write( reinterpret_cast< const char* >( &rObject ), sizeof( Ty ) );

			return sizeof( Ty );
		}

		template<typename Ty, typename IStream>
		static void ReadObject( Ty& rObject, IStream& rStream )
		{
			rStream.read( reinterpret_cast<char*>( &rObject ), sizeof( Ty ) );
		}

		template<typename OStream>
		static size_t WriteString( const std::string& rString, OStream& rStream )
		{
			size_t size = rString.size();
			rStream.write( reinterpret_cast< char* >( &size ), sizeof( size_t ) );

			rStream.write( rString.data(), size );

			return size;
		}

		template<typename OStream>
		static size_t WriteString( const std::filesystem::path& rString, OStream& rStream )
		{
			std::string stringbuf = rString.string();

			return WriteString( stringbuf, rStream );
		}

		template<typename OStream>
		static size_t WriteString( const std::stringstream& rString, OStream& rStream )
		{
			std::string stringbuf = rString.str();

			size_t size = stringbuf.size();
			rStream.write( reinterpret_cast< const char* >( &size ), sizeof( size ) );

			rStream.write( stringbuf.data(), size );

			return size;
		}

		template<typename Ty, typename IStream>
		static void ReadVector( std::vector<Ty>& rMap, IStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, Ty>::value, "Ty is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					Ty::Deserialise( value, rStream );
				}

				rMap[i] = value;
			}
		}

		template<typename K, typename V, typename IStream>
		static void ReadUnorderedMap( std::unordered_map<K, V>& rMap, IStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, K>::value, "K is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					K::Deserialise( key, rStream );
				}

				V value{};
				if constexpr( std::is_trivial<V>() )
				{
					ReadObject<V>( value, rStream );
				}
				else
				{
					//static_assert( std::is_base_of<Serialisable, V>::value, "V is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					V::Deserialise( value, rStream );
				}

				rMap[ key ] = value;
			}
		}

		// TODO: Move this into the VFS.
		// This is only used by the VFS.
		template<typename IStream>
		static void ReadUnorderedMap( std::unordered_map<std::string, std::filesystem::path>& rMap, IStream& rStream )
		{
			size_t mapSize = 0;
			rStream.read( reinterpret_cast< char* >( &mapSize ), sizeof( size_t ) );

			for( size_t i = 0; i < mapSize; i++ )
			{
				std::string K{};
				std::filesystem::path V{};

				K = ReadString( rStream );
				V = ReadString( rStream );

				rMap[ K ] = V;
			}
		}

		template<typename V, typename IStream>
		static void ReadUnorderedMap( std::unordered_map<std::string, V>& rMap, IStream& rStream )
		{
			if( rMap.size() )
				rMap.clear();

			size_t size = 0;
			rStream.read( reinterpret_cast< char* >( &size ), sizeof( size_t ) );

			for( size_t i = 0; i < size; i++ )
			{
				std::string key{};
				key = ReadString( rStream );

				V value{};
				if constexpr( std::is_trivial<V>() )
				{
					ReadObject<V>( value, rStream );
				}
				else
				{
					//static_assert( std::is_base_of<Serialisable, V>::value, "V is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					V::Deserialise( value, rStream );
				}

				rMap[ key ] = value;
			}
		}

		template<typename K, typename V, typename IStream>
		static void ReadUnorderedMap( std::unordered_map<K, std::vector<V>>& rMap, IStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, K>::value, "K is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					K::Deserialise( key, rStream );
				}

				std::vector<V> values{};
				ReadVector( values, rStream );

				rMap[ key ] = std::move( values );
			}
		}

		template<typename K, typename V, typename IStream>
		static void ReadMap( std::map<K, V>& rMap, IStream& rStream )
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
					//static_assert( std::is_base_of<Serialisable, K>::value, "K is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					K::Deserialise( key, rStream );
				}

				V value{};
				if constexpr( std::is_trivial<V>() )
				{
					ReadObject<V>( value, rStream );
				}
				else
				{
					//static_assert( std::is_base_of<Serialisable, V>::value, "V is not is trivial and is not based from Serialisable. All non trivial types in Saturn must be derived from Serialisable!" );

					V::Deserialise( value, rStream );
				}

				rMap[ key ] = value;
			}
		}

		template<typename IStream>
		static std::string ReadString( IStream& rStream )
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

		template<typename OStream>
		static void WriteVec2( const glm::vec2& rVec, OStream& rStream )
		{
			glm::vec2 temporaryVec( rVec );

			rStream.write( reinterpret_cast< char* >( &temporaryVec.x ), sizeof( float ) );
			rStream.write( reinterpret_cast< char* >( &temporaryVec.y ), sizeof( float ) );
		}

		template<typename IStream>
		static void ReadVec2( glm::vec2& rVec, IStream& rStream )
		{
			float x, y;

			rStream.read( reinterpret_cast< char* >( &x ), sizeof( float ) );
			rStream.read( reinterpret_cast< char* >( &y ), sizeof( float ) );

			rVec = glm::vec2( x, y );
		}

		template<typename OStream>
		static size_t WriteVec3( const glm::vec3& rVec, OStream& rStream )
		{
			glm::vec3 temporaryVec( rVec );

			rStream.write( reinterpret_cast< char* >( &temporaryVec.x ), sizeof( float ) );
			rStream.write( reinterpret_cast< char* >( &temporaryVec.y ), sizeof( float ) );
			rStream.write( reinterpret_cast< char* >( &temporaryVec.z ), sizeof( float ) );

			return sizeof( float ) * 3;
		}

		template<typename IStream>
		static void ReadVec3( glm::vec3& rVec, IStream& rStream )
		{
			float x, y, z;

			rStream.read( reinterpret_cast< char* >( &x ), sizeof( float ) );
			rStream.read( reinterpret_cast< char* >( &y ), sizeof( float ) );
			rStream.read( reinterpret_cast< char* >( &z ), sizeof( float ) );

			rVec = glm::vec3( x, y, z );
		}

		template<typename OStream>
		static void WriteVec4( const glm::vec4& rVec, OStream& rStream )
		{
			glm::vec4 temporaryVec( rVec );

			rStream.write( reinterpret_cast< char* >( &temporaryVec.x ), sizeof( float ) );
			rStream.write( reinterpret_cast< char* >( &temporaryVec.y ), sizeof( float ) );
			rStream.write( reinterpret_cast< char* >( &temporaryVec.z ), sizeof( float ) );
			rStream.write( reinterpret_cast< char* >( &temporaryVec.w ), sizeof( float ) );
		}

		template<typename IStream>
		static void ReadVec4( glm::vec4& rVec, IStream& rStream )
		{
			float x, y, z, w;

			rStream.read( reinterpret_cast< char* >( &x ), sizeof( float ) );
			rStream.read( reinterpret_cast< char* >( &y ), sizeof( float ) );
			rStream.read( reinterpret_cast< char* >( &z ), sizeof( float ) );
			rStream.read( reinterpret_cast< char* >( &w ), sizeof( float ) );

			rVec = glm::vec4( x, y, z, w );
		}

		template<typename OStream>
		static void WriteMatrix4x4( const glm::mat4& rMat, OStream& rStream )
		{
			glm::mat4 temporaryMat( rMat );

			WriteVec4( temporaryMat[ 0 ], rStream );
			WriteVec4( temporaryMat[ 1 ], rStream );
			WriteVec4( temporaryMat[ 2 ], rStream );
			WriteVec4( temporaryMat[ 3 ], rStream );
		}

		template<typename IStream>
		static void ReadMatrix4x4( glm::mat4& rMat, IStream& rStream )
		{
			glm::mat4 newMat{};

			ReadVec4( newMat[ 0 ], rStream );
			ReadVec4( newMat[ 1 ], rStream );
			ReadVec4( newMat[ 2 ], rStream );
			ReadVec4( newMat[ 3 ], rStream );
			
			rMat = newMat;
		}

		template<typename OStream>
		static void WriteSaturnBuffer( Buffer& rBuffer, OStream& rStream )
		{
			rStream.write( reinterpret_cast<char*>( &rBuffer.Size ), sizeof( size_t ) );
			rStream.write( reinterpret_cast<char*>( rBuffer.Data ), rBuffer.Size );
		}

		template<typename IStream>
		static void ReadSaturnBuffer( Buffer& rBuffer, IStream& rStream )
		{
			rBuffer.Free();

			size_t BufferSize = 0;

			rStream.read( reinterpret_cast< char* >( &BufferSize ), sizeof( size_t ) );
			
			uint8_t* pData = new uint8_t[ BufferSize ];
			rStream.read( reinterpret_cast< char* >( pData ), BufferSize );

			rBuffer = Buffer::Copy( pData, BufferSize );

			delete[] pData;
		}
	};
}