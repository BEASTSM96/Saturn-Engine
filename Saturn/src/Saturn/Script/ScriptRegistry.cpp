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
SAT_CORE_ERROR("No C# component class for ", #_Type); \
} \
}

	static void InitComponentType() 
	{
		ComponentRegisterType( TagComponent );
		ComponentRegisterType( IdComponent );
		ComponentRegisterType( TransformComponent );
		ComponentRegisterType( MeshComponent );
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

		mono_add_internal_call( "Saturn.Entity::GetTransform_Native", Saturn::Scripting::Saturn_Entity_GetTransform );
		mono_add_internal_call( "Saturn.Entity::SetTransform_Native", Saturn::Scripting::Saturn_Entity_SetTransform );
		mono_add_internal_call( "Saturn.Entity::CreateComponent_Native", Saturn::Scripting::Saturn_Entity_CreateComponent );
		mono_add_internal_call( "Saturn.Entity::HasComponent_Native", Saturn::Scripting::Saturn_Entity_HasComponent );

		mono_add_internal_call( "Saturn.TagComponent::GetTag_Native", Saturn::Scripting::Saturn_TagComponent_GetTag );
		mono_add_internal_call( "Saturn.TagComponent::SetTag_Native", Saturn::Scripting::Saturn_TagComponent_SetTag );
	}
}
