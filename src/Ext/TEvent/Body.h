#ifndef TEVENT_EXT_H
#define TEVENT_EXT_H

#include "../_Container.hpp"
#include "../../Utilities/Template.h"

#include <Helpers/Template.h>

#include <TEventClass.h>
#include <TechnoTypeClass.h>

class TEventExt
{
	public:
	typedef TEventClass TT;

	class ExtData : public Extension<TT>
	{
		public:
			Nullable<TechnoTypeClass*> TechnoType;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			TechnoType()
		{
		};

		virtual ~ExtData() {
		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI) {}
		virtual void Initialize(TT *pThis);

		virtual void InvalidatePointer(void *ptr) {
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
