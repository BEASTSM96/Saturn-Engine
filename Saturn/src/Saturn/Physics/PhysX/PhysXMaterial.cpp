#include "sppch.h"
#include "PhysXMaterial.h"

namespace Saturn {

	PhysXMaterial::PhysXMaterial( PhysXScene* scene, std::string name )
	{
		m_Scene = scene;
		m_Material = m_Scene->m_Physics->createMaterial( 0.5f, 0.5f, 0.6f );
	}

	PhysXMaterial::~PhysXMaterial()
	{
		if ( m_Material )
		{
			m_Material->release();
			m_Material = nullptr;
		}
	}

}
