#include "sppch.h"
#include "ScriptLoader.h"

namespace Saturn {

	Ref<Library> ScriptLoader::LoadDLL( std::string name, LPCWSTR path )
	{
		Ref<Library> library = Ref<Library>::Create();

		library->m_Library = LoadLibrary( path );
		library->SetName( name );
		library->m_Path = (const char*)path;

		if( library->m_Library == NULL )
			SAT_CORE_ERROR( "Failed to load path for : {0}", library->m_Path );
		else
			SAT_CORE_INFO( "Library: {0} loaded!", name );

		return library;
	}
}

