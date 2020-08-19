#pragma once

#include "BaseSerializer.h"

namespace Sparky {

	class Serializer : public BaseSerializer
	{
	public:
		Serializer();
		~Serializer();

		virtual void Init() override;

		virtual void OnSerialize() override;
		virtual void OnDeSerialize() override;
		virtual void OnEndSerialize() override;
		virtual void OnEndDeSerialize() override;
		void StartSerialize();
		
		

	private:

	};
}