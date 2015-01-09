#include "Body.h"
#include "../Techno/Body.h"
#include <LaserDrawClass.h>
#include "../BuildingType/Body.h"
#include "../BulletType/Body.h"
#include <BuildingClass.h>

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

	int idxWeapon = R->Base<int>(0xC); // don't use stack offsets - function uses on-the-fly stack realignments which mean offsets are not constants

	auto pData = WeaponTypeExt::ExtMap.Find(pFiringWeaponType);
	int Thickness = pData->Laser_Thickness;

	LaserDrawClass *pLaser = nullptr;

	if(BuildingClass* pBld = specific_cast<BuildingClass*>(pThis)) {
		WeaponTypeClass* pTWeapon = pBld->GetTurretWeapon()->WeaponType;

		if((pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, CoordStruct::Empty)) != nullptr) {
			
			//default thickness for buildings. this was 3 for PrismType (rising to 5 for supported prism) but no idea what it was for non-PrismType - setting to 3 for all BuildingTypes now.
			if (Thickness == -1) {
				pLaser->Thickness = 3;
			} else {
				pLaser->Thickness = Thickness;
			}
			
			//BuildingExt::ExtData *pBldData = BuildingExt::ExtMap.Find(pBld);
			BuildingTypeClass *pBldType = pBld->Type;
			BuildingTypeExt::ExtData *pBldTypeData = BuildingTypeExt::ExtMap.Find(pBldType);

			if (pBldTypeData->PrismForwarding.CanAttack()) {
				//is a prism tower
				
				if (pBld->SupportingPrisms > 0) { //Ares sets this to the longest backward chain
					//is being supported... so increase beam intensity
					if (pBldTypeData->PrismForwarding.Intensity < 0) {
						pLaser->Thickness -= pBldTypeData->PrismForwarding.Intensity; //add on absolute intensity
					} else if (pBldTypeData->PrismForwarding.Intensity > 0) {
						pLaser->Thickness += (pBldTypeData->PrismForwarding.Intensity * pBld->SupportingPrisms);
					}
				}
			}
		}
	} else {
		if((pLaser = pThis->CreateLaser(pTarget, idxWeapon, pFiringWeaponType, CoordStruct::Empty)) != nullptr) {
			if (Thickness == -1) {
				pLaser->Thickness = 2;
			} else {
				pLaser->Thickness = Thickness;
			}
		}
	}

	if(pLaser) {
		// required for Thickness to work right
		pLaser->IsSupported = true; //this appears to change the RGB values for OuterColor (1=double, 0=halve)
	}

	// skip all default handling
	return 0x6FF656;
}
