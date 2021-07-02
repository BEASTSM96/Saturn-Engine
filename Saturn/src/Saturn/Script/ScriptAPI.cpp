#include "sppch.h"
#include "ScriptAPI.h"

#include "Saturn/Input.h"
#include "Saturn/Scene/Entity.h"

#include "Saturn/Application.h"

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

	void Saturn_Log_Warn( MonoString* msg )
	{
		SAT_WARN( MonoToString( msg ) );
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

	bool Saturn_Input_IsMouseButtonPressed( MouseButton button )
	{
		return Input::IsMouseButtonPressed( button );
	}

	void Saturn_Input_SetCursorMode( CursorMode mode )
	{
		Input::SetCursorMode( mode );
	}

	CursorMode Saturn_Input_GetCursorMode()
	{
		return Input::GetCursorMode();
	}

	void Saturn_Input_GetMousePosition( glm::vec2* outPos )
	{
		auto [x,y] = Input::GetMousePosition();
		*outPos ={ x, y };
	}

	void Saturn_Entity_CreateComponent( uint64_t entityID, void* type )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		auto& map =  ScriptHelpers::GetEntityMap( scene, entityID );

		Entity e = map.at( entityID );
		MonoType* monoType = mono_reflection_type_get_type( ( MonoReflectionType* )type );
		s_CreateComponentFuncs[ monoType ]( e );
	}

	bool Saturn_Entity_HasComponent( uint64_t entityID, void* type )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		auto& map =  ScriptHelpers::GetEntityMap( scene, entityID );

		Entity e = map.at( entityID );
		MonoType* monoType = mono_reflection_type_get_type( ( MonoReflectionType* )type );
		bool res = s_HasComponentFuncs[ monoType ]( e );
		return res;
	}

	void Saturn_TransformComponent_GetTransform( uint64_t entityID, TransformComponent* outTransform )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		const auto& entityMap = scene->GetEntityMap();

		Entity entity = entityMap.at( entityID );

		*outTransform = entity.GetComponent<TransformComponent>();
	}

	void Saturn_TransformComponent_SetTransform( uint64_t entityID, TransformComponent* inTransform )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		auto& comp = ScriptHelpers::GetCompFromScene<TransformComponent>( scene, entityID );
		comp.GetTransform() = *inTransform;
	}

	void Saturn_TransformComponent_GetTranslation( uint64_t entityID, glm::vec3* outTranslation )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		const auto& entityMap = scene->GetEntityMap();

		Entity entity = entityMap.at( entityID );

		*outTranslation = entity.GetComponent<TransformComponent>().Position;
	}

	void Saturn_TransformComponent_SetTranslation( uint64_t entityID, glm::vec3* inTranslation )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		const auto& entityMap = scene->GetEntityMap();

		Entity entity = entityMap.at( entityID );

		entity.GetComponent<TransformComponent>().Position = *inTranslation;
	}

	void Saturn_TransformComponent_GetRotation( uint64_t entityID, glm::vec3* outRotation )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		const auto& entityMap = scene->GetEntityMap();

		Entity entity = entityMap.at( entityID );

		auto& rotation = entity.GetComponent<TransformComponent>().Rotation;

		glm::vec3 rot( rotation.x, rotation.y, rotation.z );

		*outRotation = rot;
	}

	void Saturn_TransformComponent_SetRotation( uint64_t entityID, glm::vec3* inRotation )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		const auto& entityMap = scene->GetEntityMap();

		Entity entity = entityMap.at( entityID );
		auto& rot = entity.GetComponent<TransformComponent>().Rotation; 
		rot = glm::quat( 0.0F, inRotation->x, inRotation->y, inRotation->z );
		SAT_CORE_INFO( "Rotation X{0}, Y{1}, Z{2}", rot.x, rot.y, rot.z );
	}

	void Saturn_TransformComponent_GetScale( uint64_t entityID, glm::vec3* outScale )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		const auto& entityMap = scene->GetEntityMap();

		Entity entity = entityMap.at( entityID );
		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	void Saturn_TransformComponent_SetScale( uint64_t entityID, glm::vec3* inScale )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();
		const auto& entityMap = scene->GetEntityMap();

		Entity entity = entityMap.at( entityID );
		entity.GetComponent<TransformComponent>().Scale = *inScale;
	}

	Mesh Saturn_Entity_GetMesh( uint64_t entityID )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<MeshComponent>( scene, entityID );
		return Mesh( comp.Mesh->GetFilePath() );
	}

	void Saturn_Entity_SetMesh( uint64_t entityID, void* type )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<MeshComponent>( scene, entityID );
		comp.Mesh = (Mesh*)type;
	}

	uint64_t Saturn_Entity_FindEntityByTag( MonoString* tag )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		Entity entity = scene->FindEntityByTag( mono_string_to_utf8( tag ) );
		if( entity.IsVaild() )
			return entity.GetComponent<IdComponent>().ID;

		return 0;
	}

	MonoString* Saturn_TagComponent_GetTag( uint64_t entityID )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<TagComponent>( scene, entityID );
		std::string tag = comp.Tag;
		return mono_string_new( mono_domain_get(), tag.c_str() );
	}

	void Saturn_TagComponent_SetTag( uint64_t entityID, MonoString* tag )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<TagComponent>( scene, entityID );
		std::string strtag = comp.Tag;
		comp.Tag = MonoToString( tag );
	}

	void Saturn_RigidBodyComponent_AddForce( uint64_t entityID, glm::vec3 forcedire, ForceType type )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<PhysXRigidbodyComponent>( scene, entityID );

		if( !comp.isKinematic )
			comp.m_Rigidbody->ApplyForce( forcedire, type );
		else
			SAT_CORE_WARN( "Cannot add a force to a kinematic actor!" ); return;
	}

	bool Saturn_Physics_Raycast( glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit )
	{
		return PhysXFnd::Raycast( *origin, *direction, maxDistance, hit );
	}

	void Saturn_RigidBodyComponent_GetLinearVelocity( uint64_t entityID, glm::vec3* outVelocity )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<PhysXRigidbodyComponent>( scene, entityID );

		*outVelocity = comp.m_Rigidbody->GetLinearVelocity();
	}

	void Saturn_RigidBodyComponent_SetLinearVelocity( uint64_t entityID, glm::vec3* velocity )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<PhysXRigidbodyComponent>( scene, entityID );

		comp.m_Rigidbody->SetLinearVelocity( *velocity );

	}

	void Saturn_RigidBodyComponent_Rotate( uint64_t entityID, glm::vec3* rotation )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<PhysXRigidbodyComponent>( scene, entityID );

		comp.m_Rigidbody->Rotate( *rotation );
	}

	void Saturn_RigidBodyComponent_ApplyForce( uint64_t entityID, glm::vec3* inForceDir, ForceType type )
	{
		Ref<Scene> scene = ScriptEngine::GetScene();

		auto& comp = ScriptHelpers::GetCompFromScene<PhysXRigidbodyComponent>( scene, entityID );

		comp.m_Rigidbody->ApplyForce( *inForceDir, type );
	}

	void Saturn_TimeStep_GetTimeStep( float* outTime )
	{
		*outTime = Application::Get().GetTimeStep();
	}

	void Saturn_TransformComponent_GetRotationX( uint64_t entityID, glm::vec3* outX )
	{

	}

	void Saturn_TransformComponent_GetRotationY( uint64_t entityID, glm::vec3* outY )
	{

	}

	void Saturn_TransformComponent_GetRotationZ( uint64_t entityID, glm::vec3* outZ )
	{

	}

}
