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
#include "PhysXMaterial.h"

namespace Saturn {

	PhysXMaterial::PhysXMaterial( PhysXScene* scene, std::string name )
	{
		m_Scene = scene;
		StaticFriction = 0.5f;
		DynamicFriction = StaticFriction;
		Restitution = 0.6f;
		m_Material = m_Scene->m_Physics->createMaterial( StaticFriction, DynamicFriction, Restitution );
	}

	PhysXMaterial::PhysXMaterial( PhysXScene* scene, std::string name, float staticFriction, float dynamicFriction, float restitution )
	{
		m_Scene = scene;
		StaticFriction = staticFriction;
		DynamicFriction = dynamicFriction;
		Restitution = restitution;
		m_Material = m_Scene->m_Physics->createMaterial( staticFriction, dynamicFriction, restitution );
	}

	PhysXMaterial::~PhysXMaterial()
	{
		if ( m_Material )
		{
			m_Material->release();
			m_Material = nullptr;
		}
	}

}
