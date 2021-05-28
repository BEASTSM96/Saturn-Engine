#include "sppch.h"
#include "PNGFile.h"

namespace Saturn {

	PNGFile::PNGFile() : File()
	{
	}

	void PNGFile::Init( std::string name, std::string filepath, FileExtensionType type )
	{
		SetUUID( UUID() );
		SetName( name );
		SetFilepath( filepath );
		SetFileExtensionType( type );

		m_Data = Texture2D::Create( filepath );
	}
}