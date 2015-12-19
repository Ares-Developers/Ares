#pragma once

#include <CCINIClass.h>
#include <WarheadTypeClass.h>
#include <GeneralStructures.h>

#include <Conversions.h>

#include "../../Misc/AttachEffect.h"

#include "../_Container.hpp"

#include "../../Utilities/Constructs.h"
#include "../../Utilities/Enums.h"
#include "../../Utilities/Template.h"

#ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
#endif

class AnimTypeClass;
class BulletClass;
class HouseClass;
class IonBlastClass;
class TechnoClass;

class WarheadTypeExt
{
public:
	using base_type = WarheadTypeClass;

	struct VersesData : public WarheadFlags {
		double Verses;

		VersesData(double VS = 1.0, bool FF = true, bool Retal = true, bool Acquire = true) : Verses(VS), WarheadFlags(FF, Retal, Acquire) {};

		bool operator ==(const VersesData &RHS) const {
			return (CLOSE_ENOUGH(this->Verses, RHS.Verses));
		}

		void Parse(const char *str) {
			this->Verses = Conversions::Str2Armor(str, this);
		}
	};

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:
		bool MindControl_Permanent;

		int Ripple_Radius;

		int EMP_Duration;
		int EMP_Cap;
		Valueable<AnimTypeClass*> EMP_Sparkles;

		int IC_Duration;
		int IC_Cap;

		DynamicVectorClass<VersesData> Verses;
		double DeployedDamage;

		Nullable<AnimTypeClass *> Temporal_WarpAway;

		bool AffectsEnemies; // request #397

		Valueable<AnimTypeClass*> InfDeathAnim;

		ValueableIdx<AnimTypeClass> PreImpactAnim;
		Nullable<int> NukeFlashDuration;

		bool KillDriver; //!< Whether this warhead turns the target vehicle over to the special side ("kills the driver"). Request #733.

		Valueable<double> KillDriver_KillBelowPercent;

		Valueable<OwnerHouseKind> KillDriver_Owner;

		Valueable<bool> Malicious;

		Valueable<bool> PreventScatter;

		Valueable<int> CellSpread_MaxAffect;

		Valueable<int> DamageAirThreshold;

		AttachEffectTypeClass AttachedEffect;

		Valueable<bool> SuppressDeathWeapon_Vehicles;
		Valueable<bool> SuppressDeathWeapon_Infantry;
		ValueableVector<TechnoTypeClass*> SuppressDeathWeapon;

		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject),
			MindControl_Permanent(false),
			Ripple_Radius(0),
			EMP_Duration(0),
			EMP_Cap(-1),
			IC_Duration(0),
			IC_Cap(-1),
			DeployedDamage(1.00),
			Temporal_WarpAway(),
			AffectsEnemies(true),
			InfDeathAnim(nullptr),
			PreImpactAnim(-1),
			KillDriver(false),
			KillDriver_KillBelowPercent(1.00),
			KillDriver_Owner(OwnerHouseKind::Special),
			Malicious(true),
			PreventScatter(false),
			CellSpread_MaxAffect(-1),
			DamageAirThreshold(0),
			AttachedEffect(OwnerObject)
		{
			VersesData vs;
			for(int i = 0; i < 11; ++i) {
				Verses.AddItem(vs);
			}
		}

		virtual ~ExtData() = default;

		virtual void Initialize() override;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void *ptr, bool bRemoved) override {
		}

		virtual void LoadFromStream(AresStreamReader &Stm) override;

		virtual void SaveToStream(AresStreamWriter &Stm) override;

		void applyRipples(const CoordStruct &coords);
		void applyIronCurtain(const CoordStruct &coords, HouseClass* pOwner, int damage);
		void applyEMP(const CoordStruct &coords, TechnoClass* pSource);
		bool applyPermaMC(HouseClass* pOwner, AbstractClass* pTarget) const;

		void applyAttachedEffect(const CoordStruct &coords, TechnoClass* pOwner);

		bool applyKillDriver(TechnoClass* pSource, AbstractClass* pTarget) const; // #733

		VersesData& GetVerses(Armor armor) {
			return this->Verses[static_cast<int>(armor)];
		}

		const VersesData& GetVerses(Armor armor) const {
			return this->Verses[static_cast<int>(armor)];
		}

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt> {
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(AresStreamReader& Stm);
	static bool SaveGlobals(AresStreamWriter& Stm);

	static WarheadTypeClass *Temporal_WH;

	static WarheadTypeClass *EMP_WH;

	static AresMap<IonBlastClass*, const WarheadTypeExt::ExtData*> IonExt;

	static void applyRipples(WarheadTypeClass * pWH, const CoordStruct &coords) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyRipples(coords);
		}
	}
	static void applyIronCurtain(WarheadTypeClass * pWH, const CoordStruct &coords, HouseClass * House, int damage) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyIronCurtain(coords, House, damage);
		}
	}
	static void applyEMP(WarheadTypeClass * pWH, const CoordStruct &coords, TechnoClass *source) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyEMP(coords, source);
		}
	}
	static bool applyPermaMC(WarheadTypeClass* pWH, HouseClass* House, AbstractClass* Target) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyPermaMC(House, Target);
		}
	}
	static void applyOccupantDamage(BulletClass *);

	static bool CanAffectTarget(TechnoClass* pTarget, HouseClass* pSourceHouse, WarheadTypeClass* pWarhead);

	static void applyAttachedEffect(WarheadTypeClass * pWH, const CoordStruct &coords, TechnoClass * Source) {
		//static void applyAttachedEffect(WarheadTypeClass * pWH, CoordStruct* coords, HouseClass* Owner) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyAttachedEffect(coords, Source);
			//	pWHExt->applyAttachedEffect(coords, Owner);
		}
	}
};
