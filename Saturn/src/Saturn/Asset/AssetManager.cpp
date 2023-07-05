/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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

#include "sppch.h"
#include "AssetManager.h"

#include "Saturn/Core/App.h"
#include "Saturn/Serialisation/AssetRegistrySerialiser.h"

namespace Saturn {

	AssetManager::AssetManager()
	{
		SingletonStorage::Get().AddSingleton( this );

		// Setup Asset Registries.
		auto project = Project::GetActiveProject();
		auto assetDir = project->GetFullAssetPath();
		assetDir /= "AssetRegistry.sreg";

		m_Assets = Ref<AssetRegistry>::Create();
		m_Assets->m_Path = assetDir;

		// Editor
		auto& contentDir = Application::Get().GetRootContentDir();
		contentDir /= "AssetRegistry.sreg";

		m_EditorAssets = Ref<AssetRegistry>::Create();
		m_EditorAssets->m_Path = contentDir;

		AssetRegistrySerialiser ars;
		ars.Deserialise( m_Assets );
		ars.Deserialise( m_EditorAssets );
	}

	AssetManager::~AssetManager()
	{
		m_Assets = nullptr;
		m_EditorAssets = nullptr;
	}

	Ref<Asset> AssetManager::FindAsset( AssetID id, AssetRegistryType Dst )
	{
		switch( Dst )
		{
			case AssetRegistryType::Game:
				return m_Assets->FindAsset( id );

			case AssetRegistryType::Editor: 
				return m_EditorAssets->FindAsset( id );
		
			case AssetRegistryType::Unknown:
			default:
				return nullptr;
		}

		return nullptr;
	}

	Ref<Asset> AssetManager::FindAsset( AssetID id )
	{
		// Search in Game assets then editor.
		Ref<Asset> result = nullptr;

		result = m_Assets->FindAsset( id );

		if( !result )
		{
			result = m_EditorAssets->FindAsset( id );
		}

		return result;
	}

	Ref<Asset> AssetManager::FindAsset( const std::filesystem::path& rPath, AssetRegistryType Dst /*= AssetRegistryType::Game */ )
	{
		switch( Dst )
		{
			case AssetRegistryType::Game:
				return m_Assets->FindAsset( rPath );

			case AssetRegistryType::Editor:
				return m_EditorAssets->FindAsset( rPath );

			case AssetRegistryType::Unknown:
			default:
				return nullptr;
		}

		return nullptr;
	}

	Ref<Asset> AssetManager::FindAsset( const std::string& rName, AssetType type, AssetRegistryType Dst /*= AssetRegistryType::Game */ )
	{
		switch( Dst )
		{
			case AssetRegistryType::Game:
				return m_Assets->FindAsset( rName, type );

			case AssetRegistryType::Editor:
				return m_EditorAssets->FindAsset( rName, type );

			case AssetRegistryType::Unknown:
			default:
				return nullptr;
		}

		return nullptr;
	}

	AssetID AssetManager::CreateAsset( AssetType type, AssetRegistryType Dst /*= AssetRegistryType::Game */ )
	{
		// TODO: Check if multiple asset ids?
		switch( Dst )
		{
			case AssetRegistryType::Game: 
				return m_Assets->CreateAsset( type );

			case AssetRegistryType::Editor:
				return m_EditorAssets->CreateAsset( type );

			case AssetRegistryType::Unknown:
			default:
				return 0;
		}

		return 0;
	}

	AssetMap AssetManager::GetCombinedAssetMap()
	{
		AssetMap map;

		map.insert( m_Assets->GetAssetMap().begin(), m_Assets->GetAssetMap().end() );
		map.insert( m_EditorAssets->GetAssetMap().begin(), m_EditorAssets->GetAssetMap().end() );

		return map;
	}

	bool AssetManager::IsAssetLoaded( AssetID id, AssetRegistryType Dst /*= AssetRegistryType::Game */ )
	{
		switch( Dst )
		{
			case AssetRegistryType::Game:
				return m_Assets->IsAssetLoaded( id );

			case AssetRegistryType::Editor:
				return m_EditorAssets->IsAssetLoaded( id );

			case AssetRegistryType::Unknown:
			default:
				return false;
		}

		return false;
	}

	AssetID AssetManager::PathToID( const std::filesystem::path& rPath, AssetRegistryType Dst /*= AssetRegistryType::Game */ )
	{
		switch( Dst )
		{
			case AssetRegistryType::Game:
				return m_Assets->PathToID( rPath );

			case AssetRegistryType::Editor:
				return m_EditorAssets->PathToID( rPath );

			case AssetRegistryType::Unknown:
			default:
				return 0;
		}
	}

	void AssetManager::Save( AssetRegistryType Dst /*= AssetRegistryType::Game */ )
	{
		AssetRegistrySerialiser ars;

		switch( Dst )
		{
			case AssetRegistryType::Game:
				ars.Serialise( m_Assets );

			case AssetRegistryType::Editor:
				ars.Serialise( m_EditorAssets );
		
			case AssetRegistryType::Unknown:
			default:
				break;
		}
	}

}