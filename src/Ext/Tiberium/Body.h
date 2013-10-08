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

		Nullable<int> Heal_Step;
		Nullable<int> Heal_IStep;
		Nullable<int> Heal_UStep;
		Nullable<double> Heal_Delay;

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			Damage(),
			Warhead(),
			Heal_Step(),
			Heal_IStep(),
			Heal_UStep(),
			Heal_Delay()
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
