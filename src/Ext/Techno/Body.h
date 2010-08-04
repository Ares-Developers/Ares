#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <xcompile.h>
#include <vector>
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

#include "../_Container.hpp"

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

		ExtData(const DWORD Canary, TT* const OwnerObject) : Extension<TT>(Canary, OwnerObject),
			idxSlot_Wave (0),
			idxSlot_Beam (0),
			idxSlot_Warp (0),
			idxSlot_Parasite(0),
			Survivors_Done (0),
			Insignia_Image (NULL),
			GarrisonedIn (NULL),
			EMPSparkleAnim (NULL),
			EMPLastMission (mission_None),
			ShadowDrawnManually (false)
			{
				this->CloakSkipTimer.Stop();
			};

		virtual ~ExtData() { };

		virtual size_t Size() const { return sizeof(*this); };

		virtual void InvalidatePointer(void *ptr) {
			AnnounceInvalidPointer(this->GarrisonedIn, ptr);
		}

		bool IsOperated();
		bool IsPowered();

		unsigned int AlphaFrame(SHPStruct * Image);

		bool DrawVisualFX();

		UnitTypeClass * GetUnitType();
	};

	static Container<TechnoExt> ExtMap;
	static hash_map<ObjectClass *, AlphaShapeClass *> AlphaExt;
	static hash_map<TechnoClass *, BuildingLightClass *> SpotlightExt;

	static BuildingLightClass * ActiveBuildingLight;

	static FireError::Value FiringStateCache;

	static bool NeedsRegap;

	static void SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select);
	static bool EjectSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select);
	static void EjectPassengers(TechnoClass *, signed short);
	static void GetPutLocation(CoordStruct const &, CoordStruct &);

	static void StopDraining(TechnoClass *Drainer, TechnoClass *Drainee);
/*
	static int SelectWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget);
	static bool EvalWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
	static float EvalVersesAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
*/
};

typedef hash_map<ObjectClass *, AlphaShapeClass *> hash_AlphaExt;
typedef hash_map<TechnoClass *, BuildingLightClass *> hash_SpotlightExt;
#endif
