#ifndef RULES_EXT_H
#define RULES_EXT_H

#include <CCINIClass.h>
#include <RulesClass.h>

#include "../_Container.hpp"
#include "../../Utilities/Template.h"

//ifdef DEBUGBUILD
#include "../../Misc/Debug.h"
//endif

class AnimTypeClass;
class MouseCursor;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;

class RulesExt
{
	public:
	typedef RulesClass TT;

	class ExtData : public Extension<TT>
	{
		public:
		Valueable<AnimTypeClass* >ElectricDeath;
		Valueable<double> EngineerDamage;
		Valueable<bool> EngineerAlwaysCaptureTech;
		Valueable<MouseCursor> EngineerDamageCursor;
		bool MultiEngineer[3];

		Valueable<bool> TogglePowerAllowed;
		Valueable<int> TogglePowerDelay;
		Valueable<int> TogglePowerIQ;
		Valueable<MouseCursor> TogglePowerCursor;
		Valueable<MouseCursor> TogglePowerNoCursor;

		Valueable<bool> CanMakeStuffUp;

		Valueable<bool> Tiberium_DamageEnabled;
		Valueable<bool> Tiberium_HealEnabled;
		Valueable<WarheadTypeClass*> Tiberium_ExplosiveWarhead;

		Valueable<int> OverlayExplodeThreshold;

		NullableIdx<VocClass> DecloakSound;
		Nullable<int> CloakHeight;

		Valueable<bool> EnemyInsignia;
		Valueable<bool> EnemyWrench;

		Valueable<bool> ReturnStructures;

		Valueable<bool> TypeSelectUseDeploy;

		Valueable<bool> TeamRetaliate;

		Valueable<double> DeactivateDim_Powered;
		Valueable<double> DeactivateDim_EMP;
		Valueable<double> DeactivateDim_Operator;

		Valueable<double> BerserkROFMultiplier;

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

		Valueable<bool> AutoRepelAI;
		Valueable<bool> AutoRepelPlayer;

		ExtData(TT* const OwnerObject) : Extension<TT>(OwnerObject),
			ElectricDeath(nullptr),
			EngineerDamage (0.0),
			EngineerAlwaysCaptureTech (true),
			EngineerDamageCursor (MouseCursor::GetCursor(MouseCursorType::Detonate)),
			TogglePowerCursor (MouseCursor::GetCursor(MouseCursorType::Power)),
			TogglePowerNoCursor (MouseCursor::GetCursor(MouseCursorType::Disallowed)),
			TogglePowerAllowed (false),
			TogglePowerDelay (45),
			TogglePowerIQ (-1),
			Tiberium_DamageEnabled (false),
			Tiberium_HealEnabled (false),
			Tiberium_ExplosiveWarhead (nullptr),
			OverlayExplodeThreshold (0),
			DecloakSound(),
			CloakHeight(),
			EnemyInsignia(true),
			EnemyWrench (true),
			ReturnStructures(false),
			TypeSelectUseDeploy(true),
			TeamRetaliate(false),
			DeactivateDim_Powered(0.5),
			DeactivateDim_EMP(0.8),
			DeactivateDim_Operator(0.65),
			BerserkROFMultiplier(0.5),
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

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void LoadBeforeTypeData(TT *pThis, CCINIClass *pINI);
		virtual void LoadAfterTypeData(TT *pThis, CCINIClass *pINI);
		virtual void InitializeConstants() override;

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

	static void Clear() {
		ClearCameos();
		Allocate(RulesClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed) {
		Global()->InvalidatePointer(ptr, removed);
	}
};

#endif
