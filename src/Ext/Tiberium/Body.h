#pragma once

#include <TiberiumClass.h>

#include "../../Utilities/Template.h"

#include "../_Container.hpp"

class WarheadTypeClass;

class TiberiumExt
{
public:
	using base_type = TiberiumClass;

	class ExtData final : public Extension<TiberiumClass>
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

		ExtData(TiberiumClass* OwnerObject) : Extension<TiberiumClass>(OwnerObject),
			Damage(),
			Warhead(),
			Heal_Step(),
			Heal_IStep(),
			Heal_UStep(),
			Heal_Delay(),
			ExplosionWarhead(),
			ExplosionDamage(),
			DebrisChance(33)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		double GetHealDelay() const;
		int GetHealStep(TechnoClass* pTechno) const;
		int GetDamage() const;
		WarheadTypeClass* GetWarhead() const;
		WarheadTypeClass* GetExplosionWarhead() const;
		int GetExplosionDamage() const;
		int GetDebrisChance() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TiberiumExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
