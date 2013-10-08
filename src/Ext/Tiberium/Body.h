#ifndef TIBERIUM_EXT_H
#define TIBERIUM_EXT_H

#include <xcompile.h>
#include <TiberiumClass.h>

#include "../../Utilities/Template.h"

#include "../_Container.hpp"

class WarheadTypeClass;

class TiberiumExt
{
	public:
	typedef TiberiumClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		Nullable<int> Damage;
		Nullable<WarheadTypeClass*> Warhead;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			Damage(),
			Warhead()
		{
		};

		virtual ~ExtData() {

		}

		virtual size_t Size() const { return sizeof(*this); };

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void Initialize(TT *pThis);
		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

		double GetHealDelay();
		int GetHealStep(TechnoClass* pTechno);
	};

	static Container<TiberiumExt> ExtMap;
};

#endif
