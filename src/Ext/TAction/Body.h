#ifndef TACTION_EXT_H
#define TACTION_EXT_H

#include "../_Container.hpp"
#include "../../Utilities/Template.h"

#include <Helpers/Template.h>

#include <TActionClass.h>
#include <TechnoTypeClass.h>

class TActionExt
{
	public:
	typedef TActionClass TT;

	class ExtData : public Extension<TT>
	{
		public:
		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject)
		{
		};

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI) {}
		virtual void Initialize(TT *pThis);

		virtual void InvalidatePointer(void *ptr) {
		}

		// executing actions
		static bool ActivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos);
		static bool DeactivateFirestorm(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos);
	};

	static bool Execute(TActionClass* pAction, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct* pos, bool* ret);

	static Container<TActionExt> ExtMap;
};

#endif
