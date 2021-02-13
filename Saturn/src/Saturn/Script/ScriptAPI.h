#pragma once

#include "Saturn/KeyCodes.h"
#include "ScriptHelpers.h"
#include "ScriptEngine.h"
#include "Saturn/Scene/Components.h"

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

	bool Saturn_Input_IsKeyPressed( KeyCode key );

	void Saturn_Entity_GetTransform( uint64_t entityID, glm::mat4* transform );
	void Saturn_Entity_SetTransform( uint64_t entityID, glm::mat4* transform );
	void Saturn_Entity_CreateComponent( uint64_t entityID, void* type );
	bool Saturn_Entity_HasComponent( uint64_t entityID, void* type );

	Mesh Saturn_Entity_GetMesh( uint64_t entityID );
	void Saturn_Entity_SetMesh( uint64_t entityID, void* type );

	MonoString* Saturn_TagComponent_GetTag( uint64_t entityID );
	void Saturn_TagComponent_SetTag( uint64_t entityID, MonoString* tag );
}
}