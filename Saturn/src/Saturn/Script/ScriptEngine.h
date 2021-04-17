#pragma once

#include "Saturn/Core/Base.h"

#include "Saturn/Scene/Scene.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>
#include <string>

extern "C" {
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoClassField MonoClassField;
};

namespace Saturn {

	enum class FieldType
	{
		None = 0,
		Float,
		Int,
		UnsignedInt,
		String,
		Vec2,
		Vec3,
		Vec4
	};

	static FieldType MonoToSaturn( MonoType* monoType )
	{
		int type = mono_type_get_type( monoType );
		switch( type )
		{
			case MONO_TYPE_R4: return FieldType::Float;
			case MONO_TYPE_I4: return FieldType::Int;
			case MONO_TYPE_U4: return FieldType::UnsignedInt;
			case MONO_TYPE_STRING: return FieldType::String;
			case MONO_TYPE_VALUETYPE:
			{
				char* name = mono_type_get_name( monoType );
				if( strcmp( name, "Saturn.Vector2" ) == 0 ) return FieldType::Vec2;
				if( strcmp( name, "Saturn.Vector3" ) == 0 ) return FieldType::Vec3;
				if( strcmp( name, "Saturn.Vector4" ) == 0 ) return FieldType::Vec4;
			}
			default:
				return FieldType::None;
				break;
		}
	}

	struct MonoUtils
	{
	public:
		static MonoMethod* GetMethod( MonoImage* image, const std::string& desc );
		static MonoObject* CallMethod( MonoObject* obj, MonoMethod* method, void** parms = nullptr );

		static void PrintClassMethod( MonoClass* monoClass );
		static void PrintClassProps( MonoClass* monoClass );
	};

	struct EntityClass;

	struct EntityInstance
	{
		EntityClass* ScriptClass;

		uint32_t Handle;
		Scene* SceneInstance;

		MonoObject* Get()
		{
			return mono_gchandle_get_target( Handle );
		}
	};

	struct PublicField
	{
		std::string Name;
		FieldType Type;

		PublicField( const std::string& name, FieldType type )
			: Name( name ), Type( type )
		{
		}

		template<typename T>
		T GetValue() const
		{
			T value;
			GetValue_Internal( &value );
			return value;
		}

		template<typename T>
		void SetValue( T value ) const
		{
			SetValue_Internal( &value );
		}

	private:
		EntityInstance* m_EntityInstance;
		MonoClassField* m_MonoClassField;

		void GetValue_Internal( void* value ) const;
		void SetValue_Internal( void* outValue ) const;


		friend class ScriptEngine;
	};

	using FieldMap = std::unordered_map<std::string, std::vector<PublicField>>;

	class ScriptEngine
	{
	public:
		ScriptEngine( const Ref<Scene>& scene );
		~ScriptEngine();

		static void Init( const std::string& path );
		static void Shutdown();

		static void OnCreateEntity( Entity entity );
		static void OnEntityBeginPlay( Entity entity );
		static void OnUpdateEntity( Entity entity, Timestep ts );
		static void OnInitEntity( Entity entity );
		static void OnCollisionBegin( Entity entity );
		static void OnCollisionExit( Entity entity );
		static void SetSceneContext( const Ref<Scene>& scene );
		static bool ModuleExists( const std::string& moduleName );
		static EntityInstance& GetEntityInstanceData( UUID entityId );
		static Ref<Scene>& GetScene();


		static const FieldMap& GetFieldMap();
	};
}