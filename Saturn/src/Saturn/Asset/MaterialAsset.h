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

#pragma once

#include "Saturn/Vulkan/Material.h"

#include "Asset.h"

namespace Saturn {

	class MaterialAsset : public Asset
	{
	public:
		MaterialAsset( Ref<Material> material );
		~MaterialAsset();

		// Texture
		Ref<Texture2D> GetAlbeoMap();
		Ref<Texture2D> GetNormalMap();
		Ref<Texture2D> GetMetallicMap();
		Ref<Texture2D> GetRoughnessMap();

		// Colors and values

		glm::vec3 GetAlbeoColor();
		float IsUsingNormalMap();
		float GetRoughness();
		float GetMetalness();

		std::string& GetName() { return m_Material->GetName(); }
		const std::string& GetName() const { return m_Material->GetName(); }

		void SetAlbeoMap( Ref<Texture2D>& rTexture );
		void SetNormalMap( Ref<Texture2D>& rTexture );
		void SetMetallicMap( Ref<Texture2D>& rTexture );
		void SetRoughnessMap( Ref<Texture2D>& rTexture );

		void SetAlbeoColor( glm::vec3 color );
		void UseNormalMap( bool val );
		void SetRoughness( float val );
		void SetMetalness( float val );

		Ref<Texture2D> GetResource( const std::string& rName );
		void SetResource( const std::string& rName, const Ref<Texture2D>& rTexture );

		template<typename Ty>
		Ty& Get( const std::string& rName ) 
		{
			return m_Material->Get< Ty >( rName );
		}

		template<typename Ty>
		void Set( const std::string& rName, const Ty& rValue )
		{
			m_ValuesChanged = true;

			m_Material->Set( rName, rValue );
		}

		void Bind( const Ref< Mesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader );

		Buffer GetPushConstantData() { return m_Material->m_PushConstantData; }

		bool IsInViewingMode();

		void ApplyChanges();

		Ref<Material> GetMaterial() const { return m_Material; }

		void SetMaterial( const Ref<Material>& rMaterial ) { m_Material = nullptr; m_Material = rMaterial; }

	private:

		void Default();

		// Only used for material asset viewer. As we need an internal material to apply the values.
		void BeginViewingSession();
		void SaveViewingSession();
		void EndViewingSession();

		void SetAlbeoMap( const std::filesystem::path& rPath );
		void SetNormalMap( const std::filesystem::path& rPath );
		void SetMetallicMap( const std::filesystem::path& rPath );
		void SetRoughnessMap( const std::filesystem::path& rPath );

	private:
		Ref<Material> m_Material = nullptr;

		bool m_ValuesChanged = false;

		std::unordered_map< std::string, VkDescriptorImageInfo > m_TextureCache;

	private:
		friend class MaterialAssetViewer;
	};

}