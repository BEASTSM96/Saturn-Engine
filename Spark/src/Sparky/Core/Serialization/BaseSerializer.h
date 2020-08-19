#pragma once

namespace Sparky {

	class BaseSerializer
	{
	public:
		BaseSerializer();
		virtual ~BaseSerializer() = default;

		virtual void OnSerialize();
		virtual void OnDeSerialize();
		virtual void OnEndSerialize();
		virtual void OnEndDeSerialize();

		virtual void Init();
	private:

	};
}
