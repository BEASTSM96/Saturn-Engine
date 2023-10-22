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

#include "Saturn/Scene/Entity.h"
#include "Asset.h"

namespace Saturn {

	class Scene;

	class Prefab : public Asset
	{
	public:
		Prefab();
		virtual ~Prefab();

		void Create( const Ref<Entity>& srcEntity );
		void Create();

		Ref<Entity> PrefabToEntity( Ref<Scene> Scene, Ref<Entity>& entity );

		Ref<Scene>& GetScene() { return m_Scene; }
		const Ref<Scene>& GetScene() const { return m_Scene; }

		// Only used when create a prefab of a game class.
		void SetEntity( const Ref<Entity>& entity ) { m_Entity = entity; }
		void CreateScene();

	private:
		Ref<Entity> CreateFromEntity( Ref<Entity>& srcEntity );
		Ref<Entity> CreateChildren( const Ref<Entity>& parent, Ref<Scene> Scene );
	private:
		Ref<Entity> m_Entity;
		
		// We need a scene to create entities in the prefab.
		Ref<Scene> m_Scene = nullptr;

	private:
		friend class PrefabSerialiser;
	};
}