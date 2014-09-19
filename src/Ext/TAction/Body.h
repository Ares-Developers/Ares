#ifndef TACTION_EXT_H
#define TACTION_EXT_H

#include "../_Container.hpp"
#include "../../Utilities/Template.h"

#include <Helpers/Template.h>

#include <TActionClass.h>

class HouseClass;

class TActionExt
{
	public:
	typedef TActionClass TT;

	class ExtData : public Extension<TT>
	{
		public:
		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject)
		{
		};

		virtual ~ExtData() {
		}

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

		// executing actions
		static bool ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos);
		static bool DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos);
	};

	static bool Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos, bool* ret);

	static Container<TActionExt> ExtMap;
};

#endif
