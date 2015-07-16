#pragma once

#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/WarheadType/Body.h"

class EMPulse
{
public:
	static void CreateEMPulse(WarheadTypeExt::ExtData *Warhead, const CoordStruct &Target, TechnoClass *Firer);
	static void DisableEMPEffect(TechnoClass *Techno);
	static bool IsTypeEMPProne(TechnoTypeClass const* pType);
	static bool IsDeactivationAdvisable(TechnoClass *Techno);
	static void UpdateSparkleAnim(
		TechnoClass* pTechno, AnimTypeClass* pSpecific = nullptr);
	static void UpdateSparkleAnim(
		TechnoClass* pTechno, TechnoClass const* pFrom);

	// two dummy functions not used by EMP
	static void DisableEMPEffect2(TechnoClass *Techno);
	static bool EnableEMPEffect2(TechnoClass *Techno);

protected:
	static void deliverEMPDamage(
		TechnoClass* pTechno, TechnoClass* pFirer,
		WarheadTypeExt::ExtData* pWarhead);
	static bool isEMPTypeImmune(TechnoClass *);
	static bool isEMPImmune(TechnoClass *, HouseClass *);
	static bool isCurrentlyEMPImmune(TechnoClass *, HouseClass *);
	static bool isEligibleEMPTarget(TechnoClass *, HouseClass *, WarheadTypeClass *);
	static void updateRadarBlackout(BuildingClass* pBuilding);
	static void updateSpawnManager(TechnoClass *, ObjectClass *);
	static AnimTypeClass* getSparkleAnimType(TechnoClass const* pTechno);
	static bool enableEMPEffect(TechnoClass *, ObjectClass *);
	static void announceAttack(TechnoClass *);
	static bool thresholdExceeded(TechnoClass *);
	static bool supportVerses;

public:
	static bool verbose;
};
