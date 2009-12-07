#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <Helpers/Macro.h>
#include <hash_map>
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

		ExtData(const DWORD Canary = 0, const TT* OwnerObject = NULL) : Extension(Canary, OwnerObject),
			idxSlot_Wave (0),
			idxSlot_Beam (0),
			idxSlot_Warp (0),
			idxSlot_Parasite(0),
			Survivors_Done (0),
			Insignia_Image (NULL)
			{
				this->CloakSkipTimer.Stop();
			};

		virtual ~ExtData() { };

		virtual size_t Size() const { return sizeof(*this); };

	};

	static Container<TechnoExt> ExtMap;
	static stdext::hash_map<ObjectClass *, AlphaShapeClass *> AlphaExt;
	static stdext::hash_map<TechnoClass *, BuildingLightClass *> SpotlightExt;

	static BuildingLightClass * ActiveBuildingLight;

	static FireError FiringStateCache;

	static bool NeedsRegap;

	static void SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select);
	static bool ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select);

	static void PointerGotInvalid(void *ptr);

/*
	static int SelectWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget);
	static bool EvalWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
	static float EvalVersesAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
*/
};

typedef stdext::hash_map<ObjectClass *, AlphaShapeClass *> hash_AlphaExt;
typedef stdext::hash_map<TechnoClass *, BuildingLightClass *> hash_SpotlightExt;
#endif
