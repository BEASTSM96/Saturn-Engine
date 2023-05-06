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
#include "JoltPhysicsFoundation.h"

#include "JoltDynamicRigidBody.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include <cstdarg>

namespace Layers {
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

namespace BroadPhaseLayers {
	static constexpr JPH::BroadPhaseLayer NON_MOVING( 0 );
	static constexpr JPH::BroadPhaseLayer MOVING( 1 );
	static constexpr uint32_t NUM_LAYERS( 2 );
};

namespace Saturn {

	//////////////////////////////////////////////////////////////////////////

	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool ShouldCollide( JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2 ) const override
		{
			switch( inObject1 )
			{
				case Layers::NON_MOVING:
					return inObject2 == Layers::MOVING; // Non moving only collides with moving
				case Layers::MOVING:
					return true; // Moving collides with everything
				default:
					SAT_ASSERT( false );
					return false;
			}
		}
	};

	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool ShouldCollide( JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2 ) const override
		{
			switch( inLayer1 )
			{
				case Layers::NON_MOVING:
					return inLayer2 == BroadPhaseLayers::MOVING;
				case Layers::MOVING:
					return true;
				default:
					SAT_ASSERT( false );
					return false;
			}
		}
	};

	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			m_ObjectToBroadPhase[ Layers::NON_MOVING ] = BroadPhaseLayers::NON_MOVING;
			m_ObjectToBroadPhase[ Layers::MOVING ]     = BroadPhaseLayers::MOVING;
		}

		virtual uint32_t GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer( JPH::ObjectLayer inLayer ) const override
		{
			SAT_ASSERT( inLayer < Layers::NUM_LAYERS );

			return m_ObjectToBroadPhase[ inLayer ];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName( JPH::BroadPhaseLayer inLayer ) const override
		{
			switch( ( JPH::BroadPhaseLayer::Type ) inLayer )
			{
				case ( JPH::BroadPhaseLayer::Type ) BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
				case ( JPH::BroadPhaseLayer::Type ) BroadPhaseLayers::MOVING:		return "MOVING";
				default:													        SAT_ASSERT( false ); return "INVALID";
			}
		}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer m_ObjectToBroadPhase[ Layers::NUM_LAYERS ];
	};

	//////////////////////////////////////////////////////////////////////////

	JoltPhysicsFoundation::JoltPhysicsFoundation()
	{
		JPH::RegisterDefaultAllocator();

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		BPLayerInterfaceImpl BroadPhaseLayerInterface;
		ObjectVsBroadPhaseLayerFilterImpl ObjectVBroadphaseLayerFilter;
		ObjectLayerPairFilterImpl ObjectVObjectLayerFilter;

		m_PhysicsSystem = new JPH::PhysicsSystem();
		m_PhysicsSystem->Init( 1024, 0, 1024, 1024, BroadPhaseLayerInterface, ObjectVBroadphaseLayerFilter, ObjectVObjectLayerFilter );

		JoltPhysicsContactListener ContactListener;
		m_PhysicsSystem->SetContactListener( &ContactListener );
	}

	JoltPhysicsFoundation::~JoltPhysicsFoundation()
	{
		JPH::UnregisterTypes();

		delete m_PhysicsSystem;
		m_PhysicsSystem = nullptr;

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	void JoltPhysicsFoundation::AddShape( PhysicsShape Shape, JoltDynamicRigidBody* pBody )
	{
		switch( Shape )
		{
			case Saturn::PhysicsShape::BOX: 
			{
				AddBoxCollider( pBody );
			} break;

			case Saturn::PhysicsShape::SPHERE:
				break;
			case Saturn::PhysicsShape::CAPSULE:
				break;
			default:
				break;
		}
	}

	void JoltPhysicsFoundation::DestoryBody( JPH::Body* pBody )
	{
		JPH::BodyInterface& rBodyInterface = m_PhysicsSystem->GetBodyInterface();
		rBodyInterface.RemoveBody( pBody->GetID() );
		rBodyInterface.DestroyBody( pBody->GetID() );
	}

	void JoltPhysicsFoundation::AddBoxCollider( JoltDynamicRigidBody* pBody )
	{
		pBody->DestoryBody();

		auto& rBoxColliderComp = pBody->GetEntity().GetComponent<BoxColliderComponent>();
		auto& trans = pBody->GetEntity().GetComponent<TransformComponent>();
		
		JPH::BodyInterface& rBodyInterface = m_PhysicsSystem->GetBodyInterface();

		JPH::BoxShapeSettings Settings( JPH::Vec3( rBoxColliderComp.Extents.x, rBoxColliderComp.Extents.y, rBoxColliderComp.Extents.z ) );

		// Create the box.
		JPH::ShapeSettings::ShapeResult Result = Settings.Create();
		JPH::ShapeRefC Box = Result.Get();

		// No rotation?
		JPH::BodyCreationSettings BodySettings( Box, JPH::RVec3( trans.Position.x, trans.Position.y, trans.Position.z ), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::NON_MOVING );

		JPH::Body* Body = rBodyInterface.CreateBody( BodySettings );
		pBody->SetBody( Body );
	}

}