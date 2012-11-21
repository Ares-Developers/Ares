#ifndef RULES_EXT_H
#define RULES_EXT_H

#include <CCINIClass.h>
#include <WeaponTypeClass.h>
#include <RulesClass.h>
#include <AnimTypeClass.h>
#include <SidebarClass.h>
#include <VocClass.h>
#include <MouseClass.h>

#include "../_Container.hpp"
#include "../../Utilities/Template.h"

//ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
//endif

class RulesExt
{
	public:
	typedef RulesClass TT;

	class ExtData : public Extension<TT>
	{
		public:
		Valueable<AnimTypeClass* >ElectricDeath;
		double EngineerDamage;
		bool EngineerAlwaysCaptureTech;
		Valueable<MouseCursor> EngineerDamageCursor;
		bool MultiEngineer[3];

		Valueable<bool> CanMakeStuffUp;

		Valueable<bool> Tiberium_DamageEnabled;
		Valueable<bool> Tiberium_HealEnabled;
		Valueable<WarheadTypeClass*> Tiberium_ExplosiveWarhead;

		Valueable<int> OverlayExplodeThreshold;

		NullableIdx<VocClass> DecloakSound;
		Nullable<int> CloakHeight;

		Valueable<bool> EnemyInsignia;

		Valueable<bool> ReturnStructures;

		Valueable<bool> TypeSelectUseDeploy;

		Valueable<double> DeactivateDim_Powered;
		Valueable<double> DeactivateDim_EMP;
		Valueable<double> DeactivateDim_Operator;

		// hunter seeker
		ValueableVector<BuildingTypeClass*> HunterSeekerBuildings;
		Valueable<int> HunterSeekerDetonateProximity;
		Valueable<int> HunterSeekerDescendProximity;
		Valueable<int> HunterSeekerAscentSpeed;
		Valueable<int> HunterSeekerDescentSpeed;
		Valueable<int> HunterSeekerEmergeSpeed;

		// drop pods
		Valueable<int> DropPodMinimum;
		Valueable<int> DropPodMaximum;
		ValueableVector<TechnoTypeClass*> DropPodTypes;
		Nullable<AnimTypeClass*> DropPodTrailer;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			ElectricDeath(nullptr),
			EngineerDamage (0.0F),
			EngineerAlwaysCaptureTech (true),
			EngineerDamageCursor (MouseCursor::First[MouseCursorType::Detonate]),
			Tiberium_DamageEnabled (false),
			Tiberium_HealEnabled (false),
			Tiberium_ExplosiveWarhead (nullptr),
			OverlayExplodeThreshold (0),
			DecloakSound(),
			CloakHeight(),
			EnemyInsignia(true),
			ReturnStructures(false),
			TypeSelectUseDeploy(true),
			DeactivateDim_Powered(0.5),
			DeactivateDim_EMP(0.8),
			DeactivateDim_Operator(0.65),
			HunterSeekerBuildings (),
			HunterSeekerDetonateProximity (0),
			HunterSeekerDescendProximity (0),
			HunterSeekerAscentSpeed (0),
			HunterSeekerDescentSpeed (0),
			HunterSeekerEmergeSpeed (0),
			DropPodMinimum (0),
			DropPodMaximum (0),
			DropPodTypes (),
			DropPodTrailer (),
			CanMakeStuffUp(false)
			{
				MultiEngineer[0] = false; // Skirmish
				MultiEngineer[1] = false; // LAN
				MultiEngineer[2] = false; // WOnline
			};

		virtual ~ExtData() {
		}

		virtual void LoadFromINIFile(TT *pThis, CCINIClass *pINI);
		virtual void LoadBeforeTypeData(TT *pThis, CCINIClass *pINI);
		virtual void LoadAfterTypeData(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants(TT *pThis);

		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
		}
};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static void Allocate(RulesClass *pThis);
	static void Remove(RulesClass *pThis);

	static void LoadFromINIFile(RulesClass *pThis, CCINIClass *pINI);
	static void LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI);
	static void LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI);

	static ExtData* Global()
	{
		return Data.get();
	};

	static DynamicVectorClass<CameoDataStruct> TabCameos[4];

	static void ClearCameos();
};

#endif
