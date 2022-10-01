/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2022 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#include <sppch.h>
#include "AssetRegistry.h"

namespace Saturn {

	AssetRegistry::AssetRegistry()
	{

	}

	AssetRegistry::~AssetRegistry()
	{

	}

	AssetID AssetRegistry::CreateAsset( AssetType type )
	{
		Ref<Asset> asset = Ref<Asset>::Create();
		asset->SetAssetType( type );

		m_Assets[ asset->GetAssetID() ] = asset;

		return asset->GetAssetID();
	}

	Ref<Asset> AssetRegistry::FindAsset( AssetID id )
	{
		return m_Assets[ id ];
	}

	Ref<Asset> AssetRegistry::FindAsset( const std::filesystem::path& rPath )
	{
		for ( const auto& [id, asset] : m_Assets )
		{
			if( asset->GetPath() == rPath )
				return asset;
		}

		return nullptr;
	}

	void AssetRegistry::SaveAssetRegistry()
	{

	}

	void AssetRegistry::AddAsset( AssetID id )
	{
		SAT_CORE_ASSERT( m_Assets.find( id ) == m_Assets.end(), "Asset already exists!" );

		m_Assets[ id ] = Ref<Asset>::Create();
	}

}