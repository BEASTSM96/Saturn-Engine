#pragma once

namespace Saturn::Core {
	#define SAT_TYPEDEF(baseClass, newClassName) typedef baseClass newClassName

	template<typename T, class baseClass, class FrombaseClass>
	class Typedef
	{
	public:
		void New(baseClass, FrombaseClass)
		{
			SAT_TYPEDEF(baseClass, FrombaseClass);
		}
	};
}