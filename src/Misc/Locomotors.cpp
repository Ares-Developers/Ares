#include "../Ares.h"
#include "../Ext/TechnoType/Body.h"

#include <AircraftClass.h>
#include <LocomotionClass.h>

DEFINE_HOOK(6622E0, RocketLocomotionClass_ILocomotion_Process_CustomMissile, 6)
{
	GET(AircraftClass*, pThis, ECX);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pThis->Type)) {
		if(pExt->IsCustomMissile.Get()) {
			R->EAX(&pExt->CustomMissileData);
			return 0x66230A;
		}
	}

	return 0;
}

DEFINE_HOOK(66305A, RocketLocomotionClass_Explode_CustomMissile, 6)
{
	GET(AircraftClass*, pMissile, EDX);
	GET(AircraftTypeClass*, pType, ECX);
	GET(ILocomotion*, pLocomotor, ESI);

	LEA_STACK(WarheadTypeClass**, pWarhead, 0x10);
	LEA_STACK(RocketStruct**, pRocketData, 0x14);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile.Get()) {
			*pRocketData = &pExt->CustomMissileData;

			// TODO: Create RocketLocomotorClass
			bool isElite = *(bool*)((int)pLocomotor + 0x51);
			*pWarhead = (isElite ? pExt->CustomMissileEliteWarhead.Get() : pExt->CustomMissileWarhead.Get());
			
			return 0x6630DD;
		}
	}

	return 0;
}

DEFINE_HOOK(6632F2, RocketLocomotionClass_ILocomotion_MoveTo_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, EDX);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile.Get()) {
			R->EDX(&pExt->CustomMissileData);
			return 0x66331E;
		}
	}

	return 0;
}

DEFINE_HOOK(6634F6, RocketLocomotionClass_ILocomotion_DrawMatrix_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, ECX);

	if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		if(pExt->IsCustomMissile.Get()) {
			R->EAX(&pExt->CustomMissileData);
			return 0x66351B;
		}
	}

	return 0;
}
