#ifndef TEVENT_EXT_H
#define TEVENT_EXT_H

#include "../_Container.hpp"
#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"

#include <Helpers/Template.h>

#include <TEventClass.h>

class TechnoTypeClass;

class TEventExt
{
	public:
	typedef TEventClass TT;

	class ExtData : public Extension<TT>
	{
		public:
			OptionalStruct<TechnoTypeClass*> TechnoType;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			TechnoType()
		{
		};

		virtual ~ExtData() {
		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI) {}

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
			AnnounceInvalidPointer(TechnoType, ptr);
		}

		// support
		TechnoTypeClass* GetTechnoType();

		// handling events
		bool TechTypeExists();
		bool TechTypeDoesNotExist();
	};

	static bool HasOccured(TEventClass* pEvent, bool* ret);

	static Container<TEventExt> ExtMap;
};

#endif
