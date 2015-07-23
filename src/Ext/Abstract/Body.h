#pragma once

#include <CCINIClass.h>
#include <AbstractClass.h>

#include "../_Container.hpp"
#include "../../Ares.h"

#include "../../Misc/Debug.h"

class AbstractExt
{
public:
	using base_type = AbstractClass;

	class ExtData final : public Extension<AbstractClass>
	{
	public:

		DWORD LastChecksumTime;
		DWORD LastChecksum;

		ExtData(AbstractClass* OwnerObject) : Extension<AbstractClass>(OwnerObject),
			LastChecksumTime(0),
			LastChecksum(0)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AbstractExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
