#ifndef WARHEADTYPE_EXT_H
#define WARHEADTYPE_EXT_H

#include <xcompile.h>
#include <CCINIClass.h>
#include <WarheadTypeClass.h>
#include <GeneralStructures.h>

#include <Conversions.h>

#include "../../Misc/AttachEffect.h"

#include "../_Container.hpp"

#include "../../Utilities/Constructs.h"
#include "../../Utilities/Template.h"

#ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
#endif

class AnimTypeClass;
class BulletClass;
class HouseClass;
class IonBlastClass;
class TechnoClass;

class WarheadTypeExt //: public Container<WarheadTypeExt>
{
public:
	typedef WarheadTypeClass TT;

	struct VersesData : public WarheadFlags {
		double Verses;

		VersesData(double VS = 1.0, bool FF = true, bool Retal = true, bool Acquire = true): Verses(VS), WarheadFlags(FF, Retal, Acquire) {};

		bool operator ==(const VersesData &RHS) const {
			return (CLOSE_ENOUGH(this->Verses, RHS.Verses));
		};

		void Parse(const char *str) {
			this->Verses = Conversions::Str2Armor(str, this);
		}
	};

	class ExtData : public Extension<TT>
	{
	public:
		bool MindControl_Permanent;

		int Ripple_Radius;

		int EMP_Duration;
		int EMP_Cap;

		int IC_Duration;
		int IC_Cap;

		DynamicVectorClass<VersesData> Verses;
		double DeployedDamage;

		Nullable<AnimTypeClass *> Temporal_WarpAway;

		bool AffectsEnemies; // request #397

		Valueable<AnimTypeClass*> InfDeathAnim;

		ValueableIdx<AnimTypeClass> PreImpactAnim;

		bool KillDriver; //!< Whether this warhead turns the target vehicle over to the special side ("kills the driver"). Request #733.

		Valueable<double> KillDriver_KillBelowPercent;

		Valueable<bool> Malicious;

		Valueable<bool> PreventScatter;

		Valueable<int> CellSpread_MaxAffect;

		AttachEffectTypeClass AttachedEffect;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			MindControl_Permanent (false),
			Ripple_Radius (0),
			EMP_Duration (0),
			EMP_Cap (-1),
			IC_Duration (0),
			IC_Cap (-1),
			DeployedDamage (1.00),
			Temporal_WarpAway (),
			AffectsEnemies (true),
			InfDeathAnim (nullptr),
			PreImpactAnim (-1),
			KillDriver (false),
			KillDriver_KillBelowPercent(1.00),
			Malicious (true),
			PreventScatter (false),
			CellSpread_MaxAffect (-1),
			AttachedEffect(OwnerObject)
			{
				for(int i = 0; i < 11; ++i) {
					VersesData vs;
					Verses.AddItem(vs);
				}
			};

		virtual ~ExtData() { };

		virtual void Initialize(TT *pThis);

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}

		void applyRipples(const CoordStruct &coords);
		void applyIronCurtain(CoordStruct *, HouseClass *, int);
		void applyEMP(CoordStruct *, TechnoClass *);
		bool applyPermaMC(CoordStruct *, HouseClass *, AbstractClass *);

		void applyAttachedEffect(CoordStruct *, TechnoClass *);

		bool applyKillDriver(BulletClass *); // #733
	};

	static Container<WarheadTypeExt> ExtMap;

	static WarheadTypeClass *Temporal_WH;

	static WarheadTypeClass *EMP_WH;

	static AresMap<IonBlastClass*, const WarheadTypeExt::ExtData*> IonExt;

	static void applyRipples(WarheadTypeClass * pWH, const CoordStruct &coords) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyRipples(coords);
		}
	}
	static void applyIronCurtain(WarheadTypeClass * pWH, CoordStruct* coords, HouseClass * House, int damage) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyIronCurtain(coords, House, damage);
		}
	}
	static void applyEMP(WarheadTypeClass * pWH, CoordStruct* coords, TechnoClass *source) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyEMP(coords, source);
		}
	}
	static bool applyPermaMC(WarheadTypeClass * pWH, CoordStruct* coords, HouseClass * House, AbstractClass * Source) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyPermaMC(coords, House, Source);
		}
	}
	static void applyOccupantDamage(BulletClass *);

    static bool canWarheadAffectTarget(TechnoClass *, HouseClass *, WarheadTypeClass *);

	static void applyAttachedEffect(WarheadTypeClass * pWH, CoordStruct* coords, TechnoClass * Source) {
	//static void applyAttachedEffect(WarheadTypeClass * pWH, CoordStruct* coords, HouseClass* Owner) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyAttachedEffect(coords, Source);
		//	pWHExt->applyAttachedEffect(coords, Owner);
		}
	}
};

#endif
