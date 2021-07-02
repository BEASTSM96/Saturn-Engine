#include "sppch.h"
#include "ScriptRegistry.h"

#include <mono/jit/jit.h>
#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"

#include "ScriptAPI.h"

namespace Saturn {

	std::unordered_map<MonoType*, std::function<bool( Entity& )>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void( Entity& )>> s_CreateComponentFuncs;

	extern MonoImage* s_CoreAssemblyImage;

#define ComponentRegisterType(_Type) \
{ \
MonoType* type = mono_reflection_type_from_name( "Saturn." #_Type, s_CoreAssemblyImage ); \
if(type) { \
uint32_t id = mono_type_get_type( type ); \
s_HasComponentFuncs[ type ] = []( Entity& entity ) { return entity.HasComponent<_Type>(); }; \
s_CreateComponentFuncs[type] = [](Entity& entity) { return entity.AddComponent<_Type>(); }; \
} else { \
SAT_CORE_ERROR("No C# component class for {0}", #_Type); \
} \
}

	static void InitComponentType() 
	{
		ComponentRegisterType( PhysXRigidbodyComponent );
		ComponentRegisterType( TagComponent );
		ComponentRegisterType( IdComponent );
		ComponentRegisterType( TransformComponent );
		ComponentRegisterType( MeshComponent );
		ComponentRegisterType( PhysXMeshColliderComponent );
		ComponentRegisterType( PhysXBoxColliderComponent );
		ComponentRegisterType( PhysXSphereColliderComponent );
		ComponentRegisterType( PhysXCapsuleColliderComponent );
		ComponentRegisterType( ScriptComponent );
		ComponentRegisterType( CameraComponent );
	}

	void ScriptRegistry::RegisterAll()
	{
		InitComponentType();

		mono_add_internal_call( "Saturn.Log::Fatal_Native", Saturn::Scripting::Saturn_Log_Fatal );
		mono_add_internal_call( "Saturn.Log::Error_Native", Saturn::Scripting::Saturn_Log_Error );
		mono_add_internal_call( "Saturn.Log::Warn_Native", Saturn::Scripting::Saturn_Log_Warn );
		mono_add_internal_call( "Saturn.Log::Info_Native", Saturn::Scripting::Saturn_Log_Info );
		mono_add_internal_call( "Saturn.Log::Trace_Native", Saturn::Scripting::Saturn_Log_Trace );

		mono_add_internal_call( "Saturn.Input::IsKeyPressed_Native", Saturn::Scripting::Saturn_Input_IsKeyPressed );
		mono_add_internal_call( "Saturn.Input::IsMouseButtonPressed_Native", Saturn::Scripting::Saturn_Input_IsMouseButtonPressed );
		mono_add_internal_call( "Saturn.Input::GetCursorMode_Native", Saturn::Scripting::Saturn_Input_GetCursorMode );
		mono_add_internal_call( "Saturn.Input::GetMousePosition_Native", Saturn::Scripting::Saturn_Input_GetMousePosition );
		mono_add_internal_call( "Saturn.Input::SetCursorMode_Native", Saturn::Scripting::Saturn_Input_SetCursorMode );

		mono_add_internal_call( "Saturn.Entity::CreateComponent_Native", Saturn::Scripting::Saturn_Entity_CreateComponent );
		mono_add_internal_call( "Saturn.Entity::HasComponent_Native", Saturn::Scripting::Saturn_Entity_HasComponent );

		mono_add_internal_call( "Saturn.Entity::GetMesh_Native", Saturn::Scripting::Saturn_Entity_GetMesh );
		mono_add_internal_call( "Saturn.Entity::SetMesh_Native", Saturn::Scripting::Saturn_Entity_SetMesh );
		mono_add_internal_call( "Saturn.Entity::FindEntityByTag_Native", Saturn::Scripting::Saturn_Entity_FindEntityByTag );

		mono_add_internal_call( "Saturn.TagComponent::GetTag_Native", Saturn::Scripting::Saturn_TagComponent_GetTag );
		mono_add_internal_call( "Saturn.TagComponent::SetTag_Native", Saturn::Scripting::Saturn_TagComponent_SetTag );

		mono_add_internal_call( "Saturn.TransformComponent::GetTransform_Native", Saturn::Scripting::Saturn_TransformComponent_GetTransform );
		mono_add_internal_call( "Saturn.TransformComponent::SetTransform_Native", Saturn::Scripting::Saturn_TransformComponent_SetTransform );
		mono_add_internal_call( "Saturn.TransformComponent::GetTranslation_Native", Saturn::Scripting::Saturn_TransformComponent_GetTranslation );
		mono_add_internal_call( "Saturn.TransformComponent::SetTranslation_Native", Saturn::Scripting::Saturn_TransformComponent_SetTranslation );
		mono_add_internal_call( "Saturn.TransformComponent::GetRotation_Native", Saturn::Scripting::Saturn_TransformComponent_GetRotation );
		mono_add_internal_call( "Saturn.TransformComponent::SetRotation_Native", Saturn::Scripting::Saturn_TransformComponent_SetRotation );
		mono_add_internal_call( "Saturn.TransformComponent::GetScale_Native", Saturn::Scripting::Saturn_TransformComponent_GetScale );
		mono_add_internal_call( "Saturn.TransformComponent::SetScale_Native", Saturn::Scripting::Saturn_TransformComponent_SetScale );

		mono_add_internal_call( "Saturn.PhysXRigidbodyComponent::GetLinearVelocity_Native", Saturn::Scripting::Saturn_RigidBodyComponent_GetLinearVelocity );
		mono_add_internal_call( "Saturn.PhysXRigidbodyComponent::SetLinearVelocity_Native", Saturn::Scripting::Saturn_RigidBodyComponent_SetLinearVelocity );
		mono_add_internal_call( "Saturn.PhysXRigidbodyComponent::Rotate_Native", Saturn::Scripting::Saturn_RigidBodyComponent_Rotate );

		mono_add_internal_call( "Saturn.PhysXRigidbodyComponent::AddForce_Native", Saturn::Scripting::Saturn_RigidBodyComponent_ApplyForce );

		mono_add_internal_call( "Saturn.TimeStep::GetTimeStep_Native", Saturn::Scripting::Saturn_TimeStep_GetTimeStep );
	}
}
