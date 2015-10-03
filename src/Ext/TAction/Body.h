#pragma once

#include "../_Container.hpp"
#include "../../Utilities/Template.h"

#include <Helpers/Template.h>

#include <TActionClass.h>

class HouseClass;

class TActionExt
{
public:
	using base_type = TActionClass;

	class ExtData final : public Extension<TActionClass>
	{
	public:
		ExtData(TActionClass* const OwnerObject) : Extension<TActionClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		// executing actions
		static bool ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);
		static bool DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location);

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TActionExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool Execute(
		TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject,
		TriggerClass* pTrigger, CellStruct const& location, bool* ret);
};
