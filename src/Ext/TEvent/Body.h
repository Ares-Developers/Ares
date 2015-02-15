#pragma once

#include "../_Container.hpp"
#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"

#include <Helpers/Template.h>

#include <TEventClass.h>

class TechnoTypeClass;

class TEventExt
{
public:
	using base_type = TEventClass;

	class ExtData : public Extension<TEventClass>
	{
	public:
		OptionalStruct<TechnoTypeClass*> TechnoType;

		ExtData(TEventClass* OwnerObject) : Extension<TEventClass>(OwnerObject),
			TechnoType()
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
			AnnounceInvalidPointer(TechnoType, ptr);
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		// support
		TechnoTypeClass* GetTechnoType();

		// handling events
		bool TechTypeExists();
		bool TechTypeDoesNotExist();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	static bool HasOccured(TEventClass* pEvent, bool* ret);

	static Container<TEventExt> ExtMap;
};
