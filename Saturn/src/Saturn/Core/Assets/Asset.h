#pragma once

#include "Saturn/Core/Base.h"

namespace Saturn {

	//TODO: Add more stuff
	class AssetBase
	{
	public:
		AssetBase();
		~AssetBase();

	private:

	};

	class Asset : public AssetBase
	{
	public:
		Asset();
		~Asset();

		void CreateGameObjectAsset();

		template<typename T, typename... Args>
		T CreateAssetFrom(Args&&... args)
		{
			//Compile time
			static_assert(std::is_base_of<Asset, T>::value, "Class is not Asset!");
			static_assert(std::is_base_of<AssetBase, T>::value, "Class is not AssetBase!");

			//Editor time
			SAT_CORE_ASSERT(std::is_base_of<Asset, T>::value, "Class is not Asset!");
			SAT_CORE_ASSERT(std::is_base_of<AssetBase, T>::value, "Class is not AssetBase!");

			return T(new T(std::forward<Args>(args)...));
		}

	private:

	};
}