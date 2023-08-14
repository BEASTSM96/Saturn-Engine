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
		float GetEmissive();

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
		void SetEmissive( float val );

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
		
		//void Bind( const Ref< StaticMesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader, const VkWriteDescriptorSet& rStorageBufferWDS );

		// Updates Uniform buffers, texture and storage buffers.
		void Bind( const Ref< StaticMesh >& rMesh, Submesh& rSubmsh, Ref< Shader >& Shader, const VkWriteDescriptorSet& rStorageBufferWDS = VkWriteDescriptorSet{} );

		void RT_Bind( const std::vector<std::vector<VkWriteDescriptorSet>>& rStorageBufferWDS = std::vector<std::vector<VkWriteDescriptorSet>>() );

		void Clean();

		Buffer GetPushConstantData() { return m_Material->m_PushConstantData; }

		bool IsInViewingMode();

		void ApplyChanges();

		Ref<Material> GetMaterial() const { return m_Material; }

		void SetMaterial( const Ref<Material>& rMaterial );

		void SetName( const std::string& rName ) { return m_Material->SetName( rName ); }

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

		Ref<Material> m_PendingMaterialChange = nullptr;

		std::unordered_map< std::string, Ref<Texture2D> > m_PendingTextureChanges;
		std::unordered_map< uint32_t, std::filesystem::path > m_VPendingTextureChanges;
	private:
		friend class MaterialAssetViewer;
	};

	class MaterialRegistry : public RefTarget
	{
	public:
		MaterialRegistry();
		// TODO: When we have animated meshes this will need to be re-worked.
		MaterialRegistry( const Ref<StaticMesh>& mesh );

		~MaterialRegistry();

		void Copy( const Ref<MaterialRegistry>& rSrc );

		void AddAsset( uint32_t index );
		void AddAsset( const Ref<MaterialAsset>& rAsset );

		Ref<MaterialAsset> GetAsset( AssetID id );

		void SetMaterial( uint32_t index, AssetID id );
		void ResetMaterial( uint32_t index );
	
		std::vector< Ref<MaterialAsset> >& GetMaterials() { return m_Materials; }
		const std::vector< Ref<MaterialAsset> >& GetMaterials() const { return m_Materials; }

		UUID GetID() { return m_ID; }
		const UUID GetID() const { return m_ID; }

		bool HasOverrides() const { return m_HasOverrides; }
		void SetOverries( bool val ) { m_HasOverrides = val; }

	private:
		//std::unordered_map< AssetID, Ref<MaterialAsset> > m_Materials;
		Ref<StaticMesh> m_Mesh = nullptr;
		std::vector< Ref<MaterialAsset> > m_Materials;

		bool m_HasOverrides = false;

		UUID m_ID;
	private:
		friend class MaterialAsset;
		friend class StaticMesh;
	};
}