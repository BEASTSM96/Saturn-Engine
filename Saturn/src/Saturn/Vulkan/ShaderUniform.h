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

#include "Texture.h"

#include "ShaderDataType.h"

namespace Saturn {

	struct ShaderUniform
	{
		ShaderUniform() {}
		~ShaderUniform()
		{
			Terminate();
		}

		ShaderUniform( const std::string& name, int location, ShaderDataType type )
			: Name( name ), Location( location ), Type( type )
		{
			UUID = location * 2;
		}

		void Terminate()
		{
			switch( Type )
			{
				case Saturn::ShaderDataType::None:
				case Saturn::ShaderDataType::Float:
				case Saturn::ShaderDataType::Float2:
				case Saturn::ShaderDataType::Float3:
				case Saturn::ShaderDataType::Float4:
				case Saturn::ShaderDataType::Mat3:
				case Saturn::ShaderDataType::Mat4:
				case Saturn::ShaderDataType::Int:
				case Saturn::ShaderDataType::Int2:
				case Saturn::ShaderDataType::Int3:
				case Saturn::ShaderDataType::Int4:
				case Saturn::ShaderDataType::Bool:
				{
					if( pValue != nullptr )
					{
						pValue = 0;
					}
				} break;
					
				case Saturn::ShaderDataType::SamplerCube:
				case Saturn::ShaderDataType::Sampler2D:
				{
					// Ty = Texture2D
					
					if ( pValue )
					{				
						Texture2D* Texture = ( Texture2D* )pValue;
						Texture->Terminate();
					}
				} break;

				default:
					break;
			}
			
			Location = -1;
			Type = ShaderDataType::None;
			pValue = nullptr;
			UUID = 0;
		}

		operator bool () const
		{
			return !( pValue == nullptr );
		}

		template<typename Ty>
		void Set( Ty& Value )
		{
			pValue = &Value;
		}

		template<typename Ty>
		Ty* As()
		{
			return ( Ty* )pValue;
		}

		ShaderUniform& operator=( const ShaderUniform& other )
		{
			Name = other.Name;
			Location = other.Location;
			Type = other.Type;
			pValue = other.pValue;
			return *this;
		}

		bool operator==( const ShaderUniform& other )
		{
			return ( Name == other.Name && Location == other.Location && Type == other.Type );
		}

		std::string Name = "";
		int Location = -1;
		ShaderDataType Type = ShaderDataType::None;

		int UUID;

		void* pValue = nullptr;
	};

}