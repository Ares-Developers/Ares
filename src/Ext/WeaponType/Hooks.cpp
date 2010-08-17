#include "Body.h"
#include "../Techno/Body.h"
#include <LaserDrawClass.h>
#include "../BuildingType/Body.h"

DEFINE_HOOK(6FD438, TechnoClass_FireLaser, 6)
{
	GET(WeaponTypeClass *, pWeapon, ECX);
	GET(LaserDrawClass *, pBeam, EAX);

	auto pData = WeaponTypeExt::ExtMap.Find(pWeapon);
	int Thickness = pData->Laser_Thickness;
	if(Thickness > 1) {
		pBeam->Thickness = Thickness;
	}

	return 0;

}

DEFINE_HOOK(6FF4DE, TechnoClass_Fire_IsLaser, 6) {
	GET(TechnoClass*, pThis, ECX);
	GET(TechnoClass*, pTarget, EDI);
	GET(WeaponTypeClass*, pFiringWeaponType, EBX);
	GET_STACK(int, idxWeapon, 0x18);

	auto pData = WeaponTypeExt::ExtMap.Find(pFiringWeaponType);
	int Thickness = pData->Laser_Thickness;

	if(BuildingClass* pBld = specific_cast<BuildingClass*>(pThis)) {
		WeaponTypeClass* pTWeapon = pBld->GetTurretWeapon()->WeaponType;

		if(LaserDrawClass* pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, &TechnoClass::DefaultCoords())) {
			
			//default thickness for buildings. this was 3 for PrismType (rising to 5 for supported prism) but no idea what it was for non-PrismType - setting to 3 for all BuildingTypes now.
			if (Thickness == -1) {
				pLaser->Thickness = 3;
			} else {
				pLaser->Thickness = Thickness;
			}
			
			//BuildingExt::ExtData *pBldData = BuildingExt::ExtMap.Find(pBld);
			BuildingTypeClass *pBldType = pBld->Type;
			BuildingTypeExt::ExtData *pBldTypeData = BuildingTypeExt::ExtMap.Find(pBldType);

			if (pBldTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::YES
				|| pBldTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::ATTACK) {
				//is a prism tower
				
				if (pBld->SupportingPrisms > 0) { //Ares sets this to the longest backward chain
					//is being supported... so increase beam intensity
					if (pBldTypeData->PrismForwarding.Intensity < 0) {
						pLaser->field_21 = 1; //this appears to change the RGB values for OuterColor (1=double, 0=halve)
						pLaser->Thickness -= pBldTypeData->PrismForwarding.Intensity; //add on absolute intensity
					} else if (pBldTypeData->PrismForwarding.Intensity > 0) {
						pLaser->field_21 = 1; //this appears to change the RGB values for OuterColor (1=double, 0=halve)
						pLaser->Thickness += (pBldTypeData->PrismForwarding.Intensity * pBld->SupportingPrisms);
					}
				}
			}
		}
	} else {
		if(LaserDrawClass* pLaser = pThis->CreateLaser(pTarget, idxWeapon, pFiringWeaponType, &TechnoClass::DefaultCoords())) {
			if (Thickness == -1) {
				pLaser->Thickness = 2;
			} else {
				pLaser->Thickness = Thickness;
			}
		}
	}

	// skip all default handling
	return 0x6FF656;
}

/* Original transcribe (by AlexB and DCoder)
DEFINE_HOOK(6FF4DE, TechnoClass_Fire_IsLaser, 6) {
	GET(TechnoClass*, pThis, ECX);
	GET(TechnoClass*, pTarget, EDI);
	GET(WeaponTypeClass*, pFiringWeaponType, EBX);
	GET_STACK(int, idxWeapon, 0x18);
	
	if(BuildingClass* pBld = specific_cast<BuildingClass*>(pThis)) {
		WeaponTypeClass* pTWeapon = pBld->GetTurretWeapon()->WeaponType;

		if(LaserDrawClass* pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, &TechnoClass::DefaultCoords())) {
			if(pBld->Type == RulesClass::Instance->PrismType) {
				pLaser->Thickness = 3;
				if(pBld->SupportingPrisms > 0) {
					pLaser->field_21 = 1;
					pLaser->Thickness = 5;
				}
			}
		}
	} else {
		if(LaserDrawClass* pLaser = pThis->CreateLaser(pTarget, idxWeapon, pFiringWeaponType, &TechnoClass::DefaultCoords())) {
			if(pFiringWeaponType->IsHouseColor) {
				pLaser->Thickness = 2;
			}
		}
	}

	// skip all default handling
	return 0x6FF656;
}
*/




