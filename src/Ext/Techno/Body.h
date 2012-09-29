#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <xcompile.h>
#include <algorithm>

#include <AircraftClass.h>
#include <CellSpread.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <MapClass.h>
#include <Randomizer.h>
#include <TemporalClass.h>
#include <WeaponTypeClass.h>
#include <WarheadTypeClass.h>
#include <AlphaShapeClass.h>
#include <SpotlightClass.h>

#include "../WarheadType/Body.h"
#include "../WeaponType/Body.h"
#include "../TechnoType/Body.h"

#include "../_Container.hpp"

//#include "../../Misc/JammerClass.h"
class JammerClass;
class PoweredUnitClass;

class TechnoExt
{
public:
	typedef TechnoClass TT;

	class ExtData : public Extension<TT>
	{
	public:
		// weapon slots fsblargh
		BYTE idxSlot_Wave;
		BYTE idxSlot_Beam;
		BYTE idxSlot_Warp;
		BYTE idxSlot_Parasite;

		bool Survivors_Done;

		TimerStruct CloakSkipTimer;
		SHPStruct * Insignia_Image;

		BuildingClass *GarrisonedIn; // when infantry garrisons a building, we need a fast way to find said building when damage forwarding kills it

		AnimClass *EMPSparkleAnim;
		eMission EMPLastMission;

		bool ShadowDrawnManually;

		bool DriverKilled;

		int HijackerHealth;
		HouseClass* HijackerHouse;

		// 305 Radar Jammers
		JammerClass* RadarJam;
		
		// issue #617 powered units
		PoweredUnitClass* PoweredUnit;

		//#1573, #1623, #255 Stat-modifiers/ongoing animations
		DynamicVectorClass <AttachEffectClass *> AttachedEffects;
		bool AttachEffects_RecreateAnims;

		//stuff for #1623
		bool AttachedTechnoEffect_isset;
		int AttachedTechnoEffect_Delay;

		//crate fields
		double Crate_FirepowerMultiplier;
		double Crate_ArmorMultiplier;
		double Crate_SpeedMultiplier;
		bool Crate_Cloakable;

		TemporalClass * MyOriginalTemporal;

		EBolt * MyBolt;

		Nullable<bool> AltOccupation; // if the unit marks cell occupation flags, this is set to whether it uses the "high" occupation members

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			idxSlot_Wave (0),
			idxSlot_Beam (0),
			idxSlot_Warp (0),
			idxSlot_Parasite(0),
			Survivors_Done (0),
			Insignia_Image (NULL),
			GarrisonedIn (NULL),
			HijackerHealth (-1),
			HijackerHouse (NULL),
			DriverKilled (false),
			EMPSparkleAnim (NULL),
			EMPLastMission (mission_None),
			ShadowDrawnManually (false),
			RadarJam(NULL),
			PoweredUnit(NULL),
			MyOriginalTemporal(NULL),
			AltOccupation(),
			AttachEffects_RecreateAnims(false),
			AttachedTechnoEffect_isset (false),
			AttachedTechnoEffect_Delay (0),
			Crate_FirepowerMultiplier(1.0),
			Crate_ArmorMultiplier(1.0),
			Crate_SpeedMultiplier(1.0),
			Crate_Cloakable(false)
			{
				this->CloakSkipTimer.Stop();
				// hope this works with the timing - I assume it does, since Types should be created before derivates thereof
				//TechnoTypeExt::ExtData* TypeExt = TechnoTypeExt::ExtMap.Find(this->AttachedToObject->GetTechnoType());
				/*if(TypeExt->RadarJamRadius) {
					RadarJam = new JammerClass(this->AttachedToObject, this); // now in hooks -> TechnoClass_Update_CheckOperators
				}*/
			};

		virtual ~ExtData() {
			//delete RadarJam; // now in hooks -> TechnoClass_Remove
		};

		virtual size_t Size() const { return sizeof(*this); };

		// when any pointer in the game expires, this is called - be sure to tell everyone we own to invalidate it
		virtual void InvalidatePointer(void *ptr, bool bRemoved) {
			AnnounceInvalidPointer(this->GarrisonedIn, ptr);
			this->InvalidateAttachEffectPointer(ptr);
			AnnounceInvalidPointer(this->MyOriginalTemporal, ptr);
		}

		bool IsOperated();
		bool IsPowered();

		AresAction::Value GetActionHijack(TechnoClass *pTarget);
		bool PerformActionHijack(TechnoClass* pTarget);

		unsigned int AlphaFrame(SHPStruct * Image);

		bool DrawVisualFX();

		UnitTypeClass * GetUnitType();

		bool IsDeactivated() const;

		eAction GetDeactivatedAction(ObjectClass *Hovered = NULL) const;

		void InvalidateAttachEffectPointer(void *ptr);

		bool IsCloakable(bool allowPassive) const;
		bool CloakAllowed(bool allowPassive) const;
		bool CloakDisallowed(bool allowPassive) const;
		bool IsReallyCloakable() const;
	};

	static Container<TechnoExt> ExtMap;
	static hash_map<ObjectClass *, AlphaShapeClass *> AlphaExt;
	static hash_map<TechnoClass *, BuildingLightClass *> SpotlightExt;

	static BuildingLightClass * ActiveBuildingLight;

	static FireError::Value FiringStateCache;

	static bool NeedsRegap;

	static void SpawnSurvivors(FootClass *pThis, TechnoClass *pKiller, bool Select, bool IgnoreDefenses);
	static bool EjectSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select);
	static void EjectPassengers(FootClass *, signed short);
	static void GetPutLocation(CoordStruct const &, CoordStruct &, int);
	static bool EjectRandomly(FootClass*, CoordStruct const &, int, bool);
	// If available, removes the hijacker from its victim and creates an InfantryClass instance.
	static InfantryClass* RecoverHijacker(FootClass *pThis);

	static void StopDraining(TechnoClass *Drainer, TechnoClass *Drainee);

	static bool CreateWithDroppod(FootClass *Object, CoordStruct *XYZ);

	static void TransferIvanBomb(TechnoClass *From, TechnoClass *To);
	static void TransferAttachedEffects(TechnoClass *From, TechnoClass *To);

	static void RecalculateStats(TechnoClass *pTechno);

	static void FreeSpecificSlave(TechnoClass *Slave, HouseClass *Affector);
	static void DetachSpecificSpawnee (TechnoClass *Spawnee, HouseClass *NewSpawneeOwner);
	static bool CanICloakByDefault(TechnoClass *pTechno);

	static void Destroy(TechnoClass* pTechno, TechnoClass* pKiller = NULL, HouseClass* pKillerHouse = NULL, WarheadTypeClass* pWarhead = NULL);
/*
	static int SelectWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget);
	static bool EvalWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
	static float EvalVersesAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
*/
};

typedef hash_map<ObjectClass *, AlphaShapeClass *> hash_AlphaExt;
typedef hash_map<TechnoClass *, BuildingLightClass *> hash_SpotlightExt;
#endif
