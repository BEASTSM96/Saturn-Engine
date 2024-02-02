/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2024 BEAST                                                           *
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

#pragma once

#include "AssetRegistryBase.h"

#include <unordered_map>
#include <unordered_set>

namespace Saturn {

	class AssetRegistry : public AssetRegistryBase
	{
	public:
		AssetRegistry();
		AssetRegistry( const AssetRegistry& rOther );

		~AssetRegistry();

		virtual AssetID CreateAsset( AssetType type ) override;
		virtual Ref<Asset> FindAsset( AssetID id ) override;

		Ref<Asset> FindAsset( const std::filesystem::path& rPath );
		Ref<Asset> FindAsset( const std::string& rName, AssetType type );

		std::vector<AssetID> FindAssetsWithType( AssetType type ) const;

		AssetID PathToID( const std::filesystem::path& rPath );

		void RemoveAsset( AssetID id );
		void TerminateAsset( AssetID id );

		bool DoesIDExists( AssetID id );

		size_t GetSize();

	private:
		void AddAsset( AssetID id );
	private:
		friend class AssetRegistrySerialiser;
		friend class AssetManager;
		friend class AssetBundle;
	};
}