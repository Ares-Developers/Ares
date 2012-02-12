#ifndef WARHEADTYPE_EXT_H
#define WARHEADTYPE_EXT_H

#include <xcompile.h>
#include <AnimClass.h>
#include <IonBlastClass.h>
#include <AnimTypeClass.h>
#include <BulletClass.h>
#include <CaptureManagerClass.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <EMPulseClass.h>
#include <MapClass.h>
#include <TechnoClass.h>
#include <WarheadTypeClass.h>
#include <HouseClass.h>
#include <ObjectClass.h>
#include <GeneralStructures.h>

#include <Conversions.h>

#include "../_Container.hpp"

#include "../../Utilities/Template.h"

#ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
#endif

//class WarheadTypeExt;
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

		Customizable<AnimTypeClass *> Temporal_WarpAway;

		bool AffectsEnemies; // request #397

		Valueable<AnimTypeClass*> InfDeathAnim;

		ValueableIdx<int, AnimTypeClass> PreImpactAnim;

		bool KillDriver; //!< Whether this warhead turns the target vehicle over to the special side ("kills the driver"). Request #733.

		Valueable<bool> Malicious;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			MindControl_Permanent (false),
			Ripple_Radius (0),
			EMP_Duration (0),
			EMP_Cap (-1),
			IC_Duration (0),
			IC_Cap (-1),
			DeployedDamage (1.00),
			Temporal_WarpAway (&RulesClass::Global()->WarpAway),
			AffectsEnemies (true),
			InfDeathAnim (NULL),
			PreImpactAnim (-1),
			KillDriver (false),
			Malicious (true)
			{
				for(int i = 0; i < 11; ++i) {
					VersesData vs;
					Verses.AddItem(vs);
				}
			};

		virtual ~ExtData() { };

		virtual size_t Size() const { return sizeof(*this); };

		virtual void Initialize(TT *pThis);

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);

		virtual void InvalidatePointer(void *ptr) {
		}

		void applyRipples(CoordStruct *);
		void applyIronCurtain(CoordStruct *, HouseClass *, int);
		void applyEMP(CoordStruct *, TechnoClass *);
		bool applyPermaMC(CoordStruct *, HouseClass *, ObjectClass *);

		bool applyKillDriver(BulletClass *); // #733
	};

	static Container<WarheadTypeExt> ExtMap;

	static WarheadTypeClass *Temporal_WH;

	static WarheadTypeClass *EMP_WH;

	static hash_map<IonBlastClass *, WarheadTypeExt::ExtData *> IonExt;

	static void applyRipples(WarheadTypeClass * pWH, CoordStruct* coords) {
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
	static bool applyPermaMC(WarheadTypeClass * pWH, CoordStruct* coords, HouseClass * House, ObjectClass * Source) {
		if(auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH)) {
			pWHExt->applyPermaMC(coords, House, Source);
		}
	}
	static void applyOccupantDamage(BulletClass *);

    static bool canWarheadAffectTarget(TechnoClass *, HouseClass *, WarheadTypeClass *);
};

typedef hash_map<IonBlastClass *, WarheadTypeExt::ExtData *> hash_ionExt;
#endif
