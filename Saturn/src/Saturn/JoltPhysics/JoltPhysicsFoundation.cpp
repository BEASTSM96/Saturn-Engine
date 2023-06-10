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

#include "JoltConversions.h"

#include "JoltMeshCollider.h"

#include "Saturn/Scene/Entity.h"

#include "Saturn/Asset/AssetRegistry.h"
#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>

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
					return inObject2 == Layers::MOVING;
				case Layers::MOVING:
					return true;
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

	BPLayerInterfaceImpl BroadPhaseLayerInterface;
	ObjectVsBroadPhaseLayerFilterImpl ObjectVBroadphaseLayerFilter;
	ObjectLayerPairFilterImpl ObjectVObjectLayerFilter;

	//////////////////////////////////////////////////////////////////////////

	JoltPhysicsFoundation::JoltPhysicsFoundation()
	{
	}

	JoltPhysicsFoundation::~JoltPhysicsFoundation()
	{
	}

	static void JphTrace( const char* inFMT, ... )
	{
		// Format the message
		va_list list;
		va_start( list, inFMT );
		char buffer[ 1024 ];
		vsnprintf( buffer, sizeof( buffer ), inFMT, list );
		va_end( list );

		SAT_CORE_INFO( "Jolt: {0}", buffer );
	}

	void JoltPhysicsFoundation::Init()
	{
		JPH::RegisterDefaultAllocator();

		JPH::Trace = JphTrace;

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		m_PhysicsSystem = new JPH::PhysicsSystem();
		m_PhysicsSystem->Init( 1024, 0, 1024, 1024, BroadPhaseLayerInterface, ObjectVBroadphaseLayerFilter, ObjectVObjectLayerFilter );

		m_PhysicsSystem->SetContactListener( &m_ContactListener );

		m_JobSystem = new JPH::JobSystemThreadPool( JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() / 2 );
		m_Allocator = new JPH::TempAllocatorImpl( 10 * 1024 * 1024 ); // 10MB buffer

		m_PhysicsSystem->OptimizeBroadPhase();
	}

	void JoltPhysicsFoundation::Terminate()
	{
		JPH::UnregisterTypes();

		m_JobSystem = nullptr;
		m_Allocator = nullptr;
		m_PhysicsSystem = nullptr;

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	void JoltPhysicsFoundation::Update( Timestep ts )
	{
		m_PhysicsSystem->Update( ts.Seconds(), 1, 1, m_Allocator, m_JobSystem );
	}

	void JoltPhysicsFoundation::DestroyBody( JPH::Body* pBody )
	{
		JPH::BodyInterface& rBodyInterface = m_PhysicsSystem->GetBodyInterface();
		rBodyInterface.RemoveBody( pBody->GetID() );
		rBodyInterface.DestroyBody( pBody->GetID() );
	}

	JPH::Body* JoltPhysicsFoundation::CreateRigidBody( const JPH::Shape* pShape, const JPH::Vec3& Position, const JPH::Quat& Rotation, bool Kinematic )
	{
		JPH::BodyInterface& rBodyInterface = m_PhysicsSystem->GetBodyInterface();

		JPH::BodyCreationSettings BodySettings( pShape, Position, Rotation, 
			Kinematic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic, 
			Kinematic ? Layers::NON_MOVING : Layers::MOVING );

		JPH::Body* Body = nullptr;
		Body = rBodyInterface.CreateBody( BodySettings );

		rBodyInterface.AddBody( Body->GetID(), Kinematic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate );

		return Body;
	}

	JPH::Body* JoltPhysicsFoundation::CreateBoxCollider( Entity& rEntity, const glm::vec3& Extents )
	{
		TransformComponent& tc = rEntity.GetComponent<TransformComponent>();
		RigidbodyComponent& rb = rEntity.GetComponent<RigidbodyComponent>();

		JPH::BodyInterface& rBodyInterface = m_PhysicsSystem->GetBodyInterface();

		JPH::Vec3 extent = Auxiliary::GLMToJPH( Extents );
		JPH::BoxShapeSettings Settings( extent );

		// Create the box.
		JPH::ShapeSettings::ShapeResult Result = Settings.Create();
		JPH::ShapeRefC Box = Result.Get();

		JPH::Vec3 pos = Auxiliary::GLMToJPH( tc.Position );
		JPH::Quat rot = Auxiliary::GLMQuatToJPH( glm::quat( tc.Rotation ) );

		JPH::Body* Body = CreateRigidBody( Box, pos, rot, rb.IsKinematic );

		return Body;
	}

	JPH::Body* JoltPhysicsFoundation::CreateCapsuleCollider( Entity& rEntity, float Extents, float Height )
	{
		TransformComponent& tc = rEntity.GetComponent<TransformComponent>();
		RigidbodyComponent& rb = rEntity.GetComponent<RigidbodyComponent>();

		JPH::BodyInterface& rBodyInterface = m_PhysicsSystem->GetBodyInterface();

		JPH::CapsuleShapeSettings Settings( Extents, Height );

		// Create the box.
		JPH::ShapeSettings::ShapeResult Result = Settings.Create();
		JPH::ShapeRefC Capsule = Result.Get();

		JPH::Vec3 pos = Auxiliary::GLMToJPH( tc.Position );
		JPH::Quat rot = Auxiliary::GLMQuatToJPH( glm::quat( tc.Rotation ) );

		JPH::Body* Body = CreateRigidBody( Capsule, pos, rot, rb.IsKinematic );

		return Body;
	}

	JPH::Body* JoltPhysicsFoundation::CreateSphereCollider( Entity& rEntity, float Extents )
	{
		TransformComponent& tc = rEntity.GetComponent<TransformComponent>();
		RigidbodyComponent& rb = rEntity.GetComponent<RigidbodyComponent>();

		JPH::BodyInterface& rBodyInterface = m_PhysicsSystem->GetBodyInterface();

		JPH::SphereShapeSettings Settings( Extents );

		// Create the box.
		JPH::ShapeSettings::ShapeResult Result = Settings.Create();
		JPH::ShapeRefC Sphere = Result.Get();

		JPH::Vec3 pos = Auxiliary::GLMToJPH( tc.Position );
		JPH::Quat rot = Auxiliary::GLMQuatToJPH( glm::quat( tc.Rotation ) );

		JPH::Body* Body = CreateRigidBody( Sphere, pos, rot, rb.IsKinematic );

		return Body;
	}

	JPH::Body* JoltPhysicsFoundation::CreateMeshCollider( Entity& rEntity, UUID ID )
	{
		StaticMeshComponent& mc = rEntity.GetComponent<StaticMeshComponent>();
		TransformComponent& tc = rEntity.GetComponent<TransformComponent>();

		Ref<Asset> asset = AssetRegistry::Get().FindAsset( ID );
		Ref<JoltMeshCollider> meshCollider = asset.As<JoltMeshCollider>();

		meshCollider = Ref<JoltMeshCollider>::Create( mc.Mesh, tc.Scale );
		meshCollider->ID = asset->ID;
		meshCollider->Type = asset->Type;
		meshCollider->Name = asset->Name;
		meshCollider->Path = asset->Path;

		meshCollider->Load();
		meshCollider->CreateBodies( rEntity );

		return meshCollider->GetFirstBody();
	}

	void JoltPhysicsFoundation::GenerateMeshCollider( Ref<StaticMesh> mesh, const glm::vec3& rScale )
	{
		AssetID id = AssetRegistry::Get().CreateAsset( AssetType::MeshCollider );
		Ref<Asset> asset = AssetRegistry::Get().FindAsset( id );

		asset->Name = mesh->Name;

		Ref<JoltMeshCollider> meshCollider = asset.As<JoltMeshCollider>();
		meshCollider = Ref<JoltMeshCollider>::Create( mesh, rScale );
		meshCollider->ID = asset->ID;
		meshCollider->Type = asset->Type;
		meshCollider->Name = asset->Name;

		meshCollider->Save();
		
		asset->SetPath( meshCollider->Path );

		AssetRegistrySerialiser ars;
		ars.Serialise();
	}

}