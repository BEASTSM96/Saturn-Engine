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
#include "PhysicsScene.h"

#include "Saturn/Scene/Scene.h"

#include <GLFW/glfw3.h>

#include "Saturn/Renderer/Renderer.h"

#include "Saturn/Scene/Components.h"

namespace Saturn {

	PhysicsScene::PhysicsScene( Scene* scene ) : m_scene( scene ), m_eventListener( this )
	{
		m_world = m_common.createPhysicsWorld();

		// Get a reference to the debug renderer 
		rp3d::DebugRenderer& debugRenderer = m_world->getDebugRenderer();

		m_world->setEventListener( &m_eventListener );
		m_world->setIsDebugRenderingEnabled( true );

		RegLog();
	}

	PhysicsScene::~PhysicsScene()
	{

	}

	void PhysicsScene::RegLog()
	{
		std::string name = "Test";

		rp3d::PhysicsWorld::WorldSettings worldSettings;
		worldSettings.worldName = name;

		rp3d::DefaultLogger* defaultLogger = m_common.createDefaultLogger();

		rp3d::uint logLevel = static_cast< rp3d::uint >( static_cast< rp3d::uint >( rp3d::Logger::Level::Warning ) | static_cast< rp3d::uint >( rp3d::Logger::Level::Error ) );

		defaultLogger->addFileDestination( "rp3d_log_" + name + ".html", logLevel, rp3d::DefaultLogger::Format::HTML );
		m_common.setLogger( defaultLogger );

		// Enable debug rendering 
		m_world->setIsDebugRenderingEnabled( true );

		rp3d::DebugRenderer& debugRenderer = m_world->getDebugRenderer();
	}

	void PhysicsScene::ContactStay( rp3d::CollisionBody* body, rp3d::CollisionBody* other )
	{
		m_scene->ContactStay( body, other );
	}

	void PhysicsScene::ContactEnter( rp3d::CollisionBody* body, rp3d::CollisionBody* other )
	{
		m_scene->ContactEnter( body, other );
	}

	void PhysicsScene::ContactExit( rp3d::CollisionBody* body, rp3d::CollisionBody* other )
	{
		m_scene->ContactExit( body, other );
	}

	void PhysicsScene::Update( float delta )
	{
		const float timeStep = 1.0f / 60.0f;

		m_accumulator += delta;


		while( m_accumulator >= timeStep )
		{
			m_world->update( timeStep );
			m_scene->PhysicsUpdate( PhysicsType::ReactPhysics, timeStep );

			m_accumulator -= delta;

		}
	}

	void PhysicsScene::Contact( rp3d::CollisionBody* body )
	{
		m_scene->Contact( body );
	}

	glm::vec3 Vec3FromReactVec3( const reactphysics3d::Vector3& matrix )
	{
		glm::vec3 result;
		result.x = matrix.x;
		result.y = matrix.y;
		result.z = matrix.z;
		return result;
	}

	glm::vec3 Vec3FromReactQuaternion( const reactphysics3d::Quaternion& matrix )
	{
		glm::vec3 result;
		result.x = matrix.x;
		result.y = matrix.y;
		result.z = matrix.z;
		return result;
	}

	glm::mat4 Mat4FromReactMat4( const reactphysics3d::Transform& matrix )
	{
		glm::mat4 result;
		result = glm::translate( result, Vec3FromReactVec3( matrix.getPosition() ) );
		//result = glm::rotate(Vec3FromReactQuaternion(matrix.getOrientation()));
		return result;
	}
}