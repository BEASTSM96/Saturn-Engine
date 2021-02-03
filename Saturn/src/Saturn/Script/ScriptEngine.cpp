#include "sppch.h"
#include "ScriptEngine.h"

#include "Saturn/Script/ScriptRegistry.h"

#include <mono/jit/jit.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/assembly.h>

namespace Saturn {

	static bool s_Init = false;
	static MonoDomain* s_MonoDomain = nullptr;
	static std::string s_AssemblyPath;

	static FieldMap s_PublicFields;

	static std::unordered_map<std::string, EntityClass> s_EntityClass;
	static std::unordered_map<uint32_t, EntityInstance> s_EntityInstanceMap;

	static Ref<Scene> m_Scene;

	MonoAssembly* LoadAssemblyFromFile( const char* filepath )
	{
		if( filepath == NULL )
		{
			return NULL;
		}

		HANDLE file = CreateFileA( filepath, FILE_READ_ACCESS, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( file == INVALID_HANDLE_VALUE )
		{
			return NULL;
		}

		DWORD file_size = GetFileSize( file, NULL );
		if( file_size == INVALID_FILE_SIZE )
		{
			CloseHandle( file );
			return NULL;
		}

		void* file_data = malloc( file_size );
		if( file_data == NULL )
		{
			CloseHandle( file );
			return NULL;
		}

		DWORD read = 0;
		ReadFile( file, file_data, file_size, &read, NULL );
		if( file_size != read )
		{
			free( file_data );
			CloseHandle( file );
			return NULL;
		}

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full( reinterpret_cast< char* >( file_data ), file_size, 1, &status, 0 );
		if( status != MONO_IMAGE_OK )
		{
			return NULL;
		}
		auto assemb = mono_assembly_load_from_full( image, filepath, &status, 0 );
		free( file_data );
		CloseHandle( file );
		mono_image_close( image );
		return assemb;
	}

	static MonoAssembly* LoadAssembly( const std::string& path )
	{
		MonoAssembly* assembly = LoadAssemblyFromFile( path.c_str() );

		if( !assembly )
			SAT_CORE_ERROR( "Failed to load assembly: {0}", path );
		else
			SAT_CORE_INFO( "Loaded assembly: {0}", path );

		return assembly;
	}

	static MonoImage* GetAssemblyImage( MonoAssembly* assembly )
	{
		MonoImage* image = mono_assembly_get_image( assembly );
		if( !image )
			SAT_CORE_ERROR( "mono_assembly_get_image failed" );

		return image;
	}

	static MonoClass* GetClass( MonoImage* image, const EntityClass& entityClass )
	{
		MonoClass* monoClass = mono_class_from_name( image, entityClass.NamespaceName.c_str(), entityClass.ClassName.c_str() );
		if( !monoClass )
			SAT_CORE_ERROR( "mono_class_from_name failed" );

		return monoClass;
	}

	static uint32_t Instantiate( EntityClass& entityClass )
	{
		MonoObject* instance = mono_object_new( s_MonoDomain, entityClass.Class );
		if(!instance)
			SAT_CORE_ERROR( "mono_object_new failed" );
		
		mono_runtime_object_init( instance );
		uint32_t handle = mono_gchandle_new( instance, false );
		return handle;
	}

	static MonoAssembly* s_AppAssembly = nullptr;
	static MonoAssembly* s_CoreAssembly = nullptr;
	MonoImage* s_AppAssemblyImage = nullptr;
	MonoImage* s_CoreAssemblyImage = nullptr;

	ScriptEngine::ScriptEngine( const Ref<Scene>& scene )
	{
		m_Scene = scene;
	}

	ScriptEngine::~ScriptEngine()
	{
	}

	void ScriptEngine::Init( const std::string& path )
	{
		// =========================== |
		// ----> INTI MONO			   |
		// =========================== |

		s_Init = true;
		s_AssemblyPath = path;

		mono_set_dirs( "C:\\Program Files\\Mono\\lib", "C:\\Program Files\\Mono\\etc" );
		mono_set_assemblies_path( "../Saturn/vendor/mono/lib" );
		mono_jit_set_trace_options( "--verbose" );

		auto domain = mono_jit_init( "Saturn" );

		char* name = ( char* )"Saturn_Runtime";
		s_MonoDomain = mono_domain_create_appdomain( name, nullptr );

		// =========================== |
		// ----> LOAD RUNTIME ASSEMBLY | 
		// =========================== |

		if(s_AppAssembly)
		{
			mono_domain_unload( s_MonoDomain );
			mono_assembly_close( s_AppAssembly );

			char* name = ( char* )"Saturn_Runtime-Runtime";
			s_MonoDomain = mono_domain_create_appdomain( name, nullptr );
		}

		s_CoreAssembly = LoadAssembly( "assets/assembly/SaturnRuntime.dll" );
		s_CoreAssemblyImage = GetAssemblyImage( s_CoreAssembly );

		s_AppAssembly = LoadAssembly( path );
		s_AppAssemblyImage = GetAssemblyImage( s_AppAssembly );
		ScriptRegistry::RegisterAll();
	}

	void ScriptEngine::Shutdown()
	{

	}

	void ScriptEngine::OnCreateEntity( Entity entity )
	{
		if( !s_Init )
			return;
		
		auto& entityInstance = GetEntityInstanceData(entity.GetComponent<IdComponent>().ID);
		if( entityInstance.ScrtptClass->MethodOnCreate )
			MonoUtils::CallMethod(entityInstance.Get(), entityInstance.ScrtptClass->MethodOnCreate);
	}

	void ScriptEngine::OnEntityBeginPlay( Entity entity )
	{
		if( !s_Init )
			return;

		auto& entityInstance = GetEntityInstanceData( entity.GetComponent<IdComponent>().ID );
		if( entityInstance.ScrtptClass->MethodBeginPlay )
			MonoUtils::CallMethod( entityInstance.Get(), entityInstance.ScrtptClass->MethodBeginPlay );
	}

	void ScriptEngine::OnUpdateEntity( Entity entity, Timestep ts )
	{
		if( !s_Init )
			return;

		auto& entityInstance = GetEntityInstanceData( entity.GetComponent<IdComponent>().ID );
		if ( entityInstance.ScrtptClass->MethodOnUpdate )
		{
			void* args[] ={ &ts };
			MonoUtils::CallMethod( entityInstance.Get(), entityInstance.ScrtptClass->MethodOnUpdate, args );
		}
	}

	void ScriptEngine::OnInitEntity( Entity entity )
	{
		if( !s_Init )
			return;

		Scene* scene = entity.m_Scene;
		UUID id = entity.GetComponent<IdComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;
		EntityClass& entityClass = s_EntityClass[ moduleName ];

		if( moduleName == "" ) return;
		if ( !ModuleExists(moduleName) ) return;

		if ( moduleName != "" )
		{
			entityClass.FullName = moduleName;
			if( moduleName.find( '.' ) != std::string::npos )
			{
				entityClass.NamespaceName = moduleName.substr( 0, moduleName.find_first_of( '.' ) );
				entityClass.ClassName = moduleName.substr( moduleName.find_first_of( '.' ) + 1 );
			}
			else
			{
				entityClass.ClassName = moduleName;
			}


			entityClass.Class = GetClass( s_AppAssemblyImage, entityClass );
			entityClass.InitClassMethods( s_AppAssemblyImage );

			EntityInstance& entityInstance = s_EntityInstanceMap[ id ];
			entityInstance.ScrtptClass = &entityClass;
			entityInstance.Handle = Instantiate( entityClass );
			{
				MonoClassField* it;
				void* ptr = 0;
				while( ( it = mono_class_get_fields( entityClass.Class, &ptr ) ) != NULL )
				{
					const char* name = mono_field_get_name( it );
					uint32_t flags = mono_field_get_flags( it );
					if( flags & MONO_FIELD_ATTR_PUBLIC == 0 )
						continue;

					MonoType* fieldtype = mono_field_get_type( it );
					FieldType fieldType = MonoToSaturn( fieldtype );

					auto& publicField = s_PublicFields[ moduleName ].emplace_back( name, fieldType );
					publicField.m_EntityInstance = &entityInstance;
					publicField.m_MonoClassField = it;

				}
			}
		}
	}

	void ScriptEngine::SetSceneContext( const Ref<Scene>& scene )
	{
		m_Scene = scene;
	}

	bool ScriptEngine::ModuleExists( const std::string& moduleName )
	{
		std::string NamespaceName, ClassName;
		if( moduleName.find( '.' ) != std::string::npos )
		{
			NamespaceName = moduleName.substr( 0, moduleName.find_first_of( '.' ) );
			ClassName = moduleName.substr( moduleName.find_first_of( '.' ) + 1 );
		}
		else
			ClassName = moduleName;

		MonoClass* monoClass = mono_class_from_name( s_AppAssemblyImage, NamespaceName.c_str(), ClassName.c_str() );
		return monoClass != nullptr;
	}

	EntityInstance& ScriptEngine::GetEntityInstanceData( UUID entityId )
	{
		return s_EntityInstanceMap.at(entityId);
	}

	Saturn::Ref<Saturn::Scene>& ScriptEngine::GetScene()
	{
		return m_Scene;
	}

	const Saturn::FieldMap& ScriptEngine::GetFieldMap()
	{
		return s_PublicFields;
	}

	void PublicField::GetValue_Internal( void* value ) const
	{
		mono_field_get_value( m_EntityInstance->Get(), m_MonoClassField, value );
	}

	void PublicField::SetValue_Internal( void* outValue ) const
	{
		mono_field_set_value( m_EntityInstance->Get(), m_MonoClassField, outValue );
	}

	MonoMethod* MonoUtils::GetMethod( MonoImage* image, const std::string& desc )
	{
		MonoMethodDesc* monoDecs = mono_method_desc_new( desc.c_str(), NULL );
		if( !monoDecs )
			SAT_CORE_ERROR("[Mono Runtime API] mono_method_desc_new failed ");

		MonoMethod* method = mono_method_desc_search_in_image( monoDecs, image );
		if( !method )
			SAT_CORE_ERROR( "[Mono Runtime API] mono_method_desc_search_in_image failed " );

		return method;
	}

	MonoObject* MonoUtils::CallMethod( MonoObject* obj, MonoMethod* method, void** parms /*= nullptr */ )
	{
		MonoObject* exception = NULL;
		MonoObject* result = mono_runtime_invoke( method, obj, parms, &exception );
		return result;
	}

	void MonoUtils::PrintClassMethod( MonoClass* monoClass ) {}
	void MonoUtils::PrintClassProps( MonoClass* monoClass ) {}

}
