#ifndef TIBERIUM_EXT_H
#define TIBERIUM_EXT_H

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

		Nullable<WarheadTypeClass*> ExplosionWarhead;
		Nullable<int> ExplosionDamage;

		Valueable<int> DebrisChance;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			Damage(),
			Warhead(),
			Heal_Step(),
			Heal_IStep(),
			Heal_UStep(),
			Heal_Delay(),
			ExplosionWarhead(),
			ExplosionDamage(),
			DebrisChance(33)
		{
		};

		virtual ~ExtData() {

		}

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

		double GetHealDelay() const;
		int GetHealStep(TechnoClass* pTechno) const;
		int GetDamage() const;
		WarheadTypeClass* GetWarhead() const;
		WarheadTypeClass* GetExplosionWarhead() const;
		int GetExplosionDamage() const;
		int GetDebrisChance() const;
	};

	static Container<TiberiumExt> ExtMap;
};

#endif
