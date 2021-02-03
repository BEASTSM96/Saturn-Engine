#include "sppch.h"
#include "ScriptAPI.h"

#include "Saturn/Input.h"
#include "Saturn/Scene/Entity.h"

#include <mono/jit/jit.h>

namespace Saturn {
	extern std::unordered_map <uint32_t, Scene*> s_ActiveScene;
	extern std::unordered_map<MonoType*, std::function<bool( Entity& )>> s_HasComponentFuncs;
	extern std::unordered_map<MonoType*, std::function<void( Entity& )>> s_CreateComponentFuncs;
}

namespace Saturn::Scripting {

	std::string MonoToString(MonoString* str) 
	{
		mono_unichar2* chl = mono_string_chars( str );
		std::string out( "" );
		for( int i = 0; i < mono_string_length( str ); i++ )
			out += chl[ i ];
		return out;
	}

	void Saturn_Log_Fatal( MonoString* msg )
	{
		SAT_FATAL( MonoToString( msg ) );
	}

	void Saturn_Log_Error( MonoString* msg )
	{
		SAT_ERROR( MonoToString( msg ) );
	}

	void Saturn_Log_Info( MonoString* msg )
	{
		SAT_INFO( MonoToString( msg ) );
	}

	void Saturn_Log_Trace( MonoString* msg )
	{
		SAT_TRACE( MonoToString( msg ) );
	}

	bool Saturn_Input_IsKeyPressed( KeyCode key )
	{
		return Input::IsKeyPressed( key );
	}

	void Saturn_Entity_GetTransform( uint32_t entityID, glm::mat4* transform )
	{
		auto& comp = ScriptHelpers::GetCompFromScene<TransformComponent>( ScriptEngine::GetScene(), entityID );
		*transform = comp.GetTransform();
	}

	void Saturn_Entity_SetTransform( uint32_t entityID, glm::mat4* transform )
	{
		auto& comp = ScriptHelpers::GetCompFromScene<TransformComponent>( ScriptEngine::GetScene(), entityID );
		comp.GetTransform() = *transform;
	}

	void Saturn_Entity_CreateComponent( uint32_t entityID, void* type )
	{
		auto& map =  ScriptHelpers::GetEntityMap( ScriptEngine::GetScene(), entityID );
		Entity e = map.at( entityID );
		MonoType* monoType = mono_reflection_type_get_type( ( MonoReflectionType* )type );
		s_CreateComponentFuncs[ monoType ]( e );
	}

	bool Saturn_Entity_HasComponent( uint32_t entityID, void* type )
	{
		auto& map =  ScriptHelpers::GetEntityMap( ScriptEngine::GetScene(), entityID );
		Entity e = map.at( entityID );
		MonoType* monoType = mono_reflection_type_get_type( ( MonoReflectionType* )type );
		bool res = s_HasComponentFuncs[ monoType ]( e );
		return res;
	}

	MonoString* Saturn_TagComponent_GetTag( uint32_t entityID )
	{
		auto& comp = ScriptHelpers::GetCompFromScene<TagComponent>( ScriptEngine::GetScene(), entityID );
		std::string tag = comp.Tag;
		return mono_string_new( mono_domain_get(), tag.c_str() );
	}

	void Saturn_TagComponent_SetTag( uint32_t entityID, MonoString* tag )
	{
		auto& comp = ScriptHelpers::GetCompFromScene<TagComponent>( ScriptEngine::GetScene(), entityID );
		std::string strtag = comp.Tag;
		comp.Tag = MonoToString( tag );
	}
}
