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

#pragma once

#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Scene.h"

#pragma warning(disable: 4005)

namespace Saturn {
	class Serialiser
	{
	public:
		Serialiser();
		Serialiser( const std::string& objectname, bool shouldSerialise = true );
		Serialiser( const Ref<Scene>& scene );

		virtual ~Serialiser();

		virtual void Serialise( const std::string& filepath );
		virtual void Deserialise( const std::string& filepath );

		virtual void SerialiseEntity( YAML::Emitter& out, Entity entity );

		virtual void SerialiseVC( const std::string& filepath );
		virtual void DeserialiseVC( const std::string& filepath );

		virtual void SerialiseProjectSettings( const std::string& filepath );
		virtual void DeserialiseProjectSettings( const std::string& filepath );

		virtual void SerialiseProject( const std::string& filepath );
		virtual void DeserialiseProject( const std::string& filepath );

		std::string m_ObjectName;
		bool m_shouldSerialise;
	private:
		Ref<Scene> m_Scene;

		friend class Scene;

	};
}
