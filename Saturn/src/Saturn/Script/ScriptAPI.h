#pragma once

#include "Saturn/Input.h"
#include "ScriptHelpers.h"
#include "ScriptEngine.h"
#include "Saturn/Scene/Components.h"

#include "Saturn/Physics/PhysX/PhysXFnd.h"

#include <glm/glm.hpp>

extern "C" {
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
};

namespace Saturn {
namespace Scripting {

	void Saturn_Log_Fatal( MonoString* msg );
	void Saturn_Log_Error( MonoString* msg );
	void Saturn_Log_Warn( MonoString* msg );
	void Saturn_Log_Info( MonoString* msg );
	void Saturn_Log_Trace( MonoString* msg );

	// Input
	bool Saturn_Input_IsKeyPressed( KeyCode key );
	bool Saturn_Input_IsMouseButtonPressed( MouseButton button );
	void Saturn_Input_SetCursorMode( CursorMode mode );
	CursorMode Saturn_Input_GetCursorMode();
	void Saturn_Input_GetMousePosition( glm::vec2* outPos );
	
	void Saturn_Entity_CreateComponent( uint64_t entityID, void* type );
	bool Saturn_Entity_HasComponent( uint64_t entityID, void* type );

	void Saturn_TransformComponent_GetTransform( uint64_t entityID, TransformComponent* outTransform );
	void Saturn_TransformComponent_SetTransform( uint64_t entityID, TransformComponent* inTransform );
	void Saturn_TransformComponent_GetTranslation( uint64_t entityID, glm::vec3* outTranslation );
	void Saturn_TransformComponent_SetTranslation( uint64_t entityID, glm::vec3* inTranslation );
	void Saturn_TransformComponent_GetRotation( uint64_t entityID, glm::vec3* outRotation );
	void Saturn_TransformComponent_SetRotation( uint64_t entityID, glm::vec3* inRotation );
	void Saturn_TransformComponent_GetScale( uint64_t entityID, glm::vec3* outScale );
	void Saturn_TransformComponent_SetScale( uint64_t entityID, glm::vec3* inScale );
	// Rotation
	void Saturn_TransformComponent_GetRotationX( uint64_t entityID, glm::vec3* outX );
	void Saturn_TransformComponent_GetRotationY( uint64_t entityID, glm::vec3* outY );
	void Saturn_TransformComponent_GetRotationZ( uint64_t entityID, glm::vec3* outZ );

	Mesh Saturn_Entity_GetMesh( uint64_t entityID );
	void Saturn_Entity_SetMesh( uint64_t entityID, void* type );

	MonoString* Saturn_TagComponent_GetTag( uint64_t entityID );
	void Saturn_TagComponent_SetTag( uint64_t entityID, MonoString* tag );

	bool Saturn_Physics_Raycast( glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit );

	void Saturn_RigidBodyComponent_GetLinearVelocity( uint64_t entityID, glm::vec3* outVelocity );
	void Saturn_RigidBodyComponent_SetLinearVelocity( uint64_t entityID, glm::vec3* velocity );
	void Saturn_RigidBodyComponent_ApplyForce( uint64_t entityID, glm::vec3* inForceDir, ForceType type );
	void Saturn_RigidBodyComponent_Rotate( uint64_t entityID, glm::vec3* rotation );

	void Saturn_TimeStep_GetTimeStep(float* outTime);

	uint64_t Saturn_Entity_FindEntityByTag(MonoString* tag);

}
}